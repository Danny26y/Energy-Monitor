# Next steps

## Resume the simulation

1. Open `EnergyMonitor.pdsprj` in Proteus 8.17 and check ARD1's Program File points to the local `build/EnergyMonitor.ino.hex`. The saved path is machine-specific. The third-party Arduino Uno model must be installed separately; it is not redistributed here.
2. Run at RV2=0%. Expect 0 W, 2.000 kWh and 27 passing startup checks in TERM1 (9600 baud, 8N1).
3. Enter `ADD 0.500` followed by Enter; expect 2.500 kWh at zero load. `ADD -1` must be rejected. Top-ups are manually entered energy units, not utility tokens.
4. Check RV2 at 50% and 100%, then return it to 0%. Expect approximately 1000 W and exactly 2000 W. Allow about 12 seconds for the average to settle.
5. For a fresh low-credit test, rebuild the current source with a temporary starting credit of 5000 mWh (0.005 kWh), use that separate HEX, and test LOW, CREDIT EMPTY, and recovery with `ADD 0.500`. Restore the normal HEX afterward. The saved historical test builds are not the current release.

## Finish the simulation prototype

1. Add and validate an external low-credit LED/buzzer on the existing D13 status output, with appropriate drive circuitry. Verify blink/steady/clear behavior.
2. Implement persistent credit with versioned records, integrity checks and write-wear management; test reset and interrupted-save recovery. Credit currently resets to the configured start on every restart.
3. Add a deliberate command for setting an initial/reconciled balance if needed. `ADD` currently adds to the existing 2 kWh default, so entering an existing meter balance directly would overstate credit.
4. Review accounting at top-up/empty transitions and sampling boundaries; characterize timing error before claiming precision. Current accounting samples about every 100 ms and is not billing-grade.
5. Decide whether a longer forecasting window better represents household usage. Validate short seconds/minutes display and changing-load estimates with timed logs.
6. If a full schematic redraw is desired, replace long LCD/power routes with named nets and clear functional blocks, preserving the documented pin map. Separate the inherited auxiliary RC/voltmeter experiment from the main monitor. The current cleanup retains that branch.

## Plan a physical prototype

Choose an isolated voltage/current measurement approach and document its specifications before making a hardware schematic or bill of materials. Real AC power requires time-aligned voltage and current samples: average `v(t) * i(t)`; then integrate watts over time to obtain kWh. RV2 only supplies a synthetic load value.

Begin with an isolated low-voltage bench demonstrator, validate sensor scaling, offsets, zero-load noise, calibration, varying loads and reset behavior. Then design the enclosure, protected power supply, PCB and installation with an appropriately qualified electrical professional. No direct Arduino-to-mains connection or modification of the MOJEC meter is part of this project.

For the current prototype, a computer connected by USB supplies the serial credit-entry interface. A standalone keypad/button interface is an optional later feature and is not implemented. The monitor's manually maintained balance remains separate from the utility meter/account.
