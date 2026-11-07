# Energy monitor prototype — 29 October 2026

## Files and preservation

Work was performed in a separate `EnergyMonitor.pdsprj`, saved through Proteus Save Project As. The original projects and Arduino sketch were not overwritten.

`originals/` contains the Desktop project that was actually open, the separate Documents project, the original sketch, and its compiled HEX. These are separate project versions, not interchangeable backups of one identical circuit.

Open `EnergyMonitor.pdsprj` for the revised circuit. Its Arduino program reference points to `build/EnergyMonitor.ino.hex` using an absolute path within this folder. The matching editable sketch is `EnergyMonitor/EnergyMonitor.ino`. `build/EnergyMonitor.normal.hex` is another copy of the normal build. The test-low and test-empty subfolders are separate firmware test variants; they have NOT been validated in Proteus.

## Observed diagnosis

Direct Windows desktop access worked. The initially open project had no RV2, an unconnected A0, and no explicit VCC/GND references on the Arduino/LCD supply wiring. It displayed 2000 W. This differs from the earlier conversation's RV2 circuit, so that earlier circuit's exact fault has not been conclusively reproduced.

Grounding A0 to the existing Arduino GND wiring still displayed 2000 W. Adding an explicit GND reference alone did not fix it. Connecting the Arduino VCC wiring to the existing VCC terminal as well produced 0 W at grounded A0. Supply/reference wiring therefore contributed to the incorrect result in this project. Do not solve this by blindly reversing the ADC formula.

Temporary input wiring was removed and a linear 1 kOhm POT-HG RV2 was added: upper endpoint to VCC, lower endpoint to GND, wiper to A0. Explicit common GND and VCC connections remain in the saved project. The unrelated resistor/capacitor/voltmeter network was retained.

## Firmware changes

- Starting energy credit defaults to 2.000 kWh (`START_CREDIT_MWH = 2000000`). Credit resets when the simulation restarts; no persistent balance or top-up interface is implemented.
- Low-credit threshold is 0.200 kWh (`LOW_CREDIT_MWH = 200000`). A rotating LCD page displays LOW CREDIT!; zero balance displays CREDIT EMPTY. These messages do not disconnect a load.
- Forecast uses the most recent 12 power samples, taken about once per second. The partial window is used during startup. Forecast is an estimate based on recent simulated load, not a guaranteed remaining time.
- Energy is accumulated in integer mWh with residual W*ms carried forward, avoiding repeated subtraction of tiny floating-point values from a kWh balance. The previous sampled load is integrated over each approximately 100 ms interval.
- Unsigned elapsed-time subtraction handles millis wrap. LCD updates every 2 seconds, clearing each row before writing to prevent stale characters. There is no blocking delay in the main loop.
- `REVERSE_SENSOR` remains false. The simulation maps ADC 0 to 0 W and ADC 1023 to 2000 W.

## Validation actually completed

1. Original baseline ran and displayed 2000 W.
2. Revised firmware compiled for Arduino Uno: 5406 bytes flash and 196 bytes static RAM with Arduino AVR core 1.8.8 and LiquidCrystal 1.0.7.
3. Grounded A0 with explicit supply references displayed 0 W and 2.000 kWh.
4. RV2 at 50% displayed 1001 W; decreasing RV2 decreased the power reading.
5. RV2 explicitly set to POS=0, STATE=0 displayed 0 W and 2.000 kWh on subsequent observations. This is a short functional check, not a long-duration energy drift test.
6. Separate 0.100 kWh and zero-credit firmware variants compiled successfully; their LCD behavior has NOT been observed.

The last confirmed project save was made with RV2 at 18%. Later tests changed RV2 to zero and then 100% in memory. The zero test passed. During the subsequent maximum-load restart attempt, the Proteus window disappeared from the window inventory. The cause is unknown. The 100% test was NOT verified, and those later in-memory position settings were not confirmed saved. Do not report a full validation pass.

## Next session

- Reopen the saved project and check that the normal build loads.
- Verify RV2 endpoints (0%, 50%, 100%) and inspect any simulation warnings/logs.
- Check steady-load energy accounting against P(W) * elapsed seconds / 3,600,000 kWh; e.g. 2000 W for 60 simulation seconds consumes about 0.033333 kWh.
- Verify forecast response to a load step and idle forecast after the rolling window fills with zeros.
- Load the separate low-credit and empty-credit test HEX files to verify the warning pages, then restore the normal build reference.
- Audit LCD contrast-pot endpoints, supply/reference connections, and the retained source network before considering further circuit cleanup.

This is a Proteus prototype only. No real mains wiring or MOJEC meter was accessed or changed, and no production-ready hardware claim is made.

## Current status — 7 November 2026
Superseded by README.md and VALIDATION.md. Serial credit entry and negative-input rejection were verified through TERM1 Paste; 2000 W full scale, LOW, CREDIT EMPTY, top-up recovery and forecast-to-idle were observed. Final normal 2 kWh firmware restored and tested; 27 checks pass. Updated source compiles to 10430 bytes flash / 442 bytes static RAM. The project was saved with RV2 at zero. Bulk all-properties text editing caused NETLIST.dll crashes; native controls worked through 100%.
