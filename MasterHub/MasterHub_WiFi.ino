/*
 * ============================================================================
 * FARMBOT MASTER HUB - WiFi HTTP VERSION (NO ESP-NOW)
 * Simple & Reliable Worker Communication via HTTP
 * ============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// ============================================
// CONFIGURATION
// ============================================

const char* AP_SSID = "FarmBot_Control";
const char* AP_PASSWORD = "L1ncolnTech";
const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

WebServer server(80);
WebSocketsServer webSocket(81);

#define STATUS_LED 2
#define BUZZER_PIN 4

// ============================================
// WORKER TRACKING
// ============================================

struct WorkerInfo {
  String name;
  IPAddress ip;
  bool online;
  unsigned long lastSeen;
  int targetsComplete;
};

WorkerInfo workers[10];  // Support up to 10 workers
int workerCount = 0;

// ============================================
// SYSTEM STATE
// ============================================

enum SystemMode {
  STANDBY,
  MANUAL,
  AUTO,
  CALIBRATE,
  EMERGENCY_STOP
};

SystemMode currentMode = STANDBY;
String applicationMode = "LASER";

struct SystemStats {
  unsigned long uptime;
  int targetsProcessed;
  int laseredWeeds;
  int sprayedWeeds;
  int fertilizedCrops;
  int errors;
} stats;

struct PowerStatus {
  float mainVoltage;
  int batteryPercent;
} powerStatus;

// ============================================
// COMMAND QUEUE
// ============================================

struct Command {
  String target;
  float x, y;
  int duration;
  int power;
  String option;
};

#define COMMAND_QUEUE_SIZE 50
Command commandQueue[COMMAND_QUEUE_SIZE];
int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

// ============================================
// WORKER MANAGEMENT
// ============================================

int findWorker(String name) {
  for(int i = 0; i < workerCount; i++) {
    if(workers[i].name == name) return i;
  }
  return -1;
}

int addWorker(String name, IPAddress ip) {
  int idx = findWorker(name);
  if(idx >= 0) {
    // Update existing
    workers[idx].ip = ip;
    workers[idx].online = true;
    workers[idx].lastSeen = millis();
    return idx;
  }
  
  // Add new
  if(workerCount < 10) {
    workers[workerCount].name = name;
    workers[workerCount].ip = ip;
    workers[workerCount].online = true;
    workers[workerCount].lastSeen = millis();
    workers[workerCount].targetsComplete = 0;
    workerCount++;
    Serial.printf("✅ New worker registered: %s at %s\n", name.c_str(), ip.toString().c_str());
    return workerCount - 1;
  }
  
  return -1;
}

void checkWorkerStatus() {
  for(int i = 0; i < workerCount; i++) {
    if(millis() - workers[i].lastSeen > 15000) {
      if(workers[i].online) {
        workers[i].online = false;
        Serial.printf("⚠️  %s OFFLINE\n", workers[i].name.c_str());
      }
    }
  }
}

// ============================================
// COMMAND QUEUE
// ============================================

void queueCommand(String type, float x, float y, int duration, int power, String option = "") {
  if(queueCount >= COMMAND_QUEUE_SIZE) {
    queueHead = (queueHead + 1) % COMMAND_QUEUE_SIZE;
    queueCount--;
  }
  
  Command& cmd = commandQueue[queueTail];
  cmd.target = type;
  cmd.x = x;
  cmd.y = y;
  cmd.duration = duration;
  cmd.power = power;
  cmd.option = option;
  
  queueTail = (queueTail + 1) % COMMAND_QUEUE_SIZE;
  queueCount++;
}

void processCommandQueue() {
  if(queueCount == 0) return;
  if(currentMode != AUTO && currentMode != MANUAL) return;
  
  static unsigned long lastExec = 0;
  if(millis() - lastExec < 100) return;
  
  Command& cmd = commandQueue[queueHead];
  
  // Find worker
  int idx = findWorker(cmd.target);
  if(idx >= 0 && workers[idx].online) {
    sendCommandToWorker(idx, cmd);
    
    queueHead = (queueHead + 1) % COMMAND_QUEUE_SIZE;
    queueCount--;
    lastExec = millis();
    stats.targetsProcessed++;
    
    if(cmd.target == "LASER") stats.laseredWeeds++;
  } else {
    Serial.printf("⚠️  Worker %s not available\n", cmd.target.c_str());
    queueHead = (queueHead + 1) % COMMAND_QUEUE_SIZE;
    queueCount--;
  }
}

void sendCommandToWorker(int workerIdx, Command& cmd) {
  WiFiClient client;
  
  if(client.connect(workers[workerIdx].ip, 80)) {
    StaticJsonDocument<256> doc;
    doc["cmd"] = "TARGET";
    doc["x"] = cmd.x;
    doc["y"] = cmd.y;
    doc["duration"] = cmd.duration;
    doc["power"] = cmd.power;
    doc["option"] = cmd.option;
    
    String json;
    serializeJson(doc, json);
    
    client.println("POST /command HTTP/1.1");
    client.println("Host: " + workers[workerIdx].ip.toString());
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(json.length());
    client.println();
    client.println(json);
    
    Serial.printf("📤 Sent command to %s: (%.2f, %.2f)\n", 
                  workers[workerIdx].name.c_str(), cmd.x, cmd.y);
  } else {
    Serial.printf("❌ Failed to connect to %s\n", workers[workerIdx].name.c_str());
  }
  
  client.stop();
}

void broadcastCommand(String cmd) {
  for(int i = 0; i < workerCount; i++) {
    if(workers[i].online) {
      WiFiClient client;
      if(client.connect(workers[i].ip, 80)) {
        StaticJsonDocument<128> doc;
        doc["cmd"] = cmd;
        
        String json;
        serializeJson(doc, json);
        
        client.println("POST /command HTTP/1.1");
        client.println("Host: " + workers[i].ip.toString());
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(json.length());
        client.println();
        client.println(json);
      }
      client.stop();
    }
  }
}

// ============================================
// SETUP
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.println("║         FARMBOT MASTER HUB - WiFi HTTP VERSION                ║");
  Serial.println("║         Simple & Reliable Communication                        ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  pinMode(STATUS_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Startup beeps
  for(int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    tone(BUZZER_PIN, 1000, 100);
    delay(200);
    digitalWrite(STATUS_LED, LOW);
    delay(200);
  }
  
  // WiFi AP
  Serial.print("📡 Starting WiFi AP... ");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("✅ OK");
  Serial.printf("   SSID: %s\n", AP_SSID);
  Serial.printf("   Password: %s\n", AP_PASSWORD);
  Serial.printf("   IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("   🌐 Web UI: http://192.168.4.1");
  
  // API Endpoints
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", generateWebUI());
  });
  
  // Worker registration endpoint
  server.on("/api/register", HTTP_POST, []() {
    if(server.hasArg("plain")) {
      StaticJsonDocument<128> doc;
      deserializeJson(doc, server.arg("plain"));
      
      String name = doc["name"];
      IPAddress ip = server.client().remoteIP();
      
      addWorker(name, ip);
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400);
    }
  });
  
  // Worker status update endpoint
  server.on("/api/worker_status", HTTP_POST, []() {
    if(server.hasArg("plain")) {
      StaticJsonDocument<256> doc;
      deserializeJson(doc, server.arg("plain"));
      
      String name = doc["name"];
      int idx = findWorker(name);
      
      if(idx >= 0) {
        workers[idx].lastSeen = millis();
        workers[idx].online = true;
        
        if(doc.containsKey("targets")) {
          workers[idx].targetsComplete = doc["targets"];
        }
      }
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400);
    }
  });
  
  // System status
  server.on("/api/status", HTTP_GET, []() {
    StaticJsonDocument<256> doc;
    doc["mode"] = getModeString();
    doc["targets"] = stats.targetsProcessed;
    doc["battery"] = powerStatus.batteryPercent;
    doc["queue"] = queueCount;
    doc["uptime"] = millis() / 1000;
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });
  
  // Statistics
  server.on("/api/stats", HTTP_GET, []() {
    StaticJsonDocument<256> doc;
    doc["lasered"] = stats.laseredWeeds;
    doc["sprayed"] = stats.sprayedWeeds;
    doc["fertilized"] = stats.fertilizedCrops;
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });
  
  // Workers list
  server.on("/api/workers", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    
    for(int i = 0; i < workerCount; i++) {
      JsonObject w = array.createNestedObject();
      w["name"] = workers[i].name;
      w["online"] = workers[i].online;
      w["ip"] = workers[i].ip.toString();
    }
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });
  
  // Control endpoint
  server.on("/api/control", HTTP_POST, []() {
    if(server.hasArg("plain")) {
      StaticJsonDocument<256> doc;
      deserializeJson(doc, server.arg("plain"));
      
      String cmd = doc["cmd"];
      
      if(cmd == "MODE") {
        String mode = doc["mode"];
        setSystemMode(mode);
      } else if(cmd == "ESTOP") {
        emergencyStop("Web UI");
      }
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400);
    }
  });
  
  // Manual target endpoint
  server.on("/api/target", HTTP_POST, []() {
    float x = server.arg("x").toFloat();
    float y = server.arg("y").toFloat();
    String type = server.arg("type");
    int duration = server.arg("dur").toInt();
    int power = server.arg("pow").toInt();
    
    queueCommand(type, x, y, duration, power);
    
    server.send(200, "application/json", "{\"status\":\"queued\"}");
  });
  
  // Application mode
  server.on("/api/mode", HTTP_POST, []() {
    if(server.hasArg("plain")) {
      StaticJsonDocument<256> doc;
      deserializeJson(doc, server.arg("plain"));
      applicationMode = String(doc["app"].as<const char*>());
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400);
    }
  });
  
  server.begin();
  Serial.println("🌐 Web Server... ✅ OK");
  
  // WebSocket
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if(type == WStype_CONNECTED) {
      Serial.printf("   WebSocket client #%u connected\n", num);
    }
  });
  Serial.println("🔌 WebSocket... ✅ OK (Port 81)");
  
  memset(&stats, 0, sizeof(stats));
  powerStatus.batteryPercent = 100;
  
  Serial.println("\n╔════════════════════════════════════════════════════════════════╗");
  Serial.println("║                  🟢 MASTER HUB ONLINE                          ║");
  Serial.println("║                                                                ║");
  Serial.println("║  🌐 Web UI: http://192.168.4.1                                ║");
  Serial.println("║  Workers will auto-register when they connect                 ║");
  Serial.println("╚════════════════════════════════════════════════════════════════╝\n");
  
  tone(BUZZER_PIN, 2000, 200);
  delay(250);
  tone(BUZZER_PIN, 2500, 200);
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
  stats.uptime = millis();
  
  server.handleClient();
  webSocket.loop();
  processCommandQueue();
  
  static unsigned long lastCheck = 0;
  if(millis() - lastCheck > 10000) {
    checkWorkerStatus();
    lastCheck = millis();
  }
  
  static unsigned long lastTelemetry = 0;
  if(millis() - lastTelemetry > 500) {
    broadcastTelemetry();
    lastTelemetry = millis();
  }
  
  static unsigned long lastBlink = 0;
  if(millis() - lastBlink > 1000) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    lastBlink = millis();
  }
  
  delay(1);
}

// ============================================
// SYSTEM CONTROL
// ============================================

void setSystemMode(String mode) {
  if(mode == "STANDBY") {
    currentMode = STANDBY;
    broadcastCommand("STANDBY");
  }
  else if(mode == "MANUAL") currentMode = MANUAL;
  else if(mode == "AUTO") {
    currentMode = AUTO;
    broadcastCommand("AUTO");
  }
  else if(mode == "CALIBRATE") {
    currentMode = CALIBRATE;
    broadcastCommand("CALIBRATE");
  }
  else if(mode == "ESTOP") emergencyStop("Manual");
  
  Serial.printf("Mode: %s\n", mode.c_str());
}

void emergencyStop(const char* reason) {
  Serial.printf("\n🚨 E-STOP: %s\n", reason);
  currentMode = EMERGENCY_STOP;
  broadcastCommand("STOP");
  
  queueHead = queueTail = queueCount = 0;
  
  for(int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, 3000, 200);
    delay(300);
  }
  
  stats.errors++;
}

// ============================================
// TELEMETRY
// ============================================

void broadcastTelemetry() {
  StaticJsonDocument<512> doc;
  doc["mode"] = getModeString();
  doc["uptime"] = stats.uptime / 1000;
  doc["targets"] = stats.targetsProcessed;
  doc["lasered"] = stats.laseredWeeds;
  doc["sprayed"] = stats.sprayedWeeds;
  doc["fertilized"] = stats.fertilizedCrops;
  doc["battery"] = powerStatus.batteryPercent;
  doc["queue"] = queueCount;
  doc["workersOnline"] = 0;
  
  for(int i = 0; i < workerCount; i++) {
    if(workers[i].online) doc["workersOnline"] = (int)doc["workersOnline"] + 1;
  }
  
  String json;
  serializeJson(doc, json);
  webSocket.broadcastTXT(json);
}

String getModeString() {
  switch(currentMode) {
    case STANDBY: return "STANDBY";
    case MANUAL: return "MANUAL";
    case AUTO: return "AUTO";
    case CALIBRATE: return "CALIBRATE";
    case EMERGENCY_STOP: return "ESTOP";
    default: return "UNKNOWN";
  }
}

// Web UI continues in next message...
String generateWebUI() {
  return R"rawliteral(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>FarmBot Control</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
  font-family: 'Courier New', monospace;
  background: linear-gradient(135deg, #0a0a0a 0%, #001a00 100%);
  color: #00ff00;
  min-height: 100vh;
  padding: 20px;
}
.header {
  text-align: center;
  padding: 20px;
  background: rgba(0,0,0,0.8);
  border: 2px solid #00ff00;
  border-radius: 10px;
  margin-bottom: 20px;
}
.header h1 {
  font-size: 2em;
  background: linear-gradient(90deg, #00ff00, #00ffff);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
}
.dashboard {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
}
.panel {
  background: rgba(26, 26, 26, 0.95);
  border: 2px solid #00ff00;
  border-radius: 10px;
  padding: 20px;
}
.panel h2 {
  color: #00ffff;
  margin-bottom: 15px;
  border-bottom: 1px solid #00ff00;
  padding-bottom: 10px;
}
.status-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 10px;
}
.status-item {
  background: rgba(0, 255, 0, 0.05);
  padding: 10px;
  border-radius: 5px;
  border: 1px solid rgba(0, 255, 0, 0.2);
}
.status-label {
  font-size: 0.8em;
  color: #00ffaa;
  margin-bottom: 5px;
}
.status-value {
  font-size: 1.5em;
  font-weight: bold;
  color: #00ff00;
}
.btn {
  background: linear-gradient(135deg, #00ff00, #00cc00);
  color: #000;
  border: none;
  padding: 12px 20px;
  border-radius: 5px;
  font-weight: bold;
  cursor: pointer;
  width: 100%;
  margin-top: 10px;
}
.btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 5px 15px rgba(0, 255, 0, 0.4);
}
.btn-danger {
  background: linear-gradient(135deg, #ff0044, #cc0033);
  color: #fff;
}
.control-group {
  margin: 10px 0;
}
.control-label {
  display: flex;
  justify-content: space-between;
  margin-bottom: 5px;
  font-size: 0.9em;
}
.slider {
  width: 100%;
  height: 8px;
  border-radius: 5px;
  background: linear-gradient(90deg, rgba(0, 255, 0, 0.2), #00ff00);
  outline: none;
}
.worker-list {
  display: grid;
  gap: 8px;
}
.worker-item {
  display: flex;
  justify-content: space-between;
  background: rgba(0, 255, 0, 0.05);
  padding: 10px;
  border-radius: 5px;
  border-left: 4px solid transparent;
}
.worker-item.online {
  border-left-color: #00ff00;
}
.worker-item.offline {
  border-left-color: #ff0044;
  opacity: 0.5;
}
.status-dot {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  background: #ff0044;
  display: inline-block;
  margin-right: 5px;
}
.status-dot.online {
  background: #00ff00;
  box-shadow: 0 0 10px #00ff00;
}
</style>
</head>
<body>

<div class="header">
  <h1>🚜 FARMBOT CONTROL CENTER</h1>
  <div style="color:#00ffaa;margin-top:10px">WiFi HTTP Communication</div>
</div>

<div class="dashboard">
  
  <div class="panel">
    <h2>SYSTEM STATUS</h2>
    <div class="status-grid">
      <div class="status-item">
        <div class="status-label">Mode</div>
        <div class="status-value" id="sysMode">STANDBY</div>
      </div>
      <div class="status-item">
        <div class="status-label">Uptime</div>
        <div class="status-value" id="uptime">0s</div>
      </div>
      <div class="status-item">
        <div class="status-label">Targets</div>
        <div class="status-value" id="targets">0</div>
      </div>
      <div class="status-item">
        <div class="status-label">Queue</div>
        <div class="status-value" id="queue">0</div>
      </div>
    </div>
  </div>

  <div class="panel">
    <h2>CONTROL</h2>
    <button class="btn" onclick="setMode('STANDBY')">⏸️ STANDBY</button>
    <button class="btn" onclick="setMode('MANUAL')">🕹️ MANUAL</button>
    <button class="btn" onclick="setMode('AUTO')">🤖 AUTO</button>
    <button class="btn btn-danger" onclick="emergencyStop()">🚨 E-STOP</button>
  </div>

  <div class="panel">
    <h2>MANUAL TARGETING</h2>
    <div class="control-group">
      <div class="control-label">
        <span>X Position</span>
        <span id="xVal">0.50</span>
      </div>
      <input type="range" class="slider" id="xSlider" min="0" max="1" step="0.01" value="0.5">
    </div>
    <div class="control-group">
      <div class="control-label">
        <span>Y Position</span>
        <span id="yVal">0.50</span>
      </div>
      <input type="range" class="slider" id="ySlider" min="0" max="1" step="0.01" value="0.5">
    </div>
    <div class="control-group">
      <div class="control-label">
        <span>Duration (ms)</span>
        <span id="durVal">50</span>
      </div>
      <input type="range" class="slider" id="durSlider" min="10" max="500" step="10" value="50">
    </div>
    <div class="control-group">
      <div class="control-label">
        <span>Power (%)</span>
        <span id="powVal">100</span>
      </div>
      <input type="range" class="slider" id="powSlider" min="0" max="100" step="5" value="100">
    </div>
    <button class="btn" onclick="manualFire()">🎯 FIRE TARGET</button>
  </div>

  <div class="panel">
    <h2>WORKER MODULES</h2>
    <div class="worker-list" id="workerList">
      <div style="color:#00ffaa;">Waiting for workers...</div>
    </div>
  </div>

  <div class="panel">
    <h2>STATISTICS</h2>
    <div class="status-grid">
      <div class="status-item">
        <div class="status-label">Lasered</div>
        <div class="status-value" id="statsLaser">0</div>
      </div>
      <div class="status-item">
        <div class="status-label">Sprayed</div>
        <div class="status-value" id="statsSpray">0</div>
      </div>
    </div>
  </div>

</div>

<script>
let ws;
let currentApp = 'LASER';

document.getElementById('xSlider').oninput = function() {
  document.getElementById('xVal').innerText = this.value;
};
document.getElementById('ySlider').oninput = function() {
  document.getElementById('yVal').innerText = this.value;
};
document.getElementById('durSlider').oninput = function() {
  document.getElementById('durVal').innerText = this.value;
};
document.getElementById('powSlider').oninput = function() {
  document.getElementById('powVal').innerText = this.value;
};

function connectWebSocket() {
  ws = new WebSocket('ws://' + location.hostname + ':81');
  ws.onmessage = function(e) {
    try {
      const data = JSON.parse(e.data);
      if(data.mode) document.getElementById('sysMode').innerText = data.mode;
      if(data.uptime !== undefined) document.getElementById('uptime').innerText = formatUptime(data.uptime);
      if(data.targets !== undefined) document.getElementById('targets').innerText = data.targets;
      if(data.queue !== undefined) document.getElementById('queue').innerText = data.queue;
      if(data.lasered !== undefined) document.getElementById('statsLaser').innerText = data.lasered;
      if(data.sprayed !== undefined) document.getElementById('statsSpray').innerText = data.sprayed;
    } catch(err) {}
  };
  ws.onclose = function() {
    setTimeout(connectWebSocket, 3000);
  };
}

function formatUptime(seconds) {
  const h = Math.floor(seconds / 3600);
  const m = Math.floor((seconds % 3600) / 60);
  const s = seconds % 60;
  return h > 0 ? `${h}h ${m}m` : m > 0 ? `${m}m ${s}s` : `${s}s`;
}

function setMode(mode) {
  fetch('/api/control', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({cmd: 'MODE', mode: mode})
  });
}

function manualFire() {
  const x = document.getElementById('xSlider').value;
  const y = document.getElementById('ySlider').value;
  const dur = document.getElementById('durSlider').value;
  const pow = document.getElementById('powSlider').value;
  
  fetch(`/api/target?type=${currentApp}&x=${x}&y=${y}&dur=${dur}&pow=${pow}`, {
    method: 'POST'
  });
}

function emergencyStop() {
  if(confirm('⚠️ EMERGENCY STOP ALL SYSTEMS?')) {
    fetch('/api/control', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({cmd: 'ESTOP'})
    });
  }
}

function updateWorkers() {
  fetch('/api/workers')
    .then(r => r.json())
    .then(workers => {
      const list = document.getElementById('workerList');
      if(workers.length === 0) {
        list.innerHTML = '<div style="color:#00ffaa;">Waiting for workers...</div>';
        return;
      }
      list.innerHTML = '';
      workers.forEach(w => {
        const item = document.createElement('div');
        item.className = 'worker-item ' + (w.online ? 'online' : 'offline');
        item.innerHTML = `
          <span>${w.name}</span>
          <span>
            <span class="status-dot ${w.online ? 'online' : ''}"></span>
            ${w.online ? 'ONLINE' : 'OFFLINE'}
          </span>
        `;
        list.appendChild(item);
      });
    });
}

connectWebSocket();
setInterval(updateWorkers, 2000);
updateWorkers();
</script>

</body>
</html>)rawliteral";
}
