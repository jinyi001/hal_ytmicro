# Etmr_Global_Time_Base_Demo

---

## Hardware Requirements

- **EVB**: YTM32B1MC0-EVB-Q64

---

## 软件要求

- **SDK version**: YTM32B1MC0_1_3_1 or later version
- **Tool Version**: YCT v2.7.4 or later version

---

## Hardware Setup on EVB

The following table shows the pin configuration of demo：

| Pin    | Function  |
|--------|-----------|
| PTC_2  | eTMR0_CH2 |
| PTB_2  | eTMR1_CH0 |
| PTA_11 | eTMR1_CH5 |

The following table shows the hardware connection on EVB, short-L is connected to short-R：

| Short-L | Short-R | Description   |
|---------|---------|---------------|
| J1-2    | 5VUSB   | USB 5V supply |
| J2-2    | 5V      | VDD 5V        |
| J3-1    | J3-2    | MCU work      |
| J4-1    | J4-2    | VDDA 5V       |
| J21-12  | -       | eTMR0_CH2     |
| J20-9   | -       | eTMR1_CH5     |

---

## Function description

This demo shows how to use multiple eTMRs to generating PWM signals at the same time.

1. Connect the logic analyzer or oscilloscope to J21-12, J20-9.
2. The logic analyser or oscilloscope will display two PWM signals being generated at the same time. PWM frequency is 2kHz and duty cycle is 50%.

---
