# Mpwm_Pwm_Demo

---

## Hardware Requirements

- **EVB**: YTM32B1MC0-EVB-Q64

---

## Software Requirements

- **SDK Version**: YTM32B1MC0_1_3_1 or later version
- **Config Tool Version**: YCT v2.6.3 or later version

---

## Hardware Setup on EVB

The following table shows the pin configuration of demo:

|  Pin        |    Function   |
|  ------     |    -----      |
|  PTE5       |    MPWM0_CH3  |

The following table shows the hardware connection on EVB, short-L is connected to short-R:

|  Short-L     |    Short-R   |    Description   |
|  ------      |    -----     |    ----          |
|  J1-2        |    J1-3      |    USB 5V supply |
|  J2-1        |    J2-2      |    VDD 5V        |
|  J3-1        |    J3-2      |    MCU work      |

---

## Function Description

This demo shows pwm output for channel 3 of MPWM0. The frequency is 10KHz, the initial duty cycle is 50%. And the duty cycle will be updated each 10ms.
