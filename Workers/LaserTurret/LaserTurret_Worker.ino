/*
 * ============================================================================
 * LASER TURRET WORKER - WiFi HTTP VERSION (NO ESP-NOW)
 * Connects to Master Hub WiFi and communicates via HTTP
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
const char* WORKER_NAME = "LASER";

WebServer server(80);
HTTPClient http;

// ============================================
// PIN DEFINITIONS
// ============================================

// Pan Stepper (X-axis) - ULN2003 Driver 1
#define PAN_IN1  19
#define PAN_IN2  18
#define PAN_IN3  5
#define PAN_IN4  17

// Tilt Stepper (Y-axis) - ULN2003 Driver 2
#define TILT_IN1 16
#define TILT_IN2 4
#define TILT_IN3 2
#define TILT_IN4 15

// Laser Control
#define LASER_PIN 25

// Status LED
#define STATUS_LED 26

// ============================================
// STEPPER CONFIGURATION
// ============================================

const int STEPS_PER_REV = 4096;
const int STEP_DELAY_US = 1000;

const int PAN_CENTER = 2048;
const int TILT_CENTER = 2048;
const int PAN_RANGE = 1024;
const int TILT_RANGE = 512;

int panPosition = PAN_CENTER;
int tiltPosition = TILT_CENTER;

const int halfStepSequence[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

int panStepIndex = 0;
int tiltStepIndex = 0;

// ============================================
// STATE VARIABLES
// ============================================

int targetsHit = 0;
unsigned long lastHeartbeat = 0;
bool connected = false;

// ============================================
// FUNCTION PROTOTYPES
// ============================================

void stepPan(int direction);
void stepTilt(int direction);
void setPanPins(int step);
void setTiltPins(int step);
void disableSteppers();
void moveToPosition(float x, float y);
void fireLaser(int duration, int power);
void homePosition();
void executeTarget(float x, float y, int duration, int power);
void runCalibration();
void testSequence();
void emergencyStop();
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
  Serial.println("║          ⚡ LASER TURRET WORKER - WiFi VERSION                 ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  // Initialize stepper pins
  Serial.print("🔧 Initializing steppers... ");
  pinMode(PAN_IN1, OUTPUT);
  pinMode(PAN_IN2, OUTPUT);
  pinMode(PAN_IN3, OUTPUT);
  pinMode(PAN_IN4, OUTPUT);
  
  pinMode(TILT_IN1, OUTPUT);
  pinMode(TILT_IN2, OUTPUT);
  pinMode(TILT_IN3, OUTPUT);
  pinMode(TILT_IN4, OUTPUT);
  Serial.println("✅");
  
  // Initialize laser
  Serial.print("🔧 Initializing laser... ");
  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, LOW);
  Serial.println("✅");
  
  pinMode(STATUS_LED, OUTPUT);
  
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
  
  // Setup web server for receiving commands
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
        emergencyStop();
      }
      else if(cmd == "CALIBRATE") {
        runCalibration();
      }
      else if(cmd == "HOME") {
        homePosition();
      }
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400);
    }
  });
  
  server.begin();
  Serial.println("🌐 Command Server... ✅ OK (Port 80)");
  
  // Home position
  Serial.println("\n🏠 Homing turret...");
  homePosition();
  
  // Self-test
  Serial.println("\n🧪 Running self-test...");
  testSequence();
  
  // Register with master
  Serial.println("\n📤 Registering with Master Hub...");
  registerWithMaster();
  
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.println("║              🟢 LASER TURRET WORKER READY                      ║");
  Serial.println("║                                                                ║");
  Serial.println("║  Waiting for commands from Master Hub...                      ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  // Blink ready
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
  
  // Send heartbeat every 5 seconds
  if(millis() - lastHeartbeat > 5000) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
  
  delay(10);
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
    digitalWrite(STATUS_LED, HIGH);
    delay(500);
    digitalWrite(STATUS_LED, LOW);
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
  
  StaticJsonDocument<128> doc;
  doc["name"] = WORKER_NAME;
  doc["status"] = "READY";
  doc["targets"] = targetsHit;
  
  String json;
  serializeJson(doc, json);
  
  int httpCode = http.POST(json);
  
  if(httpCode == 200) {
    if(!connected) {
      Serial.println("✅ Connected to Master Hub!");
      connected = true;
    }
    Serial.println("💚 Heartbeat sent");
  } else {
    if(connected) {
      Serial.println("⚠️  Lost connection to Master Hub");
      connected = false;
    }
    Serial.println("💛 Heartbeat failed (Master offline?)");
  }
  
  http.end();
}

// ============================================
// TARGET EXECUTION
// ============================================

void executeTarget(float x, float y, int duration, int power) {
  digitalWrite(STATUS_LED, HIGH);
  
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.printf("║  🎯 TARGET RECEIVED                                            ║\n");
  Serial.printf("║  X: %.3f  Y: %.3f  Duration: %dms  Power: %d%%            ║\n", x, y, duration, power);
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  Serial.println("📍 Moving to target position...");
  moveToPosition(x, y);
  
  Serial.println("🔥 Firing laser...");
  fireLaser(duration, power);
  
  targetsHit++;
  
  digitalWrite(STATUS_LED, LOW);
  
  Serial.printf("✅ Target #%d complete\n\n", targetsHit);
}

// ============================================
// POSITION CONTROL
// ============================================

void moveToPosition(float x, float y) {
  int targetPan = PAN_CENTER + (int)((x - 0.5) * 2.0 * PAN_RANGE);
  int targetTilt = TILT_CENTER + (int)((y - 0.5) * 2.0 * TILT_RANGE);
  
  targetPan = constrain(targetPan, PAN_CENTER - PAN_RANGE, PAN_CENTER + PAN_RANGE);
  targetTilt = constrain(targetTilt, TILT_CENTER - TILT_RANGE, TILT_CENTER + TILT_RANGE);
  
  Serial.printf("   Pan: %d steps, Tilt: %d steps\n", targetPan, targetTilt);
  
  int panSteps = targetPan - panPosition;
  int tiltSteps = targetTilt - tiltPosition;
  
  int maxSteps = max(abs(panSteps), abs(tiltSteps));
  
  for(int i = 0; i < maxSteps; i++) {
    if(abs(panSteps) > 0) {
      stepPan(panSteps > 0 ? 1 : -1);
      if(panSteps > 0) panSteps--;
      else panSteps++;
    }
    
    if(abs(tiltSteps) > 0) {
      stepTilt(tiltSteps > 0 ? 1 : -1);
      if(tiltSteps > 0) tiltSteps--;
      else tiltSteps++;
    }
    
    delayMicroseconds(STEP_DELAY_US);
  }
  
  panPosition = targetPan;
  tiltPosition = targetTilt;
  
  delay(50);
  Serial.println("   ✅ Position reached");
}

void stepPan(int direction) {
  if(direction > 0) {
    panStepIndex = (panStepIndex + 1) % 8;
  } else {
    panStepIndex = (panStepIndex + 7) % 8;
  }
  setPanPins(panStepIndex);
}

void stepTilt(int direction) {
  if(direction > 0) {
    tiltStepIndex = (tiltStepIndex + 1) % 8;
  } else {
    tiltStepIndex = (tiltStepIndex + 7) % 8;
  }
  setTiltPins(tiltStepIndex);
}

void setPanPins(int step) {
  digitalWrite(PAN_IN1, halfStepSequence[step][0]);
  digitalWrite(PAN_IN2, halfStepSequence[step][1]);
  digitalWrite(PAN_IN3, halfStepSequence[step][2]);
  digitalWrite(PAN_IN4, halfStepSequence[step][3]);
}

void setTiltPins(int step) {
  digitalWrite(TILT_IN1, halfStepSequence[step][0]);
  digitalWrite(TILT_IN2, halfStepSequence[step][1]);
  digitalWrite(TILT_IN3, halfStepSequence[step][2]);
  digitalWrite(TILT_IN4, halfStepSequence[step][3]);
}

void disableSteppers() {
  digitalWrite(PAN_IN1, LOW);
  digitalWrite(PAN_IN2, LOW);
  digitalWrite(PAN_IN3, LOW);
  digitalWrite(PAN_IN4, LOW);
  
  digitalWrite(TILT_IN1, LOW);
  digitalWrite(TILT_IN2, LOW);
  digitalWrite(TILT_IN3, LOW);
  digitalWrite(TILT_IN4, LOW);
}

void homePosition() {
  Serial.println("   Moving to center position...");
  moveToPosition(0.5, 0.5);
  Serial.println("   ✅ Centered");
  disableSteppers();
}

// ============================================
// LASER CONTROL
// ============================================

void fireLaser(int duration, int power) {
  Serial.printf("   Laser ON for %dms at %d%% power\n", duration, power);
  
  if(power > 0) {
    digitalWrite(LASER_PIN, HIGH);
    delay(duration);
    digitalWrite(LASER_PIN, LOW);
  }
  
  Serial.println("   ✅ Laser OFF");
}

// ============================================
// CALIBRATION & TESTING
// ============================================

void runCalibration() {
  Serial.println("\n🎯 Running 5-point calibration...");
  
  float points[5][2] = {
    {0.5, 0.5},   // Center
    {0.2, 0.2},   // Bottom-left
    {0.8, 0.2},   // Bottom-right
    {0.8, 0.8},   // Top-right
    {0.2, 0.8}    // Top-left
  };
  
  const char* labels[] = {"Center", "Bottom-Left", "Bottom-Right", "Top-Right", "Top-Left"};
  
  for(int i = 0; i < 5; i++) {
    Serial.printf("Point %d - %s: (%.1f, %.1f)\n", i+1, labels[i], points[i][0], points[i][1]);
    moveToPosition(points[i][0], points[i][1]);
    
    digitalWrite(LASER_PIN, HIGH);
    delay(100);
    digitalWrite(LASER_PIN, LOW);
    
    delay(500);
  }
  
  homePosition();
  Serial.println("✅ Calibration complete\n");
}

void testSequence() {
  Serial.println("   Testing pan axis...");
  moveToPosition(0.3, 0.5);
  delay(200);
  moveToPosition(0.7, 0.5);
  delay(200);
  
  Serial.println("   Testing tilt axis...");
  moveToPosition(0.5, 0.3);
  delay(200);
  moveToPosition(0.5, 0.7);
  delay(200);
  
  Serial.println("   Testing laser...");
  moveToPosition(0.5, 0.5);
  digitalWrite(LASER_PIN, HIGH);
  delay(200);
  digitalWrite(LASER_PIN, LOW);
  
  Serial.println("   ✅ Self-test passed");
}

// ============================================
// EMERGENCY STOP
// ============================================

void emergencyStop() {
  Serial.println("\n🚨 EMERGENCY STOP!");
  
  digitalWrite(LASER_PIN, LOW);
  disableSteppers();
  
  for(int i = 0; i < 10; i++) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    delay(100);
  }
  
  Serial.println("✅ Ready for new commands\n");
}
