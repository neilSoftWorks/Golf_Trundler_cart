# Golf Cart Receiver (The Brain)

This firmware runs on the **ESP32 attached to the Hoverboard**. It acts as the bridge between the Hoverboard motor controller and the user (via Remote or Manual Knob).

## Key Features
*   **Dual Mode:**
    1.  **Manual Mode:** Walk behind the cart, controlling speed with a rotary knob.
    2.  **Remote Mode:** Control from a distance using the handheld remote.
*   **Headless UI:** Uses 3 LEDs for robust, sunlight-visible status.
*   **Safety First:**
    *   Boots into "Paused" mode.
    *   Auto-brakes if remote link is lost.
    *   Requires deliberate action to switch modes.

## Wiring (See `wiring.txt` for full details)
*   **UART:** GPIO 16 (RX) / 17 (TX) -> Hoverboard.
*   **Encoder:** GPIO 25/33/32.
*   **LEDs:** Green (18), Yellow (19), Red (5).

## How to Flash
1.  Install **PlatformIO**.
2.  Open this folder.
3.  Run `pio run -t upload`.