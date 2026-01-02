# ⚡ Quick Start Guide

Get up and running with FarmBot in **15 minutes**!

---

## 🎯 What You'll Build

A working agricultural robotics system with:
- ✅ Master control hub with web interface
- ✅ Laser targeting worker module  
- ✅ Real-time communication over WiFi

---

## 📦 Required Parts (Minimum System)

**Order these first:**
1. **2x ESP32 DevKit boards** (~$20)
2. **2x 28BYJ-48 Stepper Motors with ULN2003 drivers** (~$10)
3. **1x Laser pointer module** (~$5)
4. **Jumper wires** (~$5)
5. **Breadboards** (~$5)
6. **12V power supply** (~$10)

**Total: ~$55**

---

## 🚀 5-Step Setup

### Step 1: Install Arduino IDE (5 min)

1. Download Arduino IDE 2.x from [arduino.cc](https://www.arduino.cc/en/software)
2. Install ESP32 support:
   - Open Arduino IDE
   - Go to: **File → Preferences**
   - Add to "Additional Board Manager URLs":
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Go to: **Tools → Board → Boards Manager**
   - Search "ESP32" and install **ESP32 by Espressif Systems**

### Step 2: Install Libraries (2 min)

Go to **Tools → Manage Libraries** and install:
- **WebSockets** by Markus Sattler
- **ArduinoJson** by Benoit Blanchon

### Step 3: Upload Master Hub (5 min)

1. Open `MasterHub/MasterHub_WiFi.ino`
2. Select: **Tools → Board → ESP32 Dev Module**
3. Select your COM port: **Tools → Port**
4. Click **Upload** (arrow button)
5. Open Serial Monitor (magnifying glass icon, 115200 baud)

**You should see:**
```
╔════════════════════════════════════════════════════════════════╗
║         FARMBOT MASTER HUB - WiFi HTTP VERSION                ║
╚════════════════════════════════════════════════════════════════╝

📡 Starting WiFi AP... ✅ OK
   SSID: FarmBot_Control
   Password: L1ncolnTech
   IP: 192.168.4.1
   🌐 Web UI: http://192.168.4.1

🟢 MASTER HUB ONLINE
```

### Step 4: Wire the Worker (10 min)

**Stepper connections:**
```
ESP32 Pin  →  ULN2003 Board 1 (Pan)
GPIO 19    →  IN1
GPIO 18    →  IN2
GPIO 5     →  IN3
GPIO 17    →  IN4

ESP32 Pin  →  ULN2003 Board 2 (Tilt)
GPIO 16    →  IN1
GPIO 4     →  IN2
GPIO 2     →  IN3
GPIO 15    →  IN4
```

**Laser connection:**
```
ESP32      →  Laser Module
GPIO 25    →  Signal/VCC
GND        →  GND
```

**Power:**
```
12V Supply →  Both ULN2003 motor power
USB 5V     →  ESP32
GND        →  Common ground (CRITICAL!)
```

### Step 5: Upload Worker (3 min)

1. Open `Workers/LaserTurret/LaserTurret_Worker.ino`
2. Select: **Tools → Board → ESP32 Dev Module**
3. Select your COM port
4. Click **Upload**
5. Open Serial Monitor (115200 baud)

**You should see:**
```
╔════════════════════════════════════════════════════════════════╗
║          ⚡ LASER TURRET WORKER - WiFi VERSION                 ║
╚════════════════════════════════════════════════════════════════╝

🔧 Initializing steppers... ✅
📡 Connecting to Master Hub WiFi... ✅
   IP Address: 192.168.4.2
📤 Registering with Master Hub...
✅ Registered with Master Hub!

🟢 LASER TURRET WORKER READY
```

---

## 🎮 Test It!

### 1. Connect to WiFi
- On your phone/computer, connect to: **FarmBot_Control**
- Password: **L1ncolnTech**

### 2. Open Web Interface
- Go to: **http://192.168.4.1**
- You should see the control dashboard

### 3. Test Movement
1. Click **MANUAL** mode button
2. Move the **X Position** slider
3. Move the **Y Position** slider
4. Watch the turret move!

### 4. Test Laser
1. Set **Duration** to 50ms
2. Set **Power** to 100%
3. Click **🎯 FIRE TARGET**
4. Laser should pulse!

---

## 📊 What You'll See

**Master Hub Serial Monitor:**
```
✅ New worker registered: LASER at 192.168.4.2
📤 Sent command to LASER: (0.75, 0.50)
```

**Worker Serial Monitor:**
```
╔════════════════════════════════════════════════════════════════╗
║  🎯 TARGET RECEIVED                                            ║
║  X: 0.750  Y: 0.500  Duration: 50ms  Power: 100%              ║
╚════════════════════════════════════════════════════════════════╝

📍 Moving to target position...
   Pan: 2560 steps, Tilt: 2048 steps
   ✅ Position reached
🔥 Firing laser...
   Laser ON for 50ms at 100% power
   ✅ Laser OFF
✅ Target #1 complete
```

**Web Interface:**
- Worker Modules panel shows: **LASER ● ONLINE**
- Statistics update in real-time
- Sliders control position smoothly

---

## ❓ Troubleshooting

### Worker Won't Connect
**Problem:** Worker shows "WiFi disconnected" or "Registration failed"

**Solution:**
1. Make sure Master Hub is powered on FIRST
2. Check Serial Monitor shows "MASTER HUB ONLINE"
3. Power cycle the worker
4. Check WiFi credentials in worker code match Master Hub

### Steppers Don't Move
**Problem:** No movement when testing

**Solution:**
1. Check 12V power is connected to ULN2003 boards
2. Verify all wiring connections
3. Check common ground between ESP32 and motor power
4. Test with simpler code (stepper test sketch)

### Laser Won't Fire
**Problem:** Laser doesn't turn on

**Solution:**
1. Check GPIO 25 connection to laser
2. Verify laser module works (connect directly to 3.3V)
3. Check power level isn't set to 0%
4. If using MOSFET, check wiring

### Can't Access Web UI
**Problem:** http://192.168.4.1 doesn't load

**Solution:**
1. Confirm you're connected to "FarmBot_Control" WiFi
2. Check Master Hub Serial shows "Web Server... ✅ OK"
3. Try different browser
4. Restart Master Hub

### Getting "Upload Error"
**Problem:** Can't upload code to ESP32

**Solution:**
1. Hold BOOT button on ESP32 during upload
2. Check correct COM port selected
3. Try lower upload speed (Tools → Upload Speed → 115200)
4. Install CH340/CP2102 USB drivers if needed

---

## 🎓 Next Steps

### Learn the System
1. Read the full [README.md](README.md)
2. Review [HARDWARE.md](HARDWARE.md) for detailed wiring
3. Try the calibration pattern (CALIBRATE mode)

### Improve Your Build
1. Design a better mounting bracket
2. Add a camera for targeting
3. Increase laser power (with proper safety!)
4. Build a second worker module

### Advanced Projects
1. Add the Galvo Laser Worker (100x faster!)
2. Integrate Jetson Nano for AI
3. Build additional worker types
4. Create autonomous field coverage

---

## 💡 Pro Tips

✅ **Always power Master Hub first**, then workers  
✅ **Keep Serial Monitors open** for debugging  
✅ **Common ground is CRITICAL** for stepper systems  
✅ **Start with low power** when testing lasers  
✅ **Document your build** with photos  
✅ **Share your improvements** via pull requests!  

---

## 🤝 Get Help

- **GitHub Issues**: Report bugs or ask questions
- **Serial Monitor**: First place to check for errors
- **Double-check wiring**: Most issues are connection problems
- **Test incrementally**: One component at a time

---

## ✅ Success Checklist

Before moving on, verify:
- [ ] Master Hub WiFi AP running (can see "FarmBot_Control")
- [ ] Master Hub web UI accessible (http://192.168.4.1)
- [ ] Worker connects and registers (shows ONLINE in web UI)
- [ ] Steppers move smoothly in both axes
- [ ] Laser fires on command
- [ ] Web UI updates in real-time
- [ ] Both Serial Monitors show clean operation

**If all checked: Congratulations! You have a working FarmBot system!** 🎉

---

**Time to completion: ~15-30 minutes**  
**Difficulty: Beginner to Intermediate**  

Ready for more? Check out the [full documentation](README.md)!
