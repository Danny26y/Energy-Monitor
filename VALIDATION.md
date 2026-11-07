# Simulation validation — 7 November 2026

Results below distinguish UI-observed simulation behavior from internal arithmetic checks. This does not certify physical hardware.

After the 7 November layout cleanup, the edited native project was saved and restarted successfully. LCD: 0 W / 2.000 kWh. Terminal: ADC=0, W=0, AvgW=0.0, credit=2.000000 kWh, idle forecast; SELFTEST PASS 27 checks, failures=0. See `screenshots/03-running-selftest.png` and `04-serial-startup.png`. Other interactive tests below were observed before this layout-only cleanup, not all rerun afterward.

| Check | Evidence/result |
| --- | --- |
| Normal boot | Serial output and LCD ran; 2.000 kWh default, ADC 0 / 0 W |
| Internal diagnostics | SELFTEST PASS 27 checks; failures=0, observed in the Proteus virtual terminal |
| Zero input | ADC 0 / 0 W; credit stopped decreasing at zero load |
| Midpoint | Earlier observed 1001 W at 50%, consistent with ADC rounding |
| Full scale | Native RV2 controls reached 100%; LCD 2000 W and terminal ADC=1023 W=2000 AvgW=2000.0 |
| UART credit entry | ADD 0.500 accepted; 2.000 → 2.500 kWh at zero load |
| Invalid amount | ADD -1 rejected; balance unchanged at zero load |
| Low warning | Temporary 0.005 kWh build at 2000 W showed LOW on LCD and LOW state in serial output |
| Empty balance | Same test reached CREDIT EMPTY by the 11.6 s observation; expected exhaustion around 9 s, between sampled observations |
| Top-up recovery | From empty, ADD 0.500 returned balance to 0.500000 kWh, cleared empty warning, and showed normal credit display while load remained 2000 W |
| Forecast | At 0.500 kWh and 2000 W, serial forecast was 0.250 hours; after load stepped to zero the LCD estimate changed gradually, then terminal reported AvgW=0.0 and Hours=IDLE |
| Stability | Zero load remained running beyond two simulated minutes without the previous maximum-power artifact |
| Build | Final normal firmware compiled: 10,430 bytes flash, 442 bytes static RAM |

## What the 27 checks cover

The checks execute the actual MonitorCore code on a separate instance inside the simulated Uno: zero-load accounting, 2000 W for 60,000 ms in one interval and 600 smaller intervals, remainder carry, zero clamping, top-ups, balance limits, low threshold boundaries, initial/filled/replaced rolling history, decimal amount parsing, malformed/negative/overflow amounts and unsigned time subtraction at wrap.

For the 60 s arithmetic case, 2000 W consumes 0.033333333 kWh. The integer state retains 1,966,667 mWh from 2,000,000 mWh and carries 1200 W*ms as the remaining fractional usage. This check is not a separately logged 60-second steady-load hardware experiment.

## Limits and follow-up work

- Balance is volatile and resets on restart. No EEPROM persistence, payment verification, tariff calculation or utility integration is provided.
- RV2 is a synthetic DC input; the circuit does not acquire real AC voltage/current or calculate real power factor.
- D13 status output is implemented but an external LED/buzzer was not added or validated.
- The final change to short forecast formatting (seconds/minutes) compiled successfully; the earlier warning run used hour formatting. Normal hours and idle behavior were observed.
- Simulation logs include third-party UI cursor warnings. The repeated application crashes during bulk property-text edits were recorded in Windows Application Error events: NETLIST.dll, exception 0xc0000409. Recovery via saved project and native controls worked. No system/security settings were changed.
- The auxiliary resistor/capacitor/voltmeter branch remains from the original design and is not the load sensor.

Final deliverable is the documented Proteus energy-monitor simulation and matching Uno source/HEX. Physical enclosure, PCB, electrical isolation, measurement calibration and installation are separate engineering work.
