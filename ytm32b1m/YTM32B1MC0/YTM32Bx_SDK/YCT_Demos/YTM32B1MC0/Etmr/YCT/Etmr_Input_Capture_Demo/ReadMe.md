# Etmr_Input_Capture_Demo

---

## Hardware Requirements

- **EVB**: YTM32B1MC0-EVB-Q64

---

## Software Requirements

- **SDK Version**: YTM32B1MC0_1_3_1 or later version
- **Config Tool Version**: YCT v2.7.4 or later version

---

## Hardware Setup on EVB

The following table shows the pin configuration of demo:

|  Pin        |    Function   |
|  ------     |    -----      |
|  PTC2       |    eTMR0_CH2  |
|  PTC3       |    eTMR0_CH3  |
|  PTB4       |    eTMR0_CH4  |
|  PTB5       |    eTMR0_CH5  |
|  PTE8       |    eTMR0_CH6  |
|  PTE9       |    eTMR0_CH7  |
|  PTB2       |    eTMR1_CH0  |
|  PTB3       |    eTMR1_CH1  |
|  PTC14      |    eTMR1_CH2  |
|  PTC15      |    eTMR1_CH3  |
|  PTC0       |    eTMR1_CH6  |
|  PTC1       |    eTMR1_CH7  |

Where eTMR0's channels 2-7 output PWM signals, which are captured by eTMR1's channels 0-3 and channels 6-7.

---

## Function Description

This demo shows the six-channel input capture function of eTMR1. The frequency of the PWM signal output by eTMR0 is 1kHz, and the duty cycle is 99%, 99%, 99%, 1%, 1% and 1% respectively.

**Note**:

1. If the frequency is 2kHz and the minimum duty cycle is still 1%, then only 4 channels can be set at most; if more channels are set to capture at the same time, the minimum duty cycle that can be captured will be greater than 1%. Set according to the actual situation.
2. This demo only considers the case of one overflow, so it is necessary to set an appropriate prescaler to ensure that the counter value does not exceed the case of one overflow.
