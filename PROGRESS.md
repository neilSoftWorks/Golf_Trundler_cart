# Golf Trundler Progress (Cart Side)

## 1. Hardware Status
- [x] **Comms:** UART @ 19200 baud (GPIO 16/17). Verified reliable.
- [x] **Manual Control:** Rotary Encoder (GPIO 25/33/32) fully integrated with acceleration ramping.
- [x] **Display:** **REMOVED** (Replaced with LED Dashboard).
- [x] **Indicators:**
    - 🟢 **Green (GPIO 18):** Connection Status (Solid=Linked).
    - 🟡 **Yellow (GPIO 19):** Mode Status (Solid=Drive, Blink=Pause).
    - 🔴 **Red (GPIO 5):** Battery Warning.

## 2. Firmware Features
### A. "Smart Manual" Mode
- **Default State:** Cart boots into Manual Mode (Paused).
- **Control:** Encoder sets target speed (0-500).
- **Safety:** Must click encoder to "Activate" drive.
- **Auto-Switching:**
    - If Remote connects + Long Press: Switches to Remote.
    - If Remote disconnects: Auto-fallback to Manual (Paused).

### B. Remote Integration
- **Protocol:** ESP-NOW.
- **Telemetry:** Sends Voltage, Amps, RPM, and **Target Command** (Manual Setting) back to remote.

## 3. Current Setup
- **Board:** ESP32 DevKit V1.
- **Power:** 5V from Hoverboard.
- **Housing:** Custom 3D Printed Box (TBD).

## 4. Todo
- [ ] Mount LEDs in visible location.
- [ ] Finalize cable management.
- [ ] Field test "Hill Hold" (Active Braking logic).
