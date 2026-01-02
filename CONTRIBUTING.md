# 🤝 Contributing to FarmBot

Thank you for your interest in contributing to the FarmBot Modular Agricultural Robotics System!

This is an **educational open-source project** built for hands-on learning and innovation in agricultural automation.

---

## 🎯 Ways to Contribute

### For Students:
- 📦 **Build new worker modules** (spray, motion, sensors)
- 📸 **Document your build** with photos and videos
- 🐛 **Report bugs** you encounter during assembly
- 💡 **Suggest improvements** to code or documentation
- 🎓 **Share your learning** - write tutorials or guides

### For Educators:
- 📚 **Create lab exercises** for classroom use
- ✏️ **Develop assessment materials** and rubrics
- 🏫 **Share implementation strategies** for different skill levels
- 📖 **Contribute to curriculum guide**
- 🎬 **Create instructional videos**

### For Developers:
- 💻 **Add new features** to Master Hub or Workers
- 🔧 **Improve existing code** - optimization, bug fixes
- 🌐 **Enhance web UI** - new visualizations, controls
- 📱 **Develop mobile app** for system control
- 🧪 **Write tests** - unit tests, integration tests

### For Hardware Enthusiasts:
- 🔌 **Design PCBs** for cleaner worker modules
- 🏗️ **Create 3D models** for enclosures and mounts
- ⚙️ **Develop mechanical systems** for new workers
- 📐 **Improve wiring diagrams** and schematics
- 🖼️ **Take quality photos** of builds

---

## 🚀 Getting Started

### 1. Set Up Development Environment

**Clone the repository:**
```bash
git clone https://github.com/yourusername/farmbot-modular-robotics.git
cd farmbot-modular-robotics
```

**Install dependencies:**
- Arduino IDE 2.x
- ESP32 board support
- Required libraries (see README.md)

**Build the system:**
- Follow [QUICKSTART.md](QUICKSTART.md) to get a working setup
- Test with actual hardware before making changes

### 2. Find an Issue

Browse [open issues](https://github.com/yourusername/farmbot-modular-robotics/issues) tagged:
- `good first issue` - Great for newcomers
- `help wanted` - Community input needed
- `student project` - Ideal for class assignments
- `enhancement` - New features
- `bug` - Something's broken

### 3. Create a Branch

```bash
git checkout -b feature/your-feature-name
```

Branch naming conventions:
- `feature/` - New features
- `bugfix/` - Bug fixes
- `docs/` - Documentation updates
- `worker/` - New worker modules
- `hardware/` - Hardware designs

### 4. Make Your Changes

**Code guidelines:**
- Follow existing code style and formatting
- Add comprehensive comments explaining your logic
- Use meaningful variable names
- Keep functions focused and simple

**Documentation guidelines:**
- Update README.md if adding features
- Add comments to complex sections
- Include wiring diagrams for hardware changes
- Write clear commit messages

### 5. Test Your Changes

**Required testing:**
- [ ] Code compiles without errors or warnings
- [ ] Upload to actual ESP32 hardware
- [ ] Verify functionality with live system
- [ ] Check Serial Monitor output for errors
- [ ] Test edge cases and error conditions

**Recommended testing:**
- [ ] Test with multiple worker modules
- [ ] Verify web UI updates correctly
- [ ] Check WiFi reconnection after power cycle
- [ ] Ensure backward compatibility

### 6. Submit Pull Request

**Before submitting:**
- [ ] All tests pass
- [ ] Documentation updated
- [ ] Code commented
- [ ] Commit messages clear
- [ ] No debugging code left in

**PR Description should include:**
- What problem does this solve?
- How did you test it?
- Any breaking changes?
- Screenshots/videos (if UI changes)
- Hardware requirements (if applicable)

---

## 📝 Code Style Guidelines

### General Principles
- **Readability first** - Code is read more than written
- **Consistent formatting** - Match existing style
- **Meaningful names** - No single-letter variables (except loop counters)
- **Comment your intent** - Explain WHY, not just WHAT

### C/C++ Style (Arduino)

**Naming:**
```cpp
// Constants: ALL_CAPS with underscores
#define LASER_PIN 25
const int MAX_WORKERS = 10;

// Variables: camelCase
int workerCount = 0;
float batteryVoltage = 12.5;

// Functions: camelCase
void sendCommand() { }
bool checkSafety() { }

// Classes/Structs: PascalCase
struct WorkerInfo { };
class LaserController { };
```

**Formatting:**
```cpp
// Braces on same line for functions
void setup() {
  Serial.begin(115200);
}

// Spaces around operators
int result = value1 + value2;

// Indent with 2 spaces (not tabs)
if(condition) {
  doSomething();
  doMore();
}

// Comments above code, not inline
// This initializes the WiFi connection
WiFi.begin(SSID, PASSWORD);
```

**Function Structure:**
```cpp
// Function header comment
/*
 * Sends target command to specified worker
 * 
 * @param workerIdx: Index in workers array
 * @param cmd: Command structure with target data
 * @return: true if sent successfully
 */
bool sendCommandToWorker(int workerIdx, Command& cmd) {
  // Validate input
  if(workerIdx < 0 || workerIdx >= workerCount) {
    Serial.println("Invalid worker index");
    return false;
  }
  
  // Implementation here
  // ...
  
  return true;
}
```

### HTML/CSS/JavaScript Style (Web UI)

**Keep it simple:**
- Use vanilla JavaScript (no frameworks needed)
- Inline CSS is acceptable for this project
- Comment complex CSS selectors
- Use meaningful element IDs

**Example:**
```javascript
// Update worker status display
function updateWorkers() {
  fetch('/api/workers')
    .then(r => r.json())
    .then(workers => {
      // Update UI with worker data
      workers.forEach(w => {
        updateWorkerElement(w);
      });
    });
}
```

---

## 🐛 Bug Reports

### Before Reporting
1. **Search existing issues** - Someone may have already reported it
2. **Test with latest code** - Pull from main branch
3. **Try different hardware** - Rule out component failure
4. **Check Serial Monitor** - Look for error messages

### Good Bug Report Includes

**Title:** Clear, specific description
- ❌ "It doesn't work"
- ✅ "Worker fails to register with Master Hub after WiFi reconnect"

**Description:**
```markdown
**Expected Behavior:**
Worker should automatically reconnect and re-register after WiFi is restored.

**Actual Behavior:**
Worker reconnects to WiFi but does not send registration POST request.

**Steps to Reproduce:**
1. Power on Master Hub and Worker
2. Verify worker registers successfully
3. Power cycle Master Hub
4. Wait for Master Hub WiFi to restart
5. Observe worker Serial Monitor

**Serial Output:**
[Paste relevant serial output here]

**Hardware:**
- ESP32: DOIT DevKit V1
- Arduino IDE: 2.3.0
- ESP32 Core: 2.0.14
- Master Hub Code: v1.0
- Worker Code: v1.0

**Additional Context:**
Only happens when Master Hub is power cycled, not when worker is power cycled.
```

---

## 🆕 Feature Requests

### Good Feature Requests

**Clear problem statement:**
- What problem are you trying to solve?
- Who would benefit from this feature?
- Why is this important?

**Proposed solution:**
- How should it work?
- What are alternative approaches?
- What are the tradeoffs?

**Example:**
```markdown
**Problem:**
Currently, there's no way to save targeting positions for repeated use. 
Students waste time re-entering coordinates during calibration.

**Proposed Solution:**
Add a "Save Position" button in Manual mode that stores current X/Y/Duration/Power 
to a preset slot. Add "Load Position" dropdown to quickly recall saved positions.

**Alternatives:**
- Could use browser localStorage
- Could store on Master Hub SD card
- Could send presets as JSON file

**Benefits:**
- Faster calibration workflows
- Consistent testing across sessions
- Better for classroom demonstrations
```

---

## 📚 Documentation Contributions

### What Needs Documentation

**Always welcome:**
- Assembly guides with photos
- Troubleshooting tips
- Calibration procedures
- Lab exercises for students
- Video tutorials
- Translation to other languages

### Documentation Style

**Use clear, simple language:**
- Short sentences
- Active voice
- Step-by-step when appropriate
- Include visual aids (photos, diagrams)

**Example - Good:**
```markdown
## Wiring the Stepper Motor

1. Connect GPIO 19 to IN1 on the ULN2003 board
2. Connect GPIO 18 to IN2
3. Connect GPIO 5 to IN3
4. Connect GPIO 17 to IN4

[Photo of completed wiring]

**Important:** All ground connections must be common!
```

**Example - Bad:**
```markdown
## Stepper Configuration

The stepper motor utilizes a ULN2003 Darlington transistor array 
interfacing with the ESP32's GPIO pins to facilitate precise 
rotational control through sequential energization of the motor windings.
```

---

## 🎓 Educational Projects

### Student Project Ideas

**Beginner (1-2 weeks):**
- Build and test basic stepper worker
- Create custom calibration pattern
- Design 3D printed mounting bracket
- Write lab report on system architecture

**Intermediate (3-4 weeks):**
- Implement new worker type (spray, sensor)
- Add data logging to SD card
- Create mobile-friendly web UI
- Develop automated testing suite

**Advanced (5-8 weeks):**
- Integrate computer vision with camera
- Multi-robot coordination system
- Machine learning for target classification
- Complete field deployment system

### Classroom Use

**As an educator, please share:**
- Lab exercise worksheets
- Grading rubrics
- Student assessment results
- Implementation challenges
- Success stories
- Curriculum integration strategies

---

## 🏆 Recognition

### Contributors

All contributors will be:
- Listed in CONTRIBUTORS.md
- Credited in release notes
- Acknowledged in documentation
- Eligible for student showcase features

### Student Showcase

Outstanding student projects may be featured:
- In README.md examples section
- On project website (if created)
- At industry events
- In educational publications

---

## ⚖️ Code of Conduct

### Our Standards

**We expect all contributors to:**
- Be respectful and inclusive
- Provide constructive feedback
- Accept criticism gracefully
- Focus on what's best for education
- Help newcomers learn

**Unacceptable behavior:**
- Harassment or discrimination
- Trolling or insulting comments
- Personal or political attacks
- Sharing private information
- Other unprofessional conduct

### Enforcement

Violations may result in:
- Warning
- Temporary ban
- Permanent ban

Report issues to: [Project maintainer email]

---

## 📧 Questions?

**Not sure where to start?**
- Check [GitHub Discussions](https://github.com/yourusername/farmbot-modular-robotics/discussions)
- Open an issue tagged `question`
- Review existing PRs for examples

**Want to contribute but don't code?**
- Document your build
- Take photos/videos
- Help with testing
- Answer questions from other users
- Spread the word!

---

## 🙏 Thank You!

Every contribution, no matter how small, helps make this project better for students and educators worldwide.

**Together, we're building the future of agricultural automation education!** 🚜🌱

---

*This project is maintained by Kanenas at Lincoln Technical Institute, Queens, NY*

*Last Updated: January 2025*
