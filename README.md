# Golf Cart Receiver (The Brain)

This firmware runs on the **ESP32-S3 SuperMini attached to the Hoverboard**. It acts as the "Software Defined" brain of the vehicle, managing motor logic, safety, and a professional diagnostic display.

## Key Features
*   **Transparent Driver Model:** The Hoverboard acts as a dumb driver; all mixing and tuning logic lives on the ESP32.
*   **Dynamic Tuning:** Adjust Torque (Amps) and Smoothing (Inertia) in real-time without re-flashing.
*   **Active Hill Hold:** Long-press STOP triggers an active PID-lock for rock-solid parking on slopes.
*   **Dual Mode:**
    1.  **Manual Mode:** Control speed/steer via integrated 5-button set.
    2.  **Remote Mode:** Broadcast mirror control from the handheld remote (ESP-NOW).
*   **Integrated UI:** 240x320 ST7789 display with high-quality GFX fonts and a professional battery footer.
*   **Safety First:**
    *   **Reverse Limit:** Hard-limited to speed '10' with visual orange warning.
    *   **Button Lock:** Speed setpoint frozen while Parked or Braked.
    *   **Watchdog:** Auto-stops if communication or remote link is lost.

## Wiring (See `gemini.md` for full details)
*   **UART:** GPIO 44 (RX) / 43 (TX) -> Hoverboard.
*   **Buttons:** FWD (12), REV (5), STOP (1), LEFT (10), RIGHT (11).
*   **Display (SPI):** SCK (6), MOSI (4), CS (7), DC (2), RST (3).

## How to Flash
See the comprehensive [FLASHING_GUIDE.md](../FLASHING_GUIDE.md) in the root directory for step-by-step instructions.
