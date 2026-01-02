/*
 * ============================================================================
 * GALVO LASER WORKER - PRODUCTION GRADE
 * ============================================================================
 * High-Speed Precision Targeting System
 * - Galvo mirror control via ESP32 DAC
 * - Sub-millisecond positioning
 * - Professional safety interlocks
 * - Advanced calibration system
 * - WiFi HTTP communication with Master Hub
 * ============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================
// NETWORK CONFIGURATION
// ============================================

const char* MASTER_SSID = "FarmBot_Control";
const char* MASTER_PASSWORD = "L1ncolnTech";
const char* MASTER_IP = "192.168.4.1";
const char* WORKER_NAME = "GALVO_LASER";

WebServer server(80);
HTTPClient http;

// ============================================
// PIN DEFINITIONS
// ============================================

// Galvo Control (DAC outputs - FIXED pins on ESP32)
#define GALVO_X_PIN     25    // DAC1 - X-axis galvo (GPIO25 ONLY)
#define GALVO_Y_PIN     26    // DAC2 - Y-axis galvo (GPIO26 ONLY)

// Laser Control
#define LASER_PWM_PIN   32    // PWM control for laser power
#define LASER_ENABLE    33    // Digital enable (HIGH = laser can fire)
#define LASER_TTL       27    // TTL trigger for pulsed operation

// Safety System
#define ESTOP_PIN       35    // Emergency stop input (active LOW)
#define INTERLOCK_PIN   34    // Safety interlock (active HIGH = safe)
#define TEMP_SENSOR     36    // Analog temperature sensor (laser cooling)

// Status Indicators
#define STATUS_LED      2     // System status LED
#define LASER_LED       15    // Laser active indicator (RED LED)
#define READY_LED       4     // System ready indicator (GREEN LED)

// Galvo Enable (some drivers need enable signal)
#define GALVO_ENABLE    14    // HIGH = galvos enabled

// ============================================
// GALVO CALIBRATION DATA
// ============================================

struct GalvoCalibration {
  // DAC value range (ESP32 DAC is 8-bit: 0-255)
  int xMin = 0;      // Leftmost position
  int xMax = 255;    // Rightmost position
  int xCenter = 128; // Center position
  
  int yMin = 0;      // Bottom position
  int yMax = 255;    // Top position
  int yCenter = 128; // Center position
  
  // Fine-tuning offsets (for optical distortion correction)
  float xOffset = 0.0;
  float yOffset = 0.0;
  float xScale = 1.0;
  float yScale = 1.0;
  
  // Settling time after position change (microseconds)
  int settleTimeUs = 500;  // 0.5ms default
  
  // Current position cache
  int currentX = 128;
  int currentY = 128;
} galvo;

// ============================================
// LASER CONTROL
// ============================================

struct LaserControl {
  bool enabled = false;
  bool systemSafe = true;
  int powerPercent = 100;
  int pwmChannel = 0;
  int pwmFreq = 25000;  // 25kHz PWM frequency
  int pwmResolution = 8; // 8-bit resolution (0-255)
  
  float temperature = 0.0;
  float tempLimit = 65.0;  // °C - shutdown if exceeded
  
  unsigned long totalFireTime = 0;  // Total laser on-time (ms)
  unsigned long lastFireTime = 0;
} laser;

// ============================================
// STATISTICS
// ============================================

int targetsHit = 0;
unsigned long lastHeartbeat = 0;
bool connected = false;
unsigned long systemUptime = 0;

// ============================================
// SAFETY FLAGS
// ============================================

enum SafetyState {
  SAFE,
  ESTOP_ACTIVE,
  INTERLOCK_OPEN,
  OVERTEMP,
  POWER_FAULT
};

SafetyState safetyState = SAFE;

// ============================================
// FUNCTION PROTOTYPES
// ============================================

void setupGalvos();
void setupLaser();
void setupSafety();
void moveGalvo(float x, float y);
void moveGalvoRaw(int xDac, int yDac);
void fireLaser(int durationMs, int powerPercent);
void enableLaser(bool enable);
void executeTarget(float x, float y, int duration, int power);
void runCalibration();
void calibrationPattern();
void homePosition();
void checkSafety();
float readTemperature();
void emergencyStop(const char* reason);
void registerWithMaster();
void sendHeartbeat();

// ============================================
// SETUP
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════════════════════════════╗");
  Serial.println("║          ⚡ GALVO LASER WORKER - PRODUCTION GRADE              ║");
  Serial.println("║          High-Speed Precision Targeting System                 ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  // Initialize status LEDs
  pinMode(STATUS_LED, OUTPUT);
  pinMode(LASER_LED, OUTPUT);
  pinMode(READY_LED, OUTPUT);
  
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(LASER_LED, LOW);
  digitalWrite(READY_LED, LOW);
  
  // Setup safety system FIRST
  Serial.print("🛡️  Initializing safety system... ");
  setupSafety();
  Serial.println("✅");
  
  // Setup galvo mirrors
  Serial.print("🔧 Initializing galvo mirrors... ");
  setupGalvos();
  Serial.println("✅");
  
  // Setup laser
  Serial.print("🔧 Initializing laser system... ");
  setupLaser();
  Serial.println("✅");
  
  // Connect to Master Hub WiFi
  Serial.print("📡 Connecting to Master Hub WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(MASTER_SSID, MASTER_PASSWORD);
  
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅");
    Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
    connected = true;
  } else {
    Serial.println(" ❌ FAILED!");
    Serial.println("   Check Master Hub is running!");
    Serial.println("   Restarting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }
  
  // Setup command server
  server.on("/command", HTTP_POST, []() {
    if(server.hasArg("plain")) {
      StaticJsonDocument<256> doc;
      deserializeJson(doc, server.arg("plain"));
      
      String cmd = doc["cmd"];
      
      if(cmd == "TARGET") {
        float x = doc["x"];
        float y = doc["y"];
        int duration = doc["duration"];
        int power = doc["power"];
        
        executeTarget(x, y, duration, power);
      }
      else if(cmd == "STOP") {
        emergencyStop("Remote command");
      }
      else if(cmd == "CALIBRATE") {
        runCalibration();
      }
      else if(cmd == "HOME") {
        homePosition();
      }
      else if(cmd == "PATTERN") {
        calibrationPattern();
      }
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400);
    }
  });
  
  server.on("/status", HTTP_GET, []() {
    StaticJsonDocument<256> doc;
    doc["name"] = WORKER_NAME;
    doc["targets"] = targetsHit;
    doc["temp"] = laser.temperature;
    doc["safe"] = (safetyState == SAFE);
    doc["laser_enabled"] = laser.enabled;
    doc["uptime"] = millis() / 1000;
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });
  
  server.begin();
  Serial.println("🌐 Command Server... ✅ OK (Port 80)");
  
  // Home position
  Serial.println("\n🏠 Homing galvos to center...");
  homePosition();
  
  // Self-test
  Serial.println("\n🧪 Running self-test...");
  calibrationPattern();
  
  // Register with master
  Serial.println("\n📤 Registering with Master Hub...");
  registerWithMaster();
  
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.println("║              🟢 GALVO LASER WORKER READY                       ║");
  Serial.println("║                                                                ║");
  Serial.println("║  ⚡ Sub-millisecond positioning                                ║");
  Serial.println("║  🛡️  Safety systems active                                     ║");
  Serial.println("║  Waiting for commands from Master Hub...                      ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  digitalWrite(READY_LED, HIGH);
  
  // Ready beeps
  for(int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(100);
    digitalWrite(STATUS_LED, LOW);
    delay(100);
  }
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
  server.handleClient();
  
  // Safety checks every loop
  checkSafety();
  
  // Send heartbeat every 5 seconds
  if(millis() - lastHeartbeat > 5000) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
  
  // Blink status LED when ready
  static unsigned long lastBlink = 0;
  if(millis() - lastBlink > 1000 && safetyState == SAFE) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    lastBlink = millis();
  }
  
  delay(1);
}

// ============================================
// GALVO CONTROL
// ============================================

void setupGalvos() {
  // Enable galvo drivers
  pinMode(GALVO_ENABLE, OUTPUT);
  digitalWrite(GALVO_ENABLE, HIGH);
  
  // ESP32 DAC setup (built-in, no extra config needed)
  // DAC1 = GPIO25, DAC2 = GPIO26
  // Output range: 0-3.3V (8-bit: 0-255 maps to 0-3.3V)
  
  // Center position
  dacWrite(GALVO_X_PIN, galvo.xCenter);
  dacWrite(GALVO_Y_PIN, galvo.yCenter);
  
  delay(100);  // Allow galvos to settle
}

void moveGalvo(float x, float y) {
  // Convert normalized coordinates (0.0-1.0) to DAC values
  // Apply calibration offsets and scaling
  
  float xNorm = (x + galvo.xOffset) * galvo.xScale;
  float yNorm = (y + galvo.yOffset) * galvo.yScale;
  
  // Constrain to 0.0-1.0
  xNorm = constrain(xNorm, 0.0, 1.0);
  yNorm = constrain(yNorm, 0.0, 1.0);
  
  // Map to DAC range
  int xDac = map(xNorm * 1000, 0, 1000, galvo.xMin, galvo.xMax);
  int yDac = map(yNorm * 1000, 0, 1000, galvo.yMin, galvo.yMax);
  
  moveGalvoRaw(xDac, yDac);
}

void moveGalvoRaw(int xDac, int yDac) {
  // Constrain to 8-bit DAC range
  xDac = constrain(xDac, 0, 255);
  yDac = constrain(yDac, 0, 255);
  
  // Write to DAC
  dacWrite(GALVO_X_PIN, xDac);
  dacWrite(GALVO_Y_PIN, yDac);
  
  // Update position cache
  galvo.currentX = xDac;
  galvo.currentY = yDac;
  
  // Allow galvos to settle (sub-millisecond)
  delayMicroseconds(galvo.settleTimeUs);
}

void homePosition() {
  Serial.println("   Moving to center position...");
  moveGalvoRaw(galvo.xCenter, galvo.yCenter);
  Serial.println("   ✅ Centered");
}

// ============================================
// LASER CONTROL
// ============================================

void setupLaser() {
  // Laser enable pin
  pinMode(LASER_ENABLE, OUTPUT);
  digitalWrite(LASER_ENABLE, LOW);  // Laser disabled by default
  
  // TTL pulse pin
  pinMode(LASER_TTL, OUTPUT);
  digitalWrite(LASER_TTL, LOW);
  
  // PWM setup for laser power control
  ledcSetup(laser.pwmChannel, laser.pwmFreq, laser.pwmResolution);
  ledcAttachPin(LASER_PWM_PIN, laser.pwmChannel);
  ledcWrite(laser.pwmChannel, 0);  // 0% power
  
  laser.enabled = false;
}

void enableLaser(bool enable) {
  if(enable && safetyState == SAFE) {
    digitalWrite(LASER_ENABLE, HIGH);
    digitalWrite(LASER_LED, HIGH);
    laser.enabled = true;
    Serial.println("   🔴 Laser ENABLED");
  } else {
    digitalWrite(LASER_ENABLE, LOW);
    digitalWrite(LASER_LED, LOW);
    digitalWrite(LASER_TTL, LOW);
    ledcWrite(laser.pwmChannel, 0);
    laser.enabled = false;
    Serial.println("   ⚫ Laser DISABLED");
  }
}

void fireLaser(int durationMs, int powerPercent) {
  if(safetyState != SAFE) {
    Serial.println("   ⚠️  SAFETY FAULT - Laser fire blocked!");
    return;
  }
  
  // Constrain power
  powerPercent = constrain(powerPercent, 0, 100);
  
  // Convert to PWM value (0-255)
  int pwmValue = map(powerPercent, 0, 100, 0, 255);
  
  Serial.printf("   🔥 Firing laser: %dms at %d%% power\n", durationMs, powerPercent);
  
  // Enable laser system
  digitalWrite(LASER_ENABLE, HIGH);
  digitalWrite(LASER_LED, HIGH);
  
  // Set power level
  ledcWrite(laser.pwmChannel, pwmValue);
  
  // TTL trigger for pulse mode
  digitalWrite(LASER_TTL, HIGH);
  
  unsigned long startTime = millis();
  delay(durationMs);
  
  // Laser off
  digitalWrite(LASER_TTL, LOW);
  ledcWrite(laser.pwmChannel, 0);
  digitalWrite(LASER_LED, LOW);
  digitalWrite(LASER_ENABLE, LOW);
  
  // Update statistics
  laser.totalFireTime += durationMs;
  laser.lastFireTime = millis();
  
  Serial.println("   ✅ Laser OFF");
}

// ============================================
// SAFETY SYSTEM
// ============================================

void setupSafety() {
  // E-stop input (active LOW with pullup)
  pinMode(ESTOP_PIN, INPUT_PULLUP);
  
  // Interlock input (active HIGH = safe)
  pinMode(INTERLOCK_PIN, INPUT);
  
  // Temperature sensor (analog)
  pinMode(TEMP_SENSOR, INPUT);
  
  // Initial safety check
  checkSafety();
}

void checkSafety() {
  SafetyState previousState = safetyState;
  
  // Check E-stop (active LOW)
  if(digitalRead(ESTOP_PIN) == LOW) {
    safetyState = ESTOP_ACTIVE;
  }
  // Check interlock (should be HIGH when safe)
  else if(digitalRead(INTERLOCK_PIN) == LOW) {
    safetyState = INTERLOCK_OPEN;
  }
  // Check temperature
  else if(readTemperature() > laser.tempLimit) {
    safetyState = OVERTEMP;
  }
  // All safe
  else {
    safetyState = SAFE;
  }
  
  // If safety state changed to fault
  if(safetyState != SAFE && previousState == SAFE) {
    emergencyStop(getSafetyStateString());
  }
  
  // If recovered
  if(safetyState == SAFE && previousState != SAFE) {
    Serial.println("✅ Safety systems restored");
    digitalWrite(READY_LED, HIGH);
  }
}

float readTemperature() {
  // Read analog temperature sensor
  int adcValue = analogRead(TEMP_SENSOR);
  
  // Convert to temperature (example: TMP36 sensor)
  // TMP36: 10mV/°C, 750mV at 25°C
  float voltage = adcValue * (3.3 / 4095.0);
  float tempC = (voltage - 0.5) * 100.0;
  
  laser.temperature = tempC;
  return tempC;
}

const char* getSafetyStateString() {
  switch(safetyState) {
    case SAFE: return "SAFE";
    case ESTOP_ACTIVE: return "E-STOP PRESSED";
    case INTERLOCK_OPEN: return "INTERLOCK OPEN";
    case OVERTEMP: return "OVERTEMPERATURE";
    case POWER_FAULT: return "POWER FAULT";
    default: return "UNKNOWN";
  }
}

void emergencyStop(const char* reason) {
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.printf("║  🚨 EMERGENCY STOP: %-42s ║\n", reason);
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  // Immediately disable laser
  enableLaser(false);
  
  // Turn off ready LED
  digitalWrite(READY_LED, LOW);
  
  // Rapid blink status LED
  for(int i = 0; i < 10; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(100);
    digitalWrite(STATUS_LED, LOW);
    delay(100);
  }
}

// ============================================
// TARGET EXECUTION
// ============================================

void executeTarget(float x, float y, int duration, int power) {
  if(safetyState != SAFE) {
    Serial.println("⚠️  Cannot execute - system not safe!");
    return;
  }
  
  digitalWrite(STATUS_LED, HIGH);
  
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.printf("║  🎯 TARGET RECEIVED                                            ║\n");
  Serial.printf("║  X: %.3f  Y: %.3f  Duration: %dms  Power: %d%%            ║\n", x, y, duration, power);
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  unsigned long startTime = micros();
  
  // Move galvos (sub-millisecond)
  Serial.println("📍 Positioning galvos...");
  moveGalvo(x, y);
  
  unsigned long moveTime = micros() - startTime;
  Serial.printf("   ⚡ Positioned in %lu µs\n", moveTime);
  
  // Fire laser
  Serial.println("🔥 Firing laser...");
  fireLaser(duration, power);
  
  targetsHit++;
  
  digitalWrite(STATUS_LED, LOW);
  
  unsigned long totalTime = micros() - startTime;
  Serial.printf("✅ Target #%d complete in %lu µs\n\n", targetsHit, totalTime);
}

// ============================================
// CALIBRATION
// ============================================

void runCalibration() {
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.println("║              🎯 GALVO CALIBRATION MODE                         ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  Serial.println("📐 Testing full range of motion...\n");
  
  // Test corners
  struct TestPoint {
    const char* name;
    float x, y;
  };
  
  TestPoint points[] = {
    {"Center", 0.5, 0.5},
    {"Top-Left", 0.0, 1.0},
    {"Top-Right", 1.0, 1.0},
    {"Bottom-Right", 1.0, 0.0},
    {"Bottom-Left", 0.0, 0.0},
    {"Center", 0.5, 0.5}
  };
  
  for(int i = 0; i < 6; i++) {
    Serial.printf("%-15s (%.2f, %.2f)\n", points[i].name, points[i].x, points[i].y);
    moveGalvo(points[i].x, points[i].y);
    
    // Brief laser pulse to mark position
    if(safetyState == SAFE) {
      enableLaser(true);
      digitalWrite(LASER_TTL, HIGH);
      delay(50);
      digitalWrite(LASER_TTL, LOW);
      enableLaser(false);
    }
    
    delay(500);
  }
  
  Serial.println("\n✅ Calibration complete\n");
}

void calibrationPattern() {
  Serial.println("   Running test pattern...");
  
  // Quick scan pattern
  for(float x = 0.2; x <= 0.8; x += 0.15) {
    moveGalvo(x, 0.5);
    delay(50);
  }
  
  for(float y = 0.2; y <= 0.8; y += 0.15) {
    moveGalvo(0.5, y);
    delay(50);
  }
  
  homePosition();
  Serial.println("   ✅ Test pattern complete");
}

// ============================================
// MASTER HUB COMMUNICATION
// ============================================

void registerWithMaster() {
  if(WiFi.status() != WL_CONNECTED) return;
  
  http.begin("http://" + String(MASTER_IP) + "/api/register");
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<128> doc;
  doc["name"] = WORKER_NAME;
  
  String json;
  serializeJson(doc, json);
  
  int httpCode = http.POST(json);
  
  if(httpCode == 200) {
    Serial.println("✅ Registered with Master Hub!");
  } else {
    Serial.printf("⚠️  Registration failed (HTTP %d)\n", httpCode);
  }
  
  http.end();
}

void sendHeartbeat() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    return;
  }
  
  http.begin("http://" + String(MASTER_IP) + "/api/worker_status");
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<256> doc;
  doc["name"] = WORKER_NAME;
  doc["status"] = (safetyState == SAFE) ? "READY" : "FAULT";
  doc["targets"] = targetsHit;
  doc["temp"] = laser.temperature;
  doc["safe"] = (safetyState == SAFE);
  
  String json;
  serializeJson(doc, json);
  
  int httpCode = http.POST(json);
  
  if(httpCode == 200) {
    if(!connected) {
      Serial.println("✅ Connected to Master Hub!");
      connected = true;
    }
    Serial.printf("💚 Heartbeat | Temp: %.1f°C | Targets: %d | %s\n", 
                  laser.temperature, targetsHit, getSafetyStateString());
  } else {
    if(connected) {
      Serial.println("⚠️  Lost connection to Master Hub");
      connected = false;
    }
    Serial.println("💛 Heartbeat failed");
  }
  
  http.end();
}
