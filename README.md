# 🚜 FarmBot Modular Agricultural Robotics System

**A professional-grade, modular agricultural automation platform built on ESP32 microcontrollers with WiFi-based inter-module communication.**

Developed by **Demetri Cherpelis** - Instructor, Electrical Systems Technology - Mechatronics Program  
Lincoln Technical Institute, Queens, NY

---

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation & Setup](#installation--setup)
- [Usage Guide](#usage-guide)
- [Worker Modules](#worker-modules)
- [Development Timeline](#development-timeline)
- [Educational Applications](#educational-applications)
- [Future Roadmap](#future-roadmap)
- [Contributing](#contributing)
- [License](#license)

---

## 🎯 Overview

The FarmBot Modular Agricultural Robotics System is a scalable platform designed for precision agriculture applications. The system enables automated weed control, targeted pesticide/fertilizer application, and crop monitoring through a distributed network of specialized worker modules coordinated by a central Master Hub.

### Key Innovations:
- **Modular Architecture**: Hot-swappable worker modules for different tasks
- **WiFi Communication**: Simple, reliable HTTP-based inter-module protocol
- **Web-Based Control**: Professional UI accessible from any device
- **Educational Focus**: Designed for teaching industrial automation and robotics
- **Production-Ready**: Safety systems, error handling, and professional code quality

### Project Goals:
1. Demonstrate advanced agricultural automation techniques
2. Provide hands-on learning platform for mechatronics students
3. Enable precision agriculture with sub-centimeter accuracy
4. Reduce chemical usage through targeted application
5. Create scalable, maintainable automation systems

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     MASTER HUB (ESP32)                      │
│  - WiFi Access Point (192.168.4.1)                         │
│  - Web Server + WebSocket Telemetry                        │
│  - Worker Registration & Management                        │
│  - Command Queue & Distribution                            │
│  - Real-time Monitoring Dashboard                          │
└─────────────┬───────────────────────────────────────────────┘
              │
              │ WiFi Network: "FarmBot_Control"
              │
    ┌─────────┼─────────┬─────────┬─────────┬─────────┐
    │         │         │         │         │         │
┌───▼───┐ ┌──▼────┐ ┌──▼────┐ ┌──▼────┐ ┌──▼────┐ ┌──▼────┐
│ LASER │ │ SPRAY │ │MOTION │ │ ENV.  │ │SAFETY │ │ POWER │
│Worker │ │Worker │ │Worker │ │Sensor │ │Monitor│ │Monitor│
└───────┘ └───────┘ └───────┘ └───────┘ └───────┘ └───────┘

Each worker:
  • Connects to Master Hub WiFi
  • Auto-registers on startup
  • Receives commands via HTTP POST
  • Sends heartbeat every 5 seconds
  • Executes specialized tasks
```

### Communication Protocol:
- **Worker → Master**: HTTP POST to `/api/register` (registration) and `/api/worker_status` (heartbeat)
- **Master → Worker**: HTTP POST to `/command` with JSON payload
- **User → Master**: Web UI via HTTP on port 80, WebSocket on port 81

---

## ✨ Features

### Master Hub Features:
- ✅ **Web-Based Control Interface** - Cyberpunk-themed professional dashboard
- ✅ **Real-Time Telemetry** - WebSocket updates every 500ms
- ✅ **Worker Management** - Auto-discovery and status monitoring
- ✅ **Command Queueing** - 50-slot FIFO queue for smooth operation
- ✅ **Multiple Operating Modes** - Standby, Manual, Auto, Calibrate, E-Stop
- ✅ **Statistics Tracking** - Targets processed, treatments applied, uptime
- ✅ **Emergency Stop System** - Immediate shutdown of all workers
- ✅ **Responsive Design** - Works on desktop, tablet, and mobile

### Worker Module Features:
- ✅ **Hot-Swappable** - Add/remove workers without system restart
- ✅ **Self-Registering** - Automatic discovery and configuration
- ✅ **Health Monitoring** - Heartbeat system with offline detection
- ✅ **Standardized Interface** - Consistent command structure across all workers
- ✅ **Local Processing** - Autonomous execution of received commands
- ✅ **Error Recovery** - WiFi reconnection and fault handling

### Safety Features:
- ✅ **Emergency Stop** - Hardware and software E-stop on all modules
- ✅ **Timeout Protection** - Auto-disable if connection lost
- ✅ **Laser Safety** - Multiple interlocks and enable signals
- ✅ **Temperature Monitoring** - Thermal shutdown protection
- ✅ **Position Limits** - Software bounds checking
- ✅ **Visual Indicators** - LED status on all modules

---

## 🔧 Hardware Requirements

### Master Hub:
- **ESP32 DevKit** (any variant with WiFi)
- **microSD Card Module** (optional, for logging)
- **Status LED** (GPIO 2)
- **Buzzer** (GPIO 4)
- **Power Supply**: 5V USB or regulated 3.3V

### Laser Turret Worker (Stepper Version):
- **ESP32 DevKit**
- **2x 28BYJ-48 Stepper Motors** (Pan & Tilt axes)
- **2x ULN2003 Driver Boards**
- **Laser Pointer Module** (3.3V/5V compatible)
- **MOSFET** (for laser control, optional)
- **Status LED** (GPIO 26)
- **Power Supply**: 12V for steppers, 5V for ESP32

**Wiring Diagram:**
```
Pan Stepper (X-axis):
  GPIO 19/18/5/17 → ULN2003 IN1-4

Tilt Stepper (Y-axis):
  GPIO 16/4/2/15 → ULN2003 IN1-4

Laser Control:
  GPIO 25 → Laser Module (+) or MOSFET Gate

Status LED:
  GPIO 26 → LED → 220Ω → GND
```

### Galvo Laser Worker (High-Speed Version):
- **ESP32 DevKit** (must have DAC pins: GPIO25, GPIO26)
- **Galvo Mirror System** (X/Y galvanometers)
- **2x Galvo Driver Boards** (analog control 0-3.3V)
- **High-Power Laser Module** (5W-40W)
- **Laser Driver** (PWM/TTL control)
- **Safety System**:
  - E-Stop Button
  - Door Interlock Switch
  - Temperature Sensor (TMP36 or similar)
- **Status Indicators**:
  - System Ready LED (Green)
  - Laser Active LED (Red)
  - Status LED
- **Power Supply**: Appropriate for laser power rating

**Galvo Wiring:**
```
Galvo Control (FIXED PINS - DAC only on these):
  GPIO 25 (DAC1) → X-Axis Galvo Driver (Analog)
  GPIO 26 (DAC2) → Y-Axis Galvo Driver (Analog)

Laser Control:
  GPIO 32 → Laser Driver PWM Input
  GPIO 33 → Laser Enable (HIGH = armed)
  GPIO 27 → Laser TTL Trigger

Safety System:
  GPIO 35 ← E-Stop Button (active LOW, pullup)
  GPIO 34 ← Interlock Switch (HIGH = safe)
  GPIO 36 ← Temperature Sensor (analog)

Status LEDs:
  GPIO 2  → Status LED (Green)
  GPIO 15 → Laser Active LED (RED)
  GPIO 4  → Ready LED (Green)
```

### Future Worker Modules (Planned):
- **Spray Worker**: Solenoid valves, pressure sensor, chemical tanks
- **Motion Worker**: Stepper/servo motors for X/Y/Z gantry movement
- **Environment Sensor**: Temp, humidity, wind speed, soil moisture
- **Safety Monitor**: Proximity sensors, fence monitoring
- **Power Monitor**: Voltage/current sensing, battery management

---

💻 Software Requirements

Arduino IDE Setup:
1. **Arduino IDE 2.x** or higher
2. **ESP32 Board Support**:
   - Add to Board Manager URLs: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Install: **ESP32 by Espressif Systems**
   - Board: **ESP32 Dev Module**
   - **CRITICAL**: Use **ESP32 Core 2.x.x** (NOT 3.x.x for this version)

Required Libraries:
Install via Arduino Library Manager:
- **WiFi** (built-in with ESP32)
- **WebServer** (built-in with ESP32)
- **WebSocketsServer** by Markus Sattler (v2.3.6+)
- **ArduinoJson** by Benoit Blanchon (v6.21.0+)
- **HTTPClient** (built-in with ESP32)

Optional (for SD logging):
- **SD** (built-in)
- **SPI** (built-in)

---

## 🚀 Installation & Setup

### 1. Clone Repository
```bash
git clone https://github.com/yourusername/farmbot-modular-robotics.git
cd farmbot-modular-robotics
```

### 2. Upload Master Hub
```bash
# Open Arduino IDE
File → Open → MasterHub_WiFi/MasterHub_WiFi.ino

# Configure
Tools → Board → ESP32 Dev Module
Tools → Upload Speed → 115200
Tools → Port → [Select your ESP32]

# Upload
Sketch → Upload
```

**Expected Serial Output:**
```
╔════════════════════════════════════════════════════════════════╗
║         FARMBOT MASTER HUB - WiFi HTTP VERSION                ║
║         Simple & Reliable Communication                        ║
╚════════════════════════════════════════════════════════════════╝

📡 Starting WiFi AP... ✅ OK
   SSID: FarmBot_Control
   Password: L1ncolnTech
   IP: 192.168.4.1
   🌐 Web UI: http://192.168.4.1

🌐 Web Server... ✅ OK
🔌 WebSocket... ✅ OK (Port 81)

╔════════════════════════════════════════════════════════════════╗
║                  🟢 MASTER HUB ONLINE                          ║
║                                                                ║
║  🌐 Open http://192.168.4.1 in your browser                   ║
║  Workers will auto-connect when powered on                    ║
╚════════════════════════════════════════════════════════════════╝
```

### 3. Upload Worker Module
```bash
# Choose worker type:
# - Worker_WiFi_Stepper_Laser.ino (for 28BYJ-48 turret)
# - Worker_WiFi_Galvo_Laser.ino (for galvo mirrors)

File → Open → [Worker sketch]

# Upload to worker ESP32
Sketch → Upload
```

**Expected Worker Serial Output:**
```
╔════════════════════════════════════════════════════════════════╗
║          ⚡ LASER TURRET WORKER - WiFi VERSION                 ║
╚════════════════════════════════════════════════════════════════╝

🔧 Initializing steppers... ✅
🔧 Initializing laser... ✅
📡 Connecting to Master Hub WiFi... ✅
   IP Address: 192.168.4.2
🌐 Command Server... ✅ OK (Port 80)

🏠 Homing turret...
   Moving to center position...
   ✅ Centered

🧪 Running self-test...
   Testing pan axis...
   Testing tilt axis...
   Testing laser...
   ✅ Self-test passed

📤 Registering with Master Hub...
✅ Registered with Master Hub!

╔════════════════════════════════════════════════════════════════╗
║              🟢 LASER TURRET WORKER READY                      ║
║                                                                ║
║  Waiting for commands from Master Hub...                      ║
╚════════════════════════════════════════════════════════════════╝
```

### 4. Access Web Interface
1. Connect your device (phone/computer) to WiFi: **FarmBot_Control**
2. Password: **L1ncolnTech**
3. Open browser to: **http://192.168.4.1**
4. You should see the FarmBot Control Center dashboard

---

## 📖 Usage Guide

### Operating Modes:

**STANDBY Mode:**
- System idle, workers maintain position
- No commands executed
- Used during setup and breaks

**MANUAL Mode:**
- Direct control via web interface
- Use sliders to position targeting system
- Click "FIRE TARGET" to execute
- Ideal for testing and calibration

**AUTO Mode:**
- Processes command queue automatically
- Commands from Jetson Nano or other AI controller
- Production operation mode
- 50 commands can be queued

**CALIBRATE Mode:**
- Runs calibration patterns on workers
- Tests full range of motion
- Verifies positioning accuracy
- Should be run after hardware changes

**E-STOP (Emergency Stop):**
- Immediately halts all operations
- Disables lasers and motors
- Clears command queue
- Must manually reset to continue

### Web Interface Controls:

**Manual Targeting Panel:**
- **X Position Slider**: 0.0 (left) to 1.0 (right)
- **Y Position Slider**: 0.0 (bottom) to 1.0 (top)
- **Duration Slider**: 10-500ms laser pulse duration
- **Power Slider**: 0-100% laser power
- **Fire Target Button**: Execute manual command

**System Status Display:**
- **Mode**: Current operating mode
- **Uptime**: System runtime
- **Targets**: Total targets processed
- **Queue**: Commands waiting in queue

**Worker Modules Panel:**
- Shows all registered workers
- Green dot = Online, Red dot = Offline
- Updates every 2 seconds

**Statistics Panel:**
- Lasered weeds count
- Sprayed weeds count
- Fertilized crops count

### Adding New Workers:
1. Upload worker code to new ESP32
2. Power on the worker
3. Wait 10-15 seconds for auto-registration
4. Check Web UI - worker should appear as ONLINE
5. Test with manual command

---

## 🤖 Worker Modules

### Currently Implemented:

#### 1. Laser Turret Worker (Stepper)
**Function**: Precision weed targeting with laser ablation  
**Hardware**: 28BYJ-48 steppers (Pan/Tilt), laser pointer  
**Speed**: 5-10 targets/second  
**Accuracy**: ±2 degrees  
**Range**: ±90° pan, ±45° tilt  

**Code**: `Worker_WiFi_Stepper_Laser.ino`

#### 2. Galvo Laser Worker (High-Speed)
**Function**: Ultra-fast precision targeting  
**Hardware**: Galvo mirrors, high-power laser (5W-40W)  
**Speed**: 50-100+ targets/second  
**Accuracy**: ±0.01 degrees  
**Range**: Configurable via calibration  

**Code**: `Worker_WiFi_Galvo_Laser.ino`

**Safety Features**:
- E-Stop button (immediate shutdown)
- Safety interlock (door/panel monitoring)
- Temperature monitoring (auto-shutdown at 65°C)
- Multi-layer laser enable logic
- Continuous safety checks every loop cycle

### Planned Future Workers:

#### 3. Spray Worker
**Function**: Targeted pesticide/fertilizer application  
**Hardware**: Solenoid valves, pressure sensor, nozzles  
**Features**:
- Multiple chemical reservoirs
- Precise volume control
- Wind speed monitoring
- Safety interlocks

#### 4. Motion Worker (X/Y/Z Gantry)
**Function**: Physical positioning system  
**Hardware**: NEMA 17 steppers, linear rails, belt drive  
**Features**:
- Multi-axis coordinated motion
- Homing switches
- Position feedback
- Emergency stop

#### 5. Environment Sensor Worker
**Function**: Real-time environmental monitoring  
**Hardware**: DHT22, soil moisture, wind speed, light sensor  
**Features**:
- Temperature/humidity logging
- Spray window calculation
- Data trending
- Alert system

#### 6. Safety Monitor Worker
**Function**: Perimeter and system safety  
**Hardware**: Proximity sensors, fence monitoring, cameras  
**Features**:
- Intrusion detection
- Safety zone monitoring
- Worker health checks
- Automatic shutdown on breach

#### 7. Power Monitor Worker
**Function**: Power system management  
**Hardware**: INA219, voltage/current sensors, battery monitor  
**Features**:
- Battery state of charge
- Solar panel monitoring
- Low battery warnings
- Power consumption tracking

---

# ⏱️ Development Timeline

# Project 

Week 1: Architecture & Core Systems 
- Day 1-2: System architecture design, protocol definition (16h)
- Day 3-4: Master Hub development and testing (12h)
- Day 5-6: Worker template and WiFi communication (12h)

Week 2: Worker Implementation 
- Day 1-2: Stepper laser worker development (16h)
- Day 3-4: Galvo laser worker development (20h)
- Day 5-6: Safety systems and interlocks (14h)

Week 3: Integration & Documentation 
- Day 1-2: System integration testing (10h)
- Day 3-4: Web UI refinement (8h)
- Day 5: Documentation and GitHub preparation (12h)

### Key Milestones:
- ✅ **Nov 2024**: Initial ESP-NOW prototype (later abandoned)
- ✅ **Dec 2024**: WiFi HTTP architecture redesign
- ✅ **Dec 2024**: Master Hub web interface complete
- ✅ **Jan 2025**: Stepper laser worker operational
- ✅ **Jan 2025**: Galvo laser worker with safety systems
- ✅ **Jan 2025**: Production-ready release

### Labor Breakdown:
- **Software Development**: ~80 hours
- **Hardware Integration**: ~20 hours
- **Testing & Debugging**: ~15 hours
- **Documentation**: ~5 hours

---

## 🎓 Educational Applications

This project was developed as part of the **Electrical Systems Technology - Mechatronics Program** at Lincoln Technical Institute, Queens, NY.

### Learning Objectives Covered:

**1. Industrial Communication Protocols**
- WiFi networking fundamentals
- HTTP request/response architecture
- JSON data serialization
- WebSocket real-time communication

**2. Embedded Systems Programming**
- ESP32 microcontroller architecture
- Multi-tasking on embedded platforms
- Interrupt handling and timing
- Memory management

**3. Motor Control Systems**
- Stepper motor control (28BYJ-48)
- Galvanometer mirror systems
- PWM for power control
- Position feedback and calibration

**4. Safety System Design**
- Hardware interlocks
- E-stop circuits
- Fault detection and recovery
- Multi-layer safety architecture

**5. Robotics & Automation**
- Modular system architecture
- Command queue management
- Real-time control systems
- Precision positioning

**6. Web Development for IoT**
- Responsive web interfaces
- Real-time data visualization
- REST API design
- WebSocket telemetry

### Classroom Integration:
- **Lab Exercises**: Students build individual workers
- **Group Projects**: Teams integrate full system
- **Capstone Projects**: Students add new worker types
- **Assessment**: Hands-on troubleshooting and calibration

### Skills Developed:
✅ C/C++ programming  
✅ Electronics and circuit design  
✅ Network communication  
✅ System integration  
✅ Safety system design  
✅ Professional documentation  
✅ Version control (Git)  
✅ Troubleshooting methodologies  

---

## 🗺️ Future Roadmap

### Phase 1: Additional Workers (Q1 2025)
- [ ] Spray worker implementation
- [ ] Motion worker (X/Y gantry)
- [ ] Environment sensor array
- [ ] Power monitoring system

### Phase 2: AI Integration (Q2 2025)
- [ ] Jetson Nano integration
- [ ] Computer vision for weed detection
- [ ] Machine learning target classification
- [ ] Autonomous decision-making

### Phase 3: Advanced Features (Q3 2025)
- [ ] Multi-robot coordination
- [ ] Path planning algorithms
- [ ] Field mapping and coverage tracking
- [ ] Data logging and analytics

### Phase 4: Production Deployment (Q4 2025)
- [ ] Weatherproof enclosures
- [ ] Solar power system
- [ ] Field testing program
- [ ] Performance optimization

### Long-Term Vision:
- **Open-source community**: Build ecosystem of compatible workers
- **Commercial deployment**: Partner with farms for pilot programs
- **Curriculum development**: Create comprehensive teaching materials
- **Certification program**: Industry-recognized training course

---

## 🤝 Contributing

This is an educational open-source project. Contributions are welcome!

How to Contribute:

For Students:
1. Build a new worker module
2. Document your design process
3. Submit pull request with code + documentation
4. Present at student showcase

For Educators:
1. Create lab exercises
2. Develop assessment materials
3. Share classroom implementation strategies
4. Contribute to curriculum guide

For Developers:
1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

Contribution Guidelines:
- Follow existing code style and structure
- Include comprehensive comments
- Test on actual hardware before submitting
- Update documentation for any new features
- Add photos/videos of working hardware

### Areas Needing Help:
- Additional worker module implementations
- Web UI enhancements
- Mobile app development
- PCB designs for worker modules
- 3D printable enclosures
- Documentation improvements
- Translation to other languages

---

📄 License

MIT License

Copyright (c) 2025 Kanenas - Lincoln Technical Institute

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## 📞 Contact & Support

Demetri Cherpelis - Kanenas  DCherpels@lincolntech.edu
Instructor, Electrical Systems Technology - 
Lincoln Technical Institute  
Queens, New York

Project Repository: [[GitHub Link]  ](https://github.com/KANENAS1/Farmbot/tree/main)
Issues & Bug Reports: [DCherpelis@lincolntech.edu]  
Discussions: [N/A]  

Acknowledgments:
- Lincoln Technical Institute for supporting innovative curriculum development
- The few Students who tested and provided feedback on the system
- Open-source community for excellent libraries and tools

---

## ⚠️ Safety Disclaimers

**IMPORTANT SAFETY INFORMATION:

⚠️ **Laser Safety**: High-power lasers can cause permanent eye damage and fire hazards. Always use:
- Proper laser safety goggles rated for your laser wavelength
- Enclosed operating area with laser warning signs
- Safety interlocks on all access panels
- Never operate without proper training

⚠️ **Electrical Safety**: System operates on mains voltage in some configurations:
- Only qualified personnel should wire power systems
- Follow local electrical codes
- Use proper fusing and circuit protection
- Ensure proper grounding

⚠️ **Chemical Safety**: Spray systems may use hazardous chemicals:
- Follow MSDS guidelines for all chemicals
- Use proper PPE (gloves, respirator, etc.)
- Ensure adequate ventilation
- Dispose of chemicals properly

⚠️ **Mechanical Safety**: Moving parts can cause injury:
- Keep hands clear of moving components
- Use emergency stops
- Proper guarding on pinch points
- Never bypass safety interlocks

*This system is intended for educational and research purposes. Users assume all responsibility for safe operation.

---

## 🌟 Star History

If this project helps you or your students, please consider giving it a ⭐!

[![Star History Chart](https://api.star-history.com/svg?repos=yourusername/farmbot-modular-robotics&type=Date)](https://star-history.com/#yourusername/farmbot-modular-robotics&Date)

---

Built with ❤️ for agricultural innovation and hands-on education

Last Updated: January 2026
