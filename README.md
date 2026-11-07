# Arduino energy monitor — Proteus simulation

Updated 7 November 2026. This project simulates an adjustable 0–2000 W load, an energy-credit balance, a rolling consumption forecast, and low/empty-credit warnings. Credit is entered through a local serial terminal in kWh. It is independent of any utility account or MOJEC meter.

## Start here

See [CHANGELOG.md](CHANGELOG.md) for the latest schematic cleanup, [NEXT_STEPS.md](NEXT_STEPS.md) to resume development, [saved screenshots](screenshots/README.md) for visual evidence, and [GITHUB.md](GITHUB.md) for publishing commands.

1. Open `EnergyMonitor.pdsprj` in Proteus 8.17.
2. ARD1's Program File should point to this folder's `build/EnergyMonitor.ino.hex`. The saved path is absolute; if moving the project, update that field. Custom Arduino UNO library/model files already installed on this machine are also required.
3. Run the simulation. The normal build starts with **2.000 kWh**, and the saved RV2 setting is **0% / 0 W**.
4. Open TERM1's Virtual Terminal if its window is hidden. Settings are **9600 baud, 8 data bits, no parity, 1 stop bit**, normal polarity, no flow control.
5. Click inside the terminal, type a command, and press Enter. For pasted commands, include a newline. The right-click **Paste** command was verified; ordinary automated keyboard input did not reliably reach this popup.

## Commands

| Command | Result |
| --- | --- |
| `ADD 0.500` | Adds 0.500 kWh to the current balance |
| `STATUS` | Reports ADC reading, instantaneous/average watts, credit, forecast and simulation milliseconds |
| `STREAM ON` | Reports status about once per second |
| `STREAM OFF` | Stops periodic reports |
| `TEST` | Runs 27 internal checks on a separate test instance without changing the live balance |
| `HELP` | Lists commands |

Amounts must be positive decimal kWh with at most six decimal places. Negative values, zero top-ups, exponents, trailing junk and balances above 1000 kWh are rejected. Commands accept either letter case. A line longer than 47 characters is rejected in full. The firmware accepts CR, LF, or CRLF line endings.

**Restart behavior:** stopping/restarting the simulation restores the configured starting credit. Top-ups and usage are held in RAM; persistent balance storage and utility-token processing are not implemented.

## Controls and display

- RV2 simulates the sensor input: 0% → 0 W, 50% → about 1000 W, 100% → 2000 W. Use the native wiper/control buttons. The midpoint may display 1001 W because the ADC has 1024 discrete levels.
- RV1 controls LCD contrast. Its endpoints are connected to VCC and GND; its wiper drives LCD VEE.
- The LCD shows power on the first row. The second row alternates between credit and estimated time. LOW appears when credit is at or below 0.200 kWh; an additional low-credit message page is available. Zero balance displays CREDIT EMPTY.
- Short forecasts use seconds or minutes; longer forecasts use hours. After sustained zero load fills the 12-sample window, the forecast displays idle. A load change intentionally takes about 12 seconds to fully replace the history.
- D13 is driven as an optional status output: blinking for low credit, steady high for empty. There is no external alarm LED/buzzer in this schematic, and visible D13 indication on the third-party Uno model was not validated.
- The existing R1/R2/C1/voltmeter branch is retained as an auxiliary voltage-reference test. It does **not** measure the simulated load; A0/RV2 is the load input.

## Wiring reference

| Signal | Connection |
| --- | --- |
| LCD RS, E | Arduino D12, D11 |
| LCD D4, D5, D6, D7 | Arduino D5, D4, D3, D2 |
| LCD RW and VSS | GND |
| LCD VDD | VCC |
| LCD VEE | RV1 wiper |
| RV1 outer endpoints | VCC and GND |
| RV2 outer endpoints | VCC and GND |
| RV2 wiper | Arduino A0 |
| Arduino D1/TX | TERM1 RXD |
| Arduino D0/RX | TERM1 TXD |
| Arduino/LCD supplies | Explicit common VCC and GND references |

## Firmware and configuration

Open `EnergyMonitor/EnergyMonitor.ino` in Arduino IDE, with `MonitorCore.h` beside it. Board: **Arduino Uno**. Dependencies used: Arduino AVR Boards 1.8.8 and LiquidCrystal 1.0.7. No physical-board upload is needed to run Proteus.

- `START_CREDIT_MWH`: starting credit, normally 2,000,000 mWh = 2 kWh.
- `LOW_CREDIT_MWH` in MonitorCore.h: warning threshold, 200,000 mWh = 0.2 kWh.
- `MAX_POWER_W`: full-scale simulated power, 2000 W.
- `REVERSE_SENSOR`: false for the saved wiring.

The final normal build uses 10,430 bytes of flash (32%) and 442 bytes of static RAM (21%). It was compiled for `arduino:avr:uno`.

Energy integration uses the previously sampled load and elapsed simulated milliseconds, sampled approximately every 100 ms. Integer mWh accounting carries the fractional W*ms remainder to avoid repeated floating-point subtraction from the balance. Sampling and display are not instantaneous physical measurements. Forecasts are estimates from the last 12 roughly one-second samples, including partial history at startup.

## Verified behavior

See `VALIDATION.md` for measured results and limits. The simulation has passed zero/full-scale input checks, UART top-up/rejection checks, low-to-empty transition and recovery, and rolling-average idle behavior. This is a simulation prototype, not production-ready physical hardware. No real mains wiring or MOJEC meter was accessed.

## Preservation and diagnostics

Original projects/sketches are in `originals/`; dated snapshots are in `backups/`. `SESSION_NOTES.md` contains historical findings and is superseded by this README and VALIDATION.md for the current state.

Do not replace all Proteus component properties with one long unbroken string. That editing path repeatedly crashed this installation in NETLIST.dll before simulation began. Reloading the saved project and using native potentiometer controls restored operation, including the full 100% endpoint. This was not an Arduino firmware crash or evidence that maximum load is unsupported.

`test-low/` contains a separate test build starting with 0.005 kWh. At 2000 W it reaches empty in about nine seconds. Do not substitute it for the normal build unintentionally. The older `test-empty/` artifacts are retained historical test files, not the final normal firmware.
