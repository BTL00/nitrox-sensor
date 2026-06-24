# Nitrox Sensor (TTGO T-Display)

Draft firmware scaffold for a Nitrox analyzer on ESP32 TTGO T-Display.

## Included in this scaffold

- Hardware-abstraction boundary for UART access (`IUart`)
- Real UART adapter (`HardwareUartAdapter`) and test/mock UART (`MockUart`)
- US1010 sensor service shell (`Us1010Sensor`) with mock-friendly line parser
- Draft TFT dashboard view (O2, pressure, temperature, battery, status)
- Battery monitor module (ADC-based, configurable divider and min/max volts)
- Button manager with basic debounce and callbacks

## What you need to implement

- Replace parser logic in `Us1010Sensor` with actual US1010 protocol handling
- Confirm/calibrate battery divider values and ADC attenuation for your hardware
- Tune TTGO pin mapping if your board revision differs

## Build and flash

```bash
pio run
pio run -t upload
pio device monitor
```

## Notes

- Mock sensor mode is enabled by default via `-DUSE_MOCK_SENSOR=1` in `platformio.ini`.
- Disable mock mode for hardware UART testing by changing it to `0`.
