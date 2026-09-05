[README.md](https://github.com/user-attachments/files/31870708/README.md)
# Firefighter Robot — STM32 Embedded Firmware

Embedded firmware for the firefighter robot developed at Kadanova (industrial robotics, IoT, and AI startup). This repository contains the STM32F407VGT6 firmware layer, responsible for real-time sensor acquisition, safety logic, and communication with the ESP32 relay and Raspberry Pi 4 main processing unit.

> **⚠️ Status: First draft — not yet tested on real hardware.**
> This code has been written and verified to *compile and link cleanly*, but has **not** been tested against the actual sensors, actuators, or a physical robot. Register maps, thresholds, and calibration values are drafted from datasheets and general conventions where the real hardware wasn't available to verify against. See [Known Gaps](#known-gaps--todos) before trusting any of this for a live demo or safety-critical decision.

---

## Hardware Architecture

| Component | Role |
|---|---|
| **Raspberry Pi 4 (4GB)** | Main processing unit — computer vision, high-level decision making |
| **STM32F407VGT6** | Real-time safety MCU — sensor acquisition, local fire state machine, motion validation |
| **ESP32** | UART relay between STM32 and Pi (transport only, no decision logic) |
| **RPLidar C1** | Housed in a sealed quartz-tube enclosure |

Reference manual: **RM0090** (STM32F405/407/415/417 family).

Key constraints driving design choices: strict mass budget, thermal dissipation inside a sealed PCM enclosure (drove Pi 4 over Pi 5), and a requirement that individual sensor failures must not compromise the rest of the system (graceful degradation).

---

## Firmware Architecture

```
App/
├── Comm/       Phase 1 — raw peripheral I/O (UART, I2C, SPI, ADC)
├── Sensors/    Phase 3 — per-device sensor drivers
├── Logic/      Fusion + safety decisions (fire state machine, motion validator)
└── Protocol/   UART packet framing for STM32 ↔ ESP32 communication
```

**Design principles** (set at project start, maintained throughout):
- **Modularity** — each driver is independent; sensor-exclusive peripherals (e.g. encoder timers) are owned by their own driver file, not centralized.
- **Minimal dependencies** — drivers depend only on CubeMX HAL headers and their own peripheral, not on each other.
- **Graceful degradation** — every sensor exposes an `IsAvailable()`-style check; a missing/failed sensor must not crash or block the rest of the system.
- **Learning-first** — the UART driver (interrupt + DMA) was written by hand, without AI-generated code, per project lead's requirement. Later files were built with AI assistance for speed, once that constraint was lifted.

---

## Module Status

### `Comm/` — Raw Peripheral Drivers

| File | Status | Notes |
|---|---|---|
| `uart_driver.c/.h` | ✅ Done | USART2, 115200 baud. Both interrupt-driven and DMA-mode implementations built; DMA is primary. Uses 1-byte-at-a-time DMA re-arming — functionally correct, but doesn't yet use DMA's full CPU-offload benefit (circular buffer would be the next iteration). |
| `i2c_driver.c/.h` | ✅ Done | I2C1 (PB6/PB7), 100 kHz standard mode, blocking calls with timeout. Register-address read/write pattern (`Mem_Read`/`Mem_Write`) for standard register-mapped sensors. |
| `adc_driver.c/.h` | ✅ Done | ADC1, channels IN0/IN1 configured. Returns status + output pointer (not a bare value) so a failed read can't be confused with a real 0 reading. |
| `spi_driver.c/.h` | ⬜ Empty | No SPI sensor identified yet — placeholder for future use. |
| `timer_encoder_driver.c/.h` | ⬜ Empty / superseded | Encoder timer setup was deliberately moved into `Sensors/encoders.c` instead, per the "sensor-exclusive peripherals owned by their driver" principle. This file is unused as a result. |

### `Sensors/` — Per-Device Drivers

| File | Status | Notes |
|---|---|---|
| `encoders.c/.h` | ✅ Done | 4 wheel encoders via TIM1–TIM4 hardware encoder mode. Exposes raw count and delta-since-last-read; TIM1/TIM3/TIM4 are 16-bit counters (wrap sooner), TIM2 would be 32-bit if reassigned — worth accounting for in `motion_validator.c` if sustained fast rotation causes wraparound issues. |
| `imu_lsm6ds3.c/.h` | ✅ Done | I2C, WHO_AM_I-verified. ±2g accel / ±245dps gyro, 104Hz ODR. Address assumes SA0 pin tied low (`0x6A`) — confirm against actual board wiring. |
| `ina226.c/.h` | ✅ Done | I2C, manufacturer-ID-verified. **Calibration constants (`SHUNT_RESISTANCE_OHMS`, `CURRENT_LSB_A`) are placeholders** and depend on the actual shunt resistor value on the board — must be corrected once known, or current/power readings will be silently wrong. |
| `adt7482.c/.h` | ⚠️ Unverified | Register map, resolution, and manufacturer/device ID values were drafted from general ADI thermal-sensor conventions, **not verified against the actual ADT7482 datasheet**. Confirm before trusting readings — this feeds fire-detection logic. |
| `mlx90614.c/.h` | ⚠️ Partial | Reads ambient + object temperature correctly per datasheet formula. **PEC (SMBus CRC-8) byte is read but not validated** — corrupted readings could currently pass through undetected. No WHO_AM_I-style identity register exists on this sensor; presence is inferred via a plausible-range sanity check only. |
| `vl53l0x.c/.h` | ⚠️ Minimal, uncalibrated | Hand-written basic single-shot ranging driver — **not** ST's official calibrated API (STSW-IMG005). No SPAD/temperature/voltage reference calibration performed. Register addresses are drafted from widely-referenced public documentation, not independently verified against ST's primary datasheet. Distance readings may be inaccurate or drift; integrating ST's official API is the recommended next step before trusting this for real proximity/obstacle decisions. |
| `mq2.c/.h` | ⚠️ Placeholder | ADC read pipeline works. **PPM conversion is a linear placeholder, not a real calibration** — MQ-2 requires a logarithmic curve fit against its Rs/Ro datasheet graphs plus a clean-air baseline unique to each physical unit. Current output should not be trusted as a real gas concentration. |

### `Logic/` — Fusion & Safety Decisions

| File | Status | Notes |
|---|---|---|
| `motion_validator.c/.h` | ✅ Done (rough) | Detects stall (commanded motion, no encoder change) and unexpected motion (encoder change with no command). IMU cross-check is intentionally coarse — compares raw acceleration magnitude against 1g, doesn't account for robot orientation/tilt. Stall/motion thresholds are unvalidated guesses pending real hardware testing. |
| `fire_state_machine.c/.h` | ✅ Done (rough) | Aggregates MLX90614, ADT7482, and MQ-2 readings into IDLE / WARNING / CONFIRMED / SENSOR_FAULT states. Requires ≥2 independent confirming indicators before escalating to CONFIRMED, to reduce false positives from any single unverified sensor. **Output is only as trustworthy as its least-verified input** — see ADT7482, MLX90614, and MQ-2 notes above. Temperature/gas thresholds are placeholders, not calibrated against real fire scenarios. |

### `Protocol/` — STM32 ↔ ESP32 Framing

| File | Status | Notes |
|---|---|---|
| `packet.c/.h` | ✅ Done | Frame format: `[0xAA][MSG_TYPE][LENGTH][PAYLOAD][XOR_CHECKSUM]`. Byte-at-a-time parser state machine, auto-resyncs on garbage/corruption. Message types (`MSG_HEARTBEAT`, `MSG_SENSOR_DATA`, `MSG_FIRE_ALERT`, `MSG_MOTION_STATUS`) are placeholders pending finalized payload definitions. |

---

## Known Gaps / TODOs

Ordered roughly by priority before this can be trusted on real hardware:

1. **Wire up `main.c`** — none of the `_Init()` functions (`Encoders_Init`, `IMU_Init`, `INA226_Init`, etc.) or periodic update calls (`MotionValidator_Check`, `FireStateMachine_Update`) are invoked yet. The architecture compiles and links, but nothing runs at startup until this is done.
2. **Confirm I2C addresses** against actual board wiring — LSM6DS3, INA226, ADT7482 addresses depend on address-select pin strapping; current values assume common defaults.
3. **Verify ADT7482 register map** against the real datasheet.
4. **Calibrate INA226** (`SHUNT_RESISTANCE_OHMS`, `CURRENT_LSB_A`) against the actual shunt resistor on the board.
5. **Implement MLX90614 PEC validation** (SMBus CRC-8) before trusting readings in a fire-detection decision path.
6. **Calibrate MQ-2** with a real logarithmic curve and clean-air baseline — current PPM output is not meaningful.
7. **Integrate ST's official VL53L0X API** (STSW-IMG005) to replace the uncalibrated minimal driver, if proximity accuracy matters for the final design.
8. **Tune all thresholds** in `motion_validator.c` and `fire_state_machine.c` against real hardware behavior — current values are unvalidated placeholders.
9. **Test UART link recovery** against an actual induced error (mismatched baud, ESP32 disconnect) to confirm `HAL_UART_ErrorCallback` re-arms correctly on real hardware, not just by code inspection.

---

## Toolchain

- **IDE/Build:** STM32CubeIDE, STM32CubeMX (`.ioc`-based HAL/CMSIS generation), CMake + Ninja, VSCode with CMake Tools + clangd
- **Version control:** Git + GitHub
- **Reference manual:** STM32F407VGT6 RM0090

## Building

```bash
cmake -B build -S .
cmake --build build
```

Peripherals are configured via `stm32_blink_test.ioc` — open in STM32CubeMX/CubeIDE's graphical editor, **not** as raw text, and use the explicit "Generate Code" action (not just Ctrl+S) after any pin/peripheral change, or generated HAL files (`usart.h`, `i2c.h`, `adc.h`, `tim.h`, etc.) won't be regenerated.
