# Smart Solid-State Circuit Breaker Firmware

First-stage firmware framework for TI TMS320F280049C/F280049CPZQ based on CCS 12.4 and C2000Ware.

## Layout

- `app`: startup flow, scheduler, system state machine
- `bsp`: board-level init and idle entry
- `driver`: ADC, ePWM, CMPSS, CAN, SPI wrappers
- `protection`: fault detection, RMS, I2T, recovery and lockout logic
- `protocol`: CAN frame encoding and command handling
- `storage`: FM25V20A FRAM access, parameters, fault log
- `selftest`: power-on and runtime self-checks
- `common`: shared types, config, CRC, time base
- `tests`: host-side logic tests

## Build Modes

Target firmware builds should define `SSCB_TARGET_C2000` and include C2000Ware driverlib headers/libraries through CCS project variables.

Host-side tests do not define `SSCB_TARGET_C2000`; hardware drivers compile as stubs so protocol, storage and protection logic can be validated without hardware.

## Hardware Map

- Voltage ADC: ADCB3, pin 8
- Current ADC: ADCB0, pin 41
- Temperature ADC: ADCB1, pin 40
- Short-circuit comparator: CMPSS3H, pin 7
- Gate PWM: ePWM3A, pin 75
- CAN: CANB, 500 kbps, standard 11-bit frames
- FRAM: FM25V20A on SPIB

