# Change record

## 7 November 2026 — schematic cleanup and repository preparation

- Preserved the pre-cleanup native project in `backups/2026-11-07-pre-cleanup/EnergyMonitor.pdsprj`. Earlier originals and backups remain on disk.
- Moved RV1 beside LCD1 so the contrast control is grouped with its display.
- Shortened the contrast wiper wire loop and moved its horizontal section above the LCD signal bundle.
- Moved the long RV1 supply-return wire nearer the left supply wiring, reducing its bottom loop.
- Added a schematic title identifying the 0–2000 W simulation and kWh credit units.
- Preserved the Arduino pin assignments, RV2 input, serial connections, auxiliary RC branch and firmware. This is a modest layout cleanup, not a complete schematic redraw.
- Saved before/after and running-validation screenshots in `screenshots/`.
- Restarted the edited project: LCD showed 0 W / 2.000 kWh; serial showed ADC=0, average=0, idle forecast, and SELFTEST PASS 27 checks with zero failures.
- Added next-step and Git instructions. Local originals, backup archives, historical experiments and generated intermediate files are excluded from Git; they have not been deleted.

The previous zero/full-scale, serial top-up, warning and forecast results are recorded in `VALIDATION.md`. Only normal startup/internal checks were repeated after this layout change; the entire interactive test sequence was not repeated.

## Earlier prototype work

- Implemented configurable starting credit, integer energy accounting, a 12-sample rolling forecast, low/empty warnings and serial credit entry.
- Corrected the simulated input direction and verified native RV2 endpoints: 0% gives 0 W; 100% gives 2000 W.
- Corrected UART connections and LCD contrast/supply references; verified serial top-ups and invalid-amount rejection.
- Compiled the normal Uno firmware and preserved original projects. See `VALIDATION.md` and the historical `SESSION_NOTES.md` for details and limits.
