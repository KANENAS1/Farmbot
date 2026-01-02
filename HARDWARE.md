# 🔧 Hardware Documentation

Detailed hardware specifications and assembly instructions for FarmBot Modular Agricultural Robotics System.

---

## 📦 Bill of Materials (BOM)

### Master Hub Components

| Component | Quantity | Specs | Est. Cost | Notes |
|-----------|----------|-------|-----------|-------|
| ESP32 DevKit | 1 | Any variant with WiFi | $8-12 | DOIT DevKit V1 recommended |
| microSD Card Module | 1 | SPI interface | $2-5 | Optional, for logging |
| Status LED | 1 | 5mm Green | $0.10 | - |
| Buzzer | 1 | Active 5V | $1-2 | - |
| Resistors | 2 | 220Ω, 1/4W | $0.10 | For LEDs |
| Breadboard/PCB | 1 | - | $5-15 | For prototyping |
| Power Supply | 1 | 5V, 2A USB | $5-10 | - |
| **Total** | - | - | **~$25-50** | - |

### Laser Turret Worker (Stepper Version)

| Component | Quantity | Specs | Est. Cost | Notes |
|-----------|----------|-------|-----------|-------|
| ESP32 DevKit | 1 | Any variant | $8-12 | - |
| 28BYJ-48 Stepper Motor | 2 | 5V/12V | $5 each | Pan & Tilt |
| ULN2003 Driver Board | 2 | Comes with motors | Included | - |
| Laser Pointer Module | 1 | 3.3V/5V, <5mW | $5-15 | Class 1 or 2 |
| MOSFET | 1 | IRLZ44N or similar | $1 | Optional, for laser |
| Status LED | 1 | 5mm Green | $0.10 | - |
| Pan/Tilt Bracket | 1 | 3D printed or metal | $10-20 | Custom design |
| Mounting Hardware | Set | M3 screws, nuts | $5 | - |
| Power Supply | 1 | 12V, 1A | $8-12 | For steppers |
| **Total** | - | - | **~$60-90** | - |

### Galvo Laser Worker (High-Speed Version)

| Component | Quantity | Specs | Est. Cost | Notes |
|-----------|----------|-------|-----------|-------|
| ESP32 DevKit | 1 | **Must have DAC pins** | $8-12 | GPIO 25/26 required |
| Galvo Mirror System | 1 | 10-20mm mirrors | $200-1200 | Depends on quality |
| Galvo Driver Boards | 2 | Analog 0-5V input | Usually included | X and Y axis |
| High-Power Laser | 1 | 5W-40W | $100-500 | 445nm blue typical |
| Laser Driver | 1 | PWM/TTL control | $30-80 | Constant current |
| E-Stop Button | 1 | Mushroom head, NC | $10-15 | Emergency stop |
| Interlock Switch | 1 | Magnetic or key | $5-10 | Door safety |
| Temperature Sensor | 1 | TMP36 or DHT22 | $2-10 | Laser cooling |
| Status LEDs | 3 | Green (2), Red (1) | $0.30 | System/Ready/Laser |
| Safety Goggles | - | OD4+ rated | $50-150 | **MANDATORY** |
| Enclosure | 1 | Laser-safe material | $50-200 | Aluminum recommended |
| Power Supply | 1 | 12V/24V, depends | $30-100 | Match laser specs |
| **Total** | - | - | **~$500-2500** | Professional system |

---

## 🔌 Wiring Diagrams

### Master Hub Wiring

```
ESP32 Master Hub
┌─────────────────────────────────┐
│                                 │
│  GPIO 2  ────────┐              │
│                  │              │
│                  ├─── 220Ω ─── LED ─── GND
│                  │   (Green)          (Status)
│                                 │
│  GPIO 4  ─────── Buzzer+ ────── │
│                                 │
│  GND ─────────── Buzzer- ────── │
│                                 │
│  GPIO 5  ─────── SD_CS          │
│  GPIO 23 ─────── SD_MOSI    (Optional)
│  GPIO 19 ─────── SD_MISO    SD Logging
│  GPIO 18 ─────── SD_CLK         │
│                                 │
│  5V ──────────── VCC ────────── │
│  GND ─────────── GND ────────── │
│                                 │
└─────────────────────────────────┘

Power: 5V USB or regulated supply
```

### Laser Turret Worker (Stepper) Wiring

```
ESP32 Worker + Steppers
┌────────────────────────────────────────────────────────┐
│                                                        │
│  PAN STEPPER (X-AXIS)                                 │
│  GPIO 19 ─────── IN1 ┐                                │
│  GPIO 18 ─────── IN2 ├─── ULN2003 ─── 28BYJ-48       │
│  GPIO 5  ─────── IN3 │    Driver      Stepper        │
│  GPIO 17 ─────── IN4 ┘                                │
│                                                        │
│  TILT STEPPER (Y-AXIS)                                │
│  GPIO 16 ─────── IN1 ┐                                │
│  GPIO 4  ─────── IN2 ├─── ULN2003 ─── 28BYJ-48       │
│  GPIO 2  ─────── IN3 │    Driver      Stepper        │
│  GPIO 15 ─────── IN4 ┘                                │
│                                                        │
│  LASER CONTROL                                        │
│  GPIO 25 ─────── Gate ┐                               │
│                       ├─── MOSFET ─── Laser+ ──┐     │
│  GND ─────────────────┘                         │     │
│                                                 │     │
│                    Laser- ─────────────────────┘     │
│                                                        │
│  STATUS LED                                           │
│  GPIO 26 ─────── 220Ω ─── LED ─── GND                │
│                        (Green)                        │
│                                                        │
│  POWER                                                │
│  12V DC ──────── ULN2003 Vcc (both drivers)          │
│  5V USB ───────── ESP32 VIN                          │
│  GND ─────────── Common Ground (all components)      │
│                                                        │
└────────────────────────────────────────────────────────┘

CRITICAL: Common ground for all components!
```

### Galvo Laser Worker (High-Speed) Wiring

```
ESP32 + Galvo System
┌──────────────────────────────────────────────────────────┐
│                                                          │
│  GALVO CONTROL (ANALOG - FIXED PINS!)                   │
│  GPIO 25 (DAC1) ──── X-Axis Galvo Driver (0-3.3V)      │
│  GPIO 26 (DAC2) ──── Y-Axis Galvo Driver (0-3.3V)      │
│  GPIO 14 ───────────  Galvo Enable (HIGH = active)      │
│                                                          │
│  LASER CONTROL                                          │
│  GPIO 32 ───────────  Laser Driver PWM Input            │
│  GPIO 33 ───────────  Laser Enable (HIGH = armed)       │
│  GPIO 27 ───────────  Laser TTL Trigger                 │
│                                                          │
│  SAFETY SYSTEM                                          │
│  GPIO 35 ◄────┬────  E-Stop Button (NO contact)        │
│               └────  3.3V (with 10kΩ pullup)            │
│                                                          │
│  GPIO 34 ◄────────   Interlock Switch (HIGH = safe)     │
│                                                          │
│  GPIO 36 (ADC) ◄───  TMP36 Temperature Sensor           │
│                      (Vout → 10mV/°C)                    │
│                                                          │
│  STATUS INDICATORS                                      │
│  GPIO 2  ──── 220Ω ── Green LED ── GND (Status)        │
│  GPIO 15 ──── 220Ω ── RED LED ─── GND (Laser Active)   │
│  GPIO 4  ──── 220Ω ── Green LED ── GND (Ready)         │
│                                                          │
│  POWER DISTRIBUTION                                     │
│  12V/24V ─────────── Galvo Drivers + Laser Supply      │
│  5V USB ──────────── ESP32 VIN                         │
│  GND ────────────────  Common Ground (CRITICAL!)        │
│                                                          │
└──────────────────────────────────────────────────────────┘

⚠️ WARNING: HIGH-POWER LASER SYSTEM
   - Use only in enclosed environment
   - Wear proper laser safety goggles
   - Multiple safety interlocks required
   - Follow local laser safety regulations
```

---

## 🔨 Assembly Instructions

### Master Hub Assembly

**Step 1: Prepare Components**
1. ESP32 DevKit board
2. Breadboard or custom PCB
3. Status LED + 220Ω resistor
4. Active buzzer (5V)
5. Jumper wires

**Step 2: Wire Status LED**
```
GPIO 2 → 220Ω resistor → LED anode (+)
LED cathode (-) → GND
```

**Step 3: Wire Buzzer**
```
GPIO 4 → Buzzer positive (+)
Buzzer negative (-) → GND
```

**Step 4: Optional SD Card Module**
```
GPIO 5  → CS
GPIO 23 → MOSI
GPIO 19 → MISO
GPIO 18 → SCK
3.3V → VCC
GND → GND
```

**Step 5: Power Connection**
- Connect via USB cable
- OR regulated 5V to VIN pin

**Step 6: Upload Firmware**
- Open `MasterHub_WiFi.ino` in Arduino IDE
- Select ESP32 Dev Module
- Upload code
- Open Serial Monitor (115200 baud)
- Verify WiFi AP starts successfully

---

### Laser Turret Worker Assembly

**Step 1: Mount Stepper Motors**
1. Create or purchase pan/tilt bracket
2. Mount first 28BYJ-48 motor (Pan axis - horizontal rotation)
3. Mount second 28BYJ-48 motor (Tilt axis - vertical movement)
4. Ensure smooth rotation with no binding

**Step 2: Wire Pan Stepper**
```
ESP32        ULN2003 Driver 1
GPIO 19  →   IN1
GPIO 18  →   IN2
GPIO 5   →   IN3
GPIO 17  →   IN4
12V      →   Motor Power (+)
GND      →   Motor Power (-)
```

**Step 3: Wire Tilt Stepper**
```
ESP32        ULN2003 Driver 2
GPIO 16  →   IN1
GPIO 4   →   IN2
GPIO 2   →   IN3
GPIO 15  →   IN4
12V      →   Motor Power (+)
GND      →   Motor Power (-)
```

**Step 4: Mount Laser**
1. Attach laser pointer to tilt bracket
2. Wire laser control:
```
GPIO 25 → MOSFET Gate
MOSFET Drain → Laser (-)
Laser (+) → 3.3V/5V (depending on laser)
MOSFET Source → GND
```

**Step 5: Add Status LED**
```
GPIO 26 → 220Ω resistor → LED (+)
LED (-) → GND
```

**Step 6: Power Distribution**
- 12V supply → Both ULN2003 boards
- 5V USB → ESP32
- **CRITICAL**: Connect all grounds together

**Step 7: Upload & Test**
- Upload `Worker_WiFi_Stepper_Laser.ino`
- Power on Master Hub first
- Power on Worker
- Watch Serial Monitor for successful registration
- Test movement with Web UI

---

### Galvo Laser Worker Assembly

**⚠️ WARNING: This is a HIGH-POWER laser system. Only build if you have:**
- Proper laser safety training
- Appropriate laser safety goggles
- Enclosed, laser-safe environment
- Understanding of local laser safety regulations

**Step 1: Safety First**
1. Purchase OD4+ laser safety goggles for your wavelength
2. Design enclosed workspace with interlock switches
3. Install E-stop button within easy reach
4. Post laser warning signs
5. Never operate without safety systems

**Step 2: Mount Galvo Mirrors**
1. Install X-axis galvo mirror (horizontal deflection)
2. Install Y-axis galvo mirror (vertical deflection)
3. Align optical path
4. Ensure mirrors are at 90° to each other
5. Secure mounting - vibration will affect accuracy

**Step 3: Wire Galvo Drivers**
```
ESP32 (FIXED PINS - DAC ONLY!)    Galvo Drivers
GPIO 25 (DAC1)  →  X-Axis Analog Input (0-3.3V)
GPIO 26 (DAC2)  →  Y-Axis Analog Input (0-3.3V)
GPIO 14         →  Enable signal (both drivers)
12V/24V         →  Driver power (check specs)
GND             →  Common ground
```

**Step 4: Wire Laser System**
```
High-Power Laser Module + Driver:
  GPIO 32 → Laser Driver PWM Input (power control)
  GPIO 33 → Laser Enable Input (arming)
  GPIO 27 → Laser TTL Trigger (firing pulse)
  
Laser Driver:
  12V/24V → Power Input (check laser specs)
  Output  → Laser Module Power
  GND     → Common ground
```

**Step 5: Safety Interlock System**
```
E-Stop Circuit:
  3.3V → 10kΩ pullup → GPIO 35 → E-Stop (NC) → GND
  (When button pressed, GPIO 35 goes LOW = STOP)

Door Interlock:
  Magnetic switch or key switch
  Output → GPIO 34 (HIGH when door closed/safe)
  
Temperature Sensor:
  TMP36 Vout → GPIO 36 (ADC)
  TMP36 Vcc  → 3.3V
  TMP36 GND  → GND
```

**Step 6: Status Indicators**
```
System Status:   GPIO 2  → 220Ω → Green LED → GND
Laser Active:    GPIO 15 → 220Ω → RED LED   → GND
Ready:           GPIO 4  → 220Ω → Green LED → GND
```

**Step 7: Enclosure & Mounting**
1. Build laser-safe enclosure (black anodized aluminum ideal)
2. Mount all components inside
3. Install viewing window (if needed, use laser-safe acrylic)
4. Route all cables neatly
5. Ensure proper ventilation for laser cooling

**Step 8: Initial Testing (WITHOUT LASER)**
1. Upload `Worker_WiFi_Galvo_Laser.ino`
2. Test galvo mirror movement with Web UI
3. Verify safety interlocks (E-stop, door switch)
4. Check temperature monitoring
5. Confirm status LEDs working

**Step 9: Laser Integration**
1. Install laser module
2. Connect laser driver
3. Test enable/disable circuits
4. Test with VERY short pulses (10ms)
5. Gradually increase duration while monitoring
6. Run full calibration pattern
7. Mark laser impact points on target

**Step 10: Calibration**
See CALIBRATION.md for detailed procedures

---

## 🔋 Power Requirements

### Master Hub
- **Voltage**: 5V DC
- **Current**: 500mA typical, 1A peak
- **Power**: 2.5W - 5W
- **Source**: USB or regulated supply

### Laser Turret Worker
- **Stepper Motors**: 12V DC, 500mA total
- **ESP32**: 5V DC, 500mA
- **Laser**: 3.3V/5V, <100mA (low-power pointer)
- **Total**: 12V @ 600mA + 5V @ 600mA
- **Recommended**: 12V 1A supply + USB 5V 1A

### Galvo Laser Worker
- **Galvo Drivers**: 12V-24V DC, 1-2A per driver
- **High-Power Laser**: 12V-48V DC, 2-10A (depends on wattage)
- **ESP32**: 5V DC, 500mA
- **Total**: Highly variable based on laser power
- **Example 20W Laser System**:
  - 24V @ 5A for laser + drivers
  - Total: ~120W
- **Recommended**: Dedicated power supply sized for laser

### Power Supply Selection Guide

**For Prototyping:**
- Master Hub: Any USB power bank
- Stepper Worker: 12V wall adapter (1A) + USB
- Galvo Worker: Bench power supply

**For Production:**
- Use regulated switching power supplies
- Add fuses on all power rails
- Include reverse polarity protection
- Consider battery backup for Master Hub

---

## 📐 Mechanical Design

### 3D Printable Components

**Pan/Tilt Bracket for Stepper Version:**
- STL files in `/hardware/3d_models/`
- Print settings:
  - Layer height: 0.2mm
  - Infill: 30%
  - Material: PLA or PETG
  - Supports: Required
  
**Galvo Mirror Mount:**
- Vibration-dampening required
- Metal preferred over 3D printed
- Commercial mounts recommended

**Enclosure Designs:**
- Laser-safe materials only
- Ventilation for cooling
- Cable management
- Easy access for maintenance

### Recommended Tools

**Essential:**
- Soldering iron (temperature controlled)
- Wire strippers
- Multimeter
- Screwdriver set
- Needle-nose pliers

**Advanced:**
- Oscilloscope (for galvo tuning)
- Power supply (variable bench type)
- Laser power meter
- 3D printer

---

## 🔍 Testing & Troubleshooting

### Initial Power-On Tests

**Master Hub:**
1. Connect power
2. Status LED should blink
3. Buzzer should beep 3 times
4. Serial monitor shows WiFi AP started
5. Can connect to "FarmBot_Control" network

**Laser Turret Worker:**
1. Connect power to steppers (12V)
2. Connect power to ESP32 (5V)
3. Status LED blinks during startup
4. Steppers should center on startup
5. Serial monitor shows WiFi connection

**Galvo Laser Worker:**
1. All safety systems checked FIRST
2. Status LEDs should show system state
3. Galvos should move to center
4. Safety checks run automatically
5. NO laser power until fully tested

### Common Issues

**Worker won't connect to Master:**
- Check Master Hub WiFi is running
- Verify WiFi credentials in worker code
- Power cycle both devices
- Check Serial Monitor for error messages

**Stepper motors jerky:**
- Check power supply current rating
- Verify wiring connections
- Reduce step delay in code
- Check common ground connection

**Galvo mirrors unstable:**
- Check DAC output with oscilloscope
- Verify analog reference voltage
- Reduce settling time in calibration
- Ensure vibration-free mounting

**Laser won't fire:**
- Verify all safety interlocks closed
- Check E-stop button not pressed
- Temperature below limit
- MOSFET/driver wiring correct
- Laser module receiving power

---

## 📚 Additional Resources

**Datasheets:**
- ESP32 Technical Reference: [Espressif Docs]
- 28BYJ-48 Stepper: See `/docs/datasheets/`
- ULN2003 Driver: See `/docs/datasheets/`
- TMP36 Temperature Sensor: See `/docs/datasheets/`

**Tutorials:**
- ESP32 Getting Started
- Stepper Motor Control
- Laser Safety Guidelines
- PCB Design Basics

**Suppliers:**
- **Adafruit**: Quality components, good documentation
- **SparkFun**: Educational focus
- **Amazon**: Bulk Chinese components
- **Digi-Key/Mouser**: Professional components
- **AliExpress**: Budget options

---

**Last Updated**: January 2025  
**Revision**: 1.0
