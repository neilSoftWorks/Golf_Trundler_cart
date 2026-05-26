# Golf Trundler Progress (Cart Side)

## 1. Hardware Status
- [x] **Comms:** UART @ 19200 baud (GPIO 44/43).
- [x] **Control Mode:** **TRANSPARENT MOTOR DRIVER**.
    - ESP32 handles all Tank Mixing, Inversions, and Speed Synchronization.
    - Hoverboard Master/Slave updated to accept dynamic Mode, Torque, and Inertia.
- [x] **Display:** **INTEGRATED ST7789**. 
    - Professional GFX fonts (FreeSans, BigFont).
    - Battery Icon and Footer.
    - User / Dev dual-screen toggle.
- [x] **Manual Control:** 5-Button Integrated Set (GPIO 1, 5, 10, 11, 12).
- [x] **Wireless:** **UNIFIED CONTROL** (ESP-NOW Paired Link).
    - Hardcoded MAC Pairing established.
    - Dual-control merging (Logical OR of all buttons).

## 2. Firmware Features
### A. "Smart Manual" Mode
- **Active Hill Hold:** Long-press STOP triggers **Mode 1 (PID)** at speed 0 for absolute wheel lock.
- **Dynamic Tuning:** Current limit and Inertia are now adjustable via ESP32.
- **Visual Feedback:** Bold User Screen with color-coded directional warnings.
- **Dev Terminal:** Double-click STOP to view real-time RPM, Voltage, and Sync factors.
- **Beeper:** Reverse beep disabled.
- **Auto-Shutdown:** Disabled (unit stays active for long test sessions).

### B. Safety
- **Reverse Limit:** Hard-limited to user-speed "10" (-175 internal).
- **Interlock:** Speed cannot be changed while Parked or Braked.
- **Remote Timeout:** Remote commands ignored if link lost for >1s.

