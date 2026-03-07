# Description

The YTM32 Software Development Kit (YTM32 SDK) is an extensive suite of peripheral abstraction layers, peripheral drivers, RTOS and so on designed to simplify and accelerate application development on Yuntu Microcontrollers.

# Compiler and IDE Versions

SDK contains many demos base on Yuntu Config Tool(YCT), and the version of supported IDE as follows:

- GCC (10.3.1)
- IAR (8.40)
- MDK (5.35)
- GHS (2019.1.4)
- YuntuIDE

# YTM32 SDK Release Change Log

## YTM32B1LE1

### 1_3_1 2024-09-09
First release.

---

## YTM32Z1LS0

### 1_3_1_HF002 2025-06-06
该版本是YTM32Z1LS0 1.3.1版本的第二版修复版本YTM32Z1LS0 1.3.1 hotfix 002。以下是更新说明。
- **platform更新**
  - 修复了进低功耗之后systick挂起的中断会立即唤醒MCU的问题。涉及power_manager_YTM32B1Lx.c，在进低功耗之前清除pending状态。
  - PINS更新
    - 将PINS_DRV_WritePin和PINS_DRV_WritePinsAPI的底层实现改为操作PSOR和PCOR寄存器。涉及pins_gpio_hw_access.h文件，修改了PINS_GPIO_WritePin函数的底层实现。
    - 增加了PINS_DRV_ReadPin API。涉及pins_driver.c/.h文件。
  - DMA更新
    - 修复了ram reload功能不会在最后一个cts结构体禁掉的问题。涉及dma_driver.c。
  - Flash更新
    - 修复了EFM预分频值，使得EFM时钟等于slow bus时钟。涉及clock_YTM32B1Lx.c文件中CLOCK_SYS_SetScuConfiguration函数里EFM prescaler的配置，以及YTM32Z1LS0_features.h文件中FEATURE_EFM_OPERATION_CLOCK_SOURCE宏的定义。
    - 增加了EFM CTRL寄存器中READONLY位的功能，其使用详见论坛，链接如下 https://forum.ytmicro.com/topic/1086/efm-readonly-bit%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E
  - FEE更新
    - 修复了FEE在同步模式下使用多个sector作为一个cluster时不能擦除下一个sector的问题。
    - 增加了Fee_Cancel API。
  - FlexCAN更新
    - 修复了当存在模块RAM ECC错误时，FLEXCAN_Enable若被中断打断，可能会造成状态判断卡死。涉及到flexcan_hw_access.c文件中FLEXCAN_Enable函数的改动。
    - 修复了数据扩展帧发送时邮箱SRR位配置出错的问题，由显性改为隐性使其严格遵从协议规范，防止兼容性问题，建议修改。涉及到flexcan_hw_access.c文件中FLEXCAN_SetTxMsgBuff函数的改动。
  - UART更新
    - 修复了LIN模式在接收同步，发送/接收PID状态下不支持超时处理的问题。
    - 修复了在修改INTE寄存器时被中断重入造成中断中的INTE修改不成功的问题。因此在UART_SetIntMode函数中加入原子操作，保证INTE的配置修改都能成功。
  - SPI更新
    - 修复了在主机下使用DMA单线模式收不到数据的问题。涉及SPI_DRV_MasterStartTransfer和SPI_DRV_SlaveTransfer两个函数的修改。
    - 修复了当FrameSize=0时，TXCFG寄存器会被写入错误值的问题。涉及spi_hw_access.c文件中SPI_SetTxCommandReg函数的修改，以及spi_master_driver.c文件中对于bitcount=0的处理。
  - eTMR更新
    - 修复了输入捕获信号宽度不准确和callback不只在捕获完成后进的问题。另外为了方便用户获取实时捕获值，增加了eTMR_DRV_GetInputCaptureValue API。涉及到etmr_ic_driver.c/.h。关于其使用参考Etmr_Input_Capture_Demo例程。
    - 修复了当配置结构体中加载频率或者输出脉冲频率为0时，写入寄存器值为0xF的问题。当配置为0时，会报错，配置时数值为n，那么就代表每n次加载一次，或者每n个脉冲就输出一个脉冲。涉及到etmr_common.c文件中eTMR_DRV_SetSync和eTMR_DRV_SetOutputTrigger两个函数的改动，以及status.h中增加了错误状态。
    - 增加了eTMR_DRV_SetGlobalTimeBase 和 eTMR_DRV_GenGlobalSignal 两个API。涉及到etmr_common.c/.h文件的改动。
- **YCT例程更新**
  - 在所有例程中增加了时钟初始化状态的判断检查。
  - 所有例程的代码在flash中8字节对齐，在ram中4字节对齐。
- **YCT配置工具界面更新**
  - 增加了多组管脚配置的功能。
  - 修改了ADC内部通道宏名。

### 1_3_1_HF001 2024-11-04
* [Fix] - [Toolchain] Fix IAR ALIGNED() macro issue.
* [Fix] - [DMA] Enable DMA channel interrupt separately.
* [Fix] - [DMA] Fix one dma channel starts may cause another closed channel restarts.
* [Fix] - [FlexCAN] Fix hardfault will occur when payload size exceeds 8 bytes if enable legacy rx fifo.
* [Fix] - [UART] UART may send error LIN frame data when PID equals numOfClassicPID.
* [Fix] - [eTMR] Fix one channel interrupt disablement affects the other channel interrupt enablement since they share the same irq number.
* [Fix] - [eTMR] Fix counter synchronization can not be disabled.
* [Fix] - [eTMR] Fix CHMASK register synchronization can not be disabled.
* [Fix] - [eTMR] Fix the return type of eTMR_DRV_CounterRead() api function. However, it can be replaced with eTMR_DRV_GetCntVal() api in the etmr_hw_access.c file.
* [Fix] - [SDK_GUI] There's no quadrature decoder state variable in config.c/h.
* [Fix] - [SDK_GUI] ADC external channel name error.
* [Fix] - [Demo] Some demos can not open normally.
* [New] - [Toolchain] Add Green Hills tool chain support.
* [New] - [eTMR] Add eTMR_DRV_GetInputCaptureValue() api.
* [Opt] - [eTMR] Improve eTMR fault related register configuration.

### 1_3_1 2024-07-18
* [Opt] - [SDK_GUI] Freertos and secure boot demo are generated through YCT.
* [Opt] - Add FXOSC gain scan when enabling FXOSC or waking up from low power mode.
* [Opt] - Display WDG, EMU, pTMR instance index for ipc clock name and irq name.
* [Opt] - Add ipc module reset api, update clock initialize flow.
* [Opt] - Put DMA_DRV_ClearIntStatus() api in front of callback in the DMA_DRV_IRQHandler() function.
* [Opt] - Dummy wait to ensure transfer complete signal is 0 for uart and lin.
* [Opt] - Clear the count of capture when measurement complete.
* [Fix] - RTOS would cause LPACK reset after wakeup.
* [Fix] - The pcs continuous function of spi is only valid for the first transmission.
* [Fix] - I2C timeout flag is set abnormally.
* [Fix] - I2C bus busy condition process in the I2C_DRV_MasterInit api.
* [Fix] - UART Tx/Rx DMA enablement can not be clear.
* [Fix] - ECC error when viewing register value of flexcan in the debug mode.
* [Fix] - DEV_ASSERT error in can pal layer when using enhanced FIFO.
* [Fix] - [SDK_GUI] The sequence of pins which have been configured will change when modifying the feature of pinout.
* [Fix] - [SDK_GUI] There's no value in parameter pullConfig of pinmux.c when just enabling pull enable.
* [Fix] - [SDK_GUI] Interrupt vector table update.
* [Fix] - [SDK_GUI] Data section is not initialzied completely when power on reset.
* [Fix] - [SDK_GUI] Configuration issue of driveSelect in pinout configuration.
* [Fix] - [SDK-GUI] DMA callback function is not declared externally when generating dma configuration.
* [Fix] - [SDK-GUI] FlexCAN payload size label is incorrect.
* [Fix] - [SDK_GUI] eTMR measurement configuration in input capture mode is missing.
* [Fix] - [SDK_GUI] eTMR overflow and fault handler name both mismatch.
* [Fix] - [SDK_GUI] ADC trigger type does not support hardware trigger.
* [New] - [SDK_GUI] Add api highlight for pTMR_DRV_InitChannel api.
* [New] - Open debug disable bit of CTRL_OVRD register.
* [New] - Add EMU_DRV_GetChannelSBInterruptFlag api.

### 1_3_0 2024-04-19
First release.

---

## YTM32B1HA0

### 1_3_1_HF001 2024-09-09
* [Opt] - [Clock] Optimize PLL initialization flow.
* [Opt] - [DMA] Enable DMA channel interrupt separately.
* [Opt] - [eTMR] Restore to enable channel irq number before eTMR enable, and add clearing interrupt flag before eTMR enable.
* [Opt] - [YCT_Demo] Add duty cycle update in the MPWM pwm demo.
* [Fix] - [Toolchain] Fix IAR ALIGNED() macro issue.
* [Fix] - [DMA] Fix one dma channel starts may cause another closed channel restarts.
* [Fix] - [LINFlexD] Fix linflexd_break_length_t enumeration value issue.
* [Fix] - [FlexCAN] Fix hardfault will occur when payload size exceeds 8 bytes if enable legacy rx fifo.
* [Fix] - [eTMR] Fix double switch can not be configured out of complementary mode.
* [Fix] - [eTMR] Fix counter synchronization can not be disabled.
* [Fix] - [eTMR] Fix one channel interrupt disablement affects the other channel interrupt enablement since they share the same irq number.
* [Fix] - [eTMR] Fix the return type of eTMR_DRV_CounterRead() api function. However, it can be replaced with eTMR_DRV_GetCntVal() api in the etmr_hw_access.c file.
* [Fix] - [MPWM] Fix MPWM channel combination source selection issue.
* [Fix] - [SDK_GUI] The lowest interrupt priority is wrong in the Freertos demo.
* [Fix] - [SDK_GUI] MPWM channel pair number is 1 by default and can not be set to 0.
* [Fix] - [SDK_GUI] There's no quadrature decoder state variable in config.c/h.
* [Fix] - [SDK_GUI] ADC external channel name error.
* [New] - [Toolchain] Add Green Hills tool chain support.
* [New] - [eTMR] Add eTMR_DRV_GetInputCaptureValue() api.

### 1_3_1 2024-07-18
* [Opt] - [SDK_GUI] Freertos and secure boot demo are generated through YCT.
* [Opt] - Add FXOSC gain scan when enabling FXOSC or waking up from low power mode.
* [Opt] - Display WDG, EMU, pTMR instance index for ipc clock name and irq name.
* [Opt] - Put DMA_DRV_ClearIntStatus api in front of callback in the DMA_DRV_IRQHandler function.
* [Opt] - Remove Timeout enable and disable operation in linflexd uart isr handler, and set to configurable in the linflexd yct demo by user.
* [Opt] - Dummy wait to ensure transfer complete signal is 0 for linflexd and sai module.
* [Opt] - Add ipc module reset api for module reset function.
* [Fix] - __vector_table not found occurred when debugging iar project.
* [Fix] - RWS is set as 2 in 100MHz fast bus in YCT GUI.
* [Fix] - RTOS would cause LPACK reset after wakeup.
* [Fix] - The pcs continuous function of spi is only valid for the first transmission.
* [Fix] - I2C timeout flag is set abnormally.
* [Fix] - I2C bus busy condition process in the I2C_DRV_MasterInit api.
* [Fix] - ECC error when viewing register value of flexcan in the debug mode.
* [Fix] - DEV_ASSERT error in can pal layer when using enhanced FIFO.
* [Fix] - The input size of flash api check issue.
* [Fix] - PLL clock frequency will overflow when selecting FIRC as reference for pll.
* [Fix] - MPWM disable counter api can not disable the counter of mpwm.
* [Fix] - The variable type of monitorIrqn in the intm_config_t will cause compile warning when using iar project.
* [Fix] - [SDK_GUI] The sequence of pins which have been configured will change when modifying the feature of pinout.
* [Fix] - [SDK_GUI] There's no value in parameter pullConfig of pinmux.c when just enabling pull enable.
* [Fix] - [SDK_GUI] Interrupt vector table update.
* [Fix] - [SDK_GUI] Data section is not initialzied completely when power on reset.
* [Fix] - [SDK_GUI] Configuration issue of driveSelect in pinout configuration.
* [Fix] - [SDK-GUI] DMA callback function is not declared externally when generating dma configuration.
* [Fix] - [SDK_GUI] WKU channel configuration issue.
* [Fix] - [SDK-GUI] FlexCAN payload size label is incorrect.
* [Fix] - [SDK_GUI] eTMR measurement configuration in input capture mode is missing.
* [Fix] - [SDK_GUI] ADC trigger type does not support hardware trigger.
* [Fix] - [SDK_GUI] INTM can not load IRQn of chip in the GUI.
* [Fix] - [SDK_GUI] INTM module initial api parameter issue.
* [New] - [SDK_GUI] Add pulse check in sent configuation ui.
* [New] - [SDK_GUI] Add mpwm module configuration ui.
* [New] - [SDK_GUI] Add api highlight for pTMR_DRV_InitChannel api.
* [New] - [YCT_Demo] Add flash disable debugger and flash ecc demos.
* [New] - [YCT_Demo] Add mpwm pwm demo.
* [New] - Open debug disable bit of CTRL_OVRD register.
* [New] - Add eTMR_DRV_SetCounterMid and eTMR_DRV_GetMid api.
* [New] - Add EMU_DRV_GetChannelSBInterruptFlag api.

### 1_3_0 2024-04-19
* [Fix] - [FLEXCAN] Add enhanceRxFifoState.state init when init flexcan.
* [Fix] - I2C won't detect line status when init function calls.
* [Fix] - I2C won't process SDA SCL low timeout event, user may need this callback for bus error process.
* [Fix] - LINFlexD deinit function disable IRQ flow error.

### 1_2_2 2024-04-12
The SDK code remains the same as the last version. Just YT Config Tool has been updated to fix the issue that would cause the loss of secure_boot_YTM32B1HA0.c/h file in the startup directory.

### 1_2_1 2024-03-28
* [Opt] - [FEE] Loosen the restriction that pflash cannot use fee
* [Fix] - [FEE] Fix sync mode donot launch command in RAM.
* [Fix] - [DMA] It issues BTCE error, if call DMA_CTSClearReg() when dma request enable.
* [Fix] - YCT Demos YT_LINK issue and the size of generated binary file is overlarge.

### 1_2_0 2024-03-12
* [Opt] - Enable/Disable EFM ECC Error interrupt should not change NVIC configuration
* [Opt] - Add FEATURE_EFM_OPERATION_CLOCK_SOURCE for YTM32B1Mx.
* [Opt] - Update mpwm driver
* [Opt] - Disable or enable each arithmetic by the HCU features of each device
* [Opt] - DMA_REQ_Disable change to DMA_REQ_DISABLED
* [Opt] - Optimize ACMP YCT demos
* [Opt] - Update all spi demos for new YT link.
* [Opt] - Wdg_Demo always reset, added abort condition.
* [Fix] - Defines of EFM_READ_COLLISION_IRQS are wrong in in YTM32B1HA0.h
* [Fix] - MPWM function redefinition
* [Fix] - Add global interrupt disable in the FLEXCAN_SetMsgBuffIntCmd() function
* [Fix] - The event type of error_callback in the FlexCANState should be flexcan_error_event_type_t 
* [Fix] - [TRNG] Cannot generate random data after TRNG_DRV_DeInit() and reinit.
* [Fix] - [Flash] Boot swap alway fail if fail once.
* [Fix] - Lin will stop transfer when bit error occurs.
* [Fix] - WDG reset value is not correct.
* [Fix] - CR[DSDIS] and CR[DBGDIS] bit logic error in the WDG driver.
* [Fix] - The value of typeOfUpdate is wrong when using eTMR_DRV_UpdatePwmPeriodAndDuty function
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitPwm after calling eTMR_DRV_DeinitPwm function
* [Fix] - Flash async mode cannot get idle by FLASH_DRV_GetBusyStatus() if occur access error.
* [Fix] - It would hardfault, if callback is not NULL in flash sync mode.
* [Fix] - Data left in RxFIFO cannot be read during i2c transmission timeout
* [Fix] - i2c deinit function does not clear the flag register
* [Fix] - The value of clock out divider can not be the value+1
* [Fix] - loading source and opportunity will be changed when calling eTMR_DRV_SetChnOutMask API
* [Fix] - Remove loading source and opportunity configuration in eTMR_DRV_SetCounterInit API
* [Fix] - It will generate assert error when calling eTMR_DRV_QuadDecodeStop API
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitOutputCompare after calling eTMR_DRV_DeinitOutputCompare API
* [Fix] - Etmr channel n(n>0) interrupt can not be set correctly
* [Fix] - Assert will be triggered when to capture just one channel
* [Fix] - When using the GPIO falling edge interrupt, an interrupt will be mistakenly responded to.
* [Fix] - Dead time can not be configured correctly when configuring multiple channels
* [Fix] - Counter initial and maximal value is not appropriate when configuring output compare or timer mode
* [Fix] - Odd channel io status is always 0 in complementary mode
* [Fix] - ACMP channel output status can not be obtained in continuous mode
* [Fix] - Warning reported when compiling linflexd file
* [Fix] - Continuous enable will not configured correctly in the ACMP_DRV_Init function
* [Fix] - In i2c slave mode, the restart interrupt event and the address match interrupt event may have an execution sequence error.
* [Fix] - When emu supports 64-bit injection, the EMU_SetErrInjectData function fails to inject a single bit and injects multiple bits.
* [Fix] - Wdg clock source error on file YTM32B1HA0_features.h .
* [Fix] - HA0 Secure Boot default configuration error
* [Fix] - FMU enumerated type mixed with another type which lead to iar compile warning.
* [Fix] - Mix type for s_sentFastDMASrc and s_sentSlowDMASrc which will lead to iar compile warning.
* [Fix] - Add fast bus clock for HA0 ipc clock source
* [New] - Add period calculation in capture period event
* [New] - Add new API MPWM_DRV_EnableDma() to enable channel dma feature
* [New] - Use customerized section to place ram function other than __ramfunc
* [New] - Add Emu_DoubleBit_Demo.
* [New] - Remove EMU_DRV_GetErrReportDataBit() API.

---

## YTM32B1ME0

### 1_3_1_HF001 2024-10-14
* [Opt] - [Clock] Optimize PLL initialization flow.
* [Opt] - [DMA] Enable DMA channel interrupt separately.
* [Opt] - [eTMR] Restore to enable channel irq number before eTMR enable, and add clearing interrupt flag before eTMR enable.
* **[Fix] - [MPU] Fix hardfault on unaligned access to flash address.**
* [Fix] - [Toolchain] Fix IAR ALIGNED() macro issue.
* [Fix] - [EFM] Fix flash readVerify configuration issue in the initial function.
* [Fix] - [DMA] Fix one dma channel starts may cause another closed channel restarts.
* [Fix] - [LINFlexD] Fix linflexd_break_length_t enumeration value issue.
* [Fix] - [FlexCAN] Fix hardfault will occur when payload size exceeds 8 bytes if enable legacy rx fifo.
* [Fix] - [eTMR] Fix double switch can not be configured out of complementary mode.
* [Fix] - [eTMR] Fix counter synchronization can not be disabled.
* [Fix] - [eTMR] Fix one channel interrupt disablement affects the other channel interrupt enablement since they share the same irq number.
* [Fix] - [eTMR] Fix the return type of eTMR_DRV_CounterRead() api function. However, it can be replaced with eTMR_DRV_GetCntVal() api in the etmr_hw_access.c file.
* [Fix] - [PTU] Remove PTU_DRV_ClearAdcPreTriggerSeqErrFlags() api.
* [Fix] - [SDK_GUI] There's no quadrature decoder state variable in config.c/h.
* [Fix] - [SDK_GUI] ADC external channel name error.
* [New] - [Toolchain] Add Green Hills tool chain support.
* [New] - [eTMR] Add eTMR_DRV_GetInputCaptureValue() api.

### 1_3_1 2024-07-18
* [Opt] - [SDK_GUI] Freertos and secure boot demo are generated through YCT.
* [Opt] - Add FXOSC gain scan when enabling FXOSC or waking up from low power mode.
* [Opt] - Display WDG, EWDG, EMU, pTMR instance index for ipc clock name and irq name.
* [Opt] - Put DMA_DRV_ClearIntStatus api in front of callback in the DMA_DRV_IRQHandler function.
* [Opt] - Remove Timeout enable and disable operation in linflexd uart isr handler, and set to configurable in the linflexd yct demo by user.
* [Opt] - Dummy wait to ensure transfer complete signal is 0 for linflexd module.
* [Opt] - Add ipc module reset api for module reset function.
* [Fix] - RTOS would cause LPACK reset after wakeup.
* [Fix] - The pcs continuous function of spi is only valid for the first transmission.
* [Fix] - I2C timeout flag is set abnormally.
* [Fix] - I2C bus busy condition process in the I2C_DRV_MasterInit api.
* [Fix] - ECC error when viewing register value of flexcan in the debug mode.
* [Fix] - DEV_ASSERT error in can pal layer when using enhanced FIFO.
* [Fix] - The input size of flash api check issue.
* [Fix] - [SDK_GUI] The sequence of pins which have been configured will change when modifying the feature of pinout.
* [Fix] - [SDK_GUI] There's no value in parameter pullConfig of pinmux.c when just enabling pull enable.
* [Fix] - [SDK_GUI] Interrupt vector table update.
* [Fix] - [SDK_GUI] Data section is not initialzied completely when power on reset.
* [Fix] - [SDK_GUI] Configuration issue of driveSelect in pinout configuration.
* [Fix] - [SDK-GUI] DMA callback function is not declared externally when generating dma configuration.
* [Fix] - [SDK_GUI] WKU channel configuration issue.
* [Fix] - [SDK-GUI] FlexCAN payload size label is incorrect.
* [Fix] - [SDK_GUI] eTMR measurement configuration in input capture mode is missing.
* [Fix] - [SDK_GUI] ADC trigger type does not support hardware trigger.
* [Fix] - [SDK_GUI] The number of ADC channel is incorrect in 64pin configuration.
* [Fix] - [SDK_GUI] INTM can not load IRQn of chip in the GUI.
* [Fix] - [SDK_GUI] INTM module initial api parameter issue.
* [New] - [SDK_GUI] Add api highlight for pTMR_DRV_InitChannel api.
* [New] - [YCT_Demo] Add flash disable debugger and flash ecc demos.
* [New] - Add eTMR_DRV_SetCounterMid and eTMR_DRV_GetMid api.
* [New] - Add EMU_DRV_GetChannelSBInterruptFlag api.
* [New] - Add EWDG_DRV_DisableInterrupt api.

### 1_3_0 2024-04-19
* [Fix] - [FLEXCAN] Add enhanceRxFifoState.state init when init flexcan.
* [Fix] - I2C won't detect line status when init function calls.
* [Fix] - I2C won't process SDA SCL low timeout event, user may need this callback for bus error process.
* [Fix] - LINFlexD deinit function disable IRQ flow error.

### 1_2_1 2024-03-28
* [Opt] - [FEE] Loosen the restriction that pflash cannot use fee
* [Fix] - [FEE] Fix sync mode donot launch command in RAM.
* [Fix] - Fix ME0 tmr driver to solve IAR warning
* [Fix] - [DMA] It issues BTCE error, if call DMA_CTSClearReg() when dma request enable.
* [Fix] - YCT Demos YT_LINK issue and the size of generated binary file is overlarge.

### 1_2_0 2024-03-12
* [Opt] - Enable/Disable EFM ECC Error interrupt should not change NVIC configuration
* [Opt] - Add FEATURE_EFM_OPERATION_CLOCK_SOURCE for YTM32B1Mx.
* [Opt] - Disable or enable each arithmetic by the HCU features of each device
* [Opt] - DMA_REQ_Disable change to DMA_REQ_DISABLED
* [Opt] - Optimize ACMP YCT demos
* [Opt] - Add a step to clear injection error for Emu_Demo.
* [Opt] - Wdg_Demo is always reset, added abort condition.
* [Opt] - MPU configuration would be overlapped, when boot uses SDK and app uses MCAL.
* [Fix] - Add global interrupt disable in the FLEXCAN_SetMsgBuffIntCmd() function
* [Fix] - The event type of error_callback in the FlexCANState should be flexcan_error_event_type_t 
* [Fix] - [TRNG] Cannot generate random data after TRNG_DRV_DeInit() and reinit.
* [Fix] - The return value of CRC_GetProtocolWidth function is not correct
* [Fix] - [Flash] Boot swap alway fail if fail once.
* [Fix] - Lin will stop transfer when bit error occurs.
* [Fix] - PWM channel pair set error when configuring odd channel invert in complementary mode
* [Fix] - CR[DSDIS] and CR[DBGDIS] bit logic error in the WDG driver.
* [Fix] - The value of typeOfUpdate is wrong when using eTMR_DRV_UpdatePwmPeriodAndDuty function
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitPwm after calling eTMR_DRV_DeinitPwm function
* [Fix] - Flash async mode cannot get idle by FLASH_DRV_GetBusyStatus() if occur access error.
* [Fix] - It would hardfault, if callback is not NULL in flash sync mode.
* [Fix] - Data left in RxFIFO cannot be read during i2c transmission timeout
* [Fix] - i2c deinit function does not clear the flag register
* [Fix] - The value of clock out divider can not be the value+1
* [Fix] - loading source and opportunity will be changed when calling eTMR_DRV_SetChnOutMask API
* [Fix] - Remove loading source and opportunity configuration in eTMR_DRV_SetCounterInit API
* [Fix] - It will generate assert error when calling eTMR_DRV_QuadDecodeStop API
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitOutputCompare after calling eTMR_DRV_DeinitOutputCompare API
* [Fix] - Etmr channel n(n>0) interrupt can not be set correctly
* [Fix] - Assert will be triggered when to capture just one channel
* [Fix] - When using the GPIO falling edge interrupt, an interrupt will be mistakenly responded to.
* [Fix] - Dead time can not be configured correctly when configuring multiple channels
* [Fix] - Counter initial and maximal value is not appropriate when configuring output compare or timer mode
* [Fix] - Odd channel io status is always 0 in complementary mode
* [Fix] - ACMP channel output status can not be obtained in continuous mode
* [Fix] - Warning reported when compiling linflexd file
* [Fix] - Continuous enable will not configured correctly in the ACMP_DRV_Init function
* [Fix] - In i2c slave mode, the restart interrupt event and the address match interrupt event may have an execution sequence error.
* [Fix] - [FLASH] Add nop command for FLASH_LaunchCommandSequence.
* [New] - Add EWDG_DeInit API
* [New] - Add period calculation in capture period event
* [New] - Add Emu_DoubleBit_Demo.
* [New] - Use customerized section to place ram function other than __ramfunc
* [New] - Remove EMU_DRV_GetErrReportDataBit() API.

---

## YTM32B1MD1

### 1_3_1_HF001 2024-10-14
* [Opt] - [Clock] Optimize PLL initialization flow.
* [Opt] - [DMA] Enable DMA channel interrupt separately.
* [Opt] - [eTMR] Restore to enable channel irq number before eTMR enable, and add clearing interrupt flag before eTMR enable.
* **[Fix] - [MPU] Fix hardfault on unaligned access of flash address.**
* [Fix] - [Toolchain] Fix IAR ALIGNED() macro issue.
* [Fix] - [EFM] Fix flash readVerify configuration issue in the initial function.
* [Fix] - [DMA] Fix one dma channel starts may cause another closed channel restarts.
* [Fix] - [LINFlexD] Fix linflexd_break_length_t enumeration value issue.
* [Fix] - [FlexCAN] Fix hardfault will occur when payload size exceeds 8 bytes if enable legacy rx fifo.
* [Fix] - [eTMR] Fix double switch can not be configured out of complementary mode.
* [Fix] - [eTMR] Fix counter synchronization can not be disabled.
* [Fix] - [eTMR] Fix one channel interrupt disablement affects the other channel interrupt enablement since they share the same irq number.
* [Fix] - [eTMR] Fix the return type of eTMR_DRV_CounterRead() api function. However, it can be replaced with eTMR_DRV_GetCntVal() api in the etmr_hw_access.c file.
* [Fix] - [PTU] Remove PTU_DRV_ClearAdcPreTriggerSeqErrFlags() api.
* [Fix] - [SDK_GUI] There's no quadrature decoder state variable in config.c/h.
* [Fix] - [SDK_GUI] ADC external channel name error.
* [New] - [Toolchain] Add Green Hills tool chain support.
* [New] - [eTMR] Add eTMR_DRV_GetInputCaptureValue() api.

### 1_3_1 2024-07-18
* [Opt] - [SDK_GUI] Freertos and secure boot demo are generated through YCT.
* [Opt] - Add FXOSC gain scan when enabling FXOSC or waking up from low power mode.
* [Opt] - Display WDG, EWDG, EMU, pTMR instance index for ipc clock name and irq name.
* [Opt] - Put DMA_DRV_ClearIntStatus api in front of callback in the DMA_DRV_IRQHandler function.
* [Opt] - Remove Timeout enable and disable operation in linflexd uart isr handler, and set to configurable in the linflexd yct demo by user.
* [Opt] - Dummy wait to ensure transfer complete signal is 0 for linflexd module.
* [Opt] - Add ipc module reset api for module reset function.
* [Fix] - RTOS would cause LPACK reset after wakeup.
* [Fix] - The pcs continuous function of spi is only valid for the first transmission.
* [Fix] - I2C timeout flag is set abnormally.
* [Fix] - I2C bus busy condition process in the I2C_DRV_MasterInit api.
* [Fix] - ECC error when viewing register value of flexcan in the debug mode.
* [Fix] - DEV_ASSERT error in can pal layer when using enhanced FIFO.
* [Fix] - The input size of flash api check issue.
* [Fix] - [YCT_Demo] EWDG demo can not enter into isr handler.
* [Fix] - [SDK_GUI] The sequence of pins which have been configured will change when modifying the feature of pinout.
* [Fix] - [SDK_GUI] There's no value in parameter pullConfig of pinmux.c when just enabling pull enable.
* [Fix] - [SDK_GUI] Interrupt vector table update.
* [Fix] - [SDK_GUI] Data section is not initialzied completely when power on reset.
* [Fix] - [SDK_GUI] Configuration issue of driveSelect in pinout configuration.
* [Fix] - [SDK-GUI] DMA callback function is not declared externally when generating dma configuration.
* [Fix] - [SDK_GUI] WKU channel configuration issue.
* [Fix] - [SDK-GUI] FlexCAN payload size label is incorrect.
* [Fix] - [SDK_GUI] eTMR measurement configuration in input capture mode is missing.
* [Fix] - [SDK_GUI] ADC trigger type does not support hardware trigger.
* [New] - [SDK_GUI] Add pulse check in sent configuation ui.
* [New] - [SDK_GUI] Add api highlight for pTMR_DRV_InitChannel api.
* [New] - [YCT_Demo] Add flash disable debugger and flash ecc demos.
* [New] - Add eTMR_DRV_SetCounterMid and eTMR_DRV_GetMid api.
* [New] - Add EMU_DRV_GetChannelSBInterruptFlag api.
* [New] - Add EWDG_DRV_DisableInterrupt api.

### 1_3_0 2024-04-19
* [Fix] - [FLEXCAN] Add enhanceRxFifoState.state init when init flexcan.
* [Fix] - I2C won't detect line status when init function calls.
* [Fix] - I2C won't process SDA SCL low timeout event, user may need this callback for bus error process.
* [Fix] - LINFlexD deinit function disable IRQ flow error.

### 1_2_1 2024-03-28
* [Opt] - [FEE] Loosen the restriction that pflash cannot use fee
* [Fix] - [FEE] Fix sync mode donot launch command in RAM.
* [Fix] - [DMA] It issues BTCE error, if call DMA_CTSClearReg() when dma request enable.
* [Fix] - YCT Demos YT_LINK issue and the size of generated binary file is overlarge.

### 1_2_0 2024-03-12
* [Opt] - Enable/Disable EFM ECC Error interrupt should not change NVIC configuration
* [Opt] - Add FEATURE_EFM_OPERATION_CLOCK_SOURCE for YTM32B1Mx.
* [Opt] - Disable or enable each arithmetic by the HCU features of each device
* [Opt] - DMA_REQ_Disable change to DMA_REQ_DISABLED
* [Opt] - Optimize ACMP YCT demos
* [Opt] - Add a step to clear injection error for Emu_Demo.
* [Opt] - Update all spi demos for new YT link.
* [Opt] - MPU configuration would be overlapped, when boot uses SDK and app uses MCAL.
* [Fix] - Add global interrupt disable in the FLEXCAN_SetMsgBuffIntCmd() function
* [Fix] - The event type of error_callback in the FlexCANState should be flexcan_error_event_type_t 
* [Fix] - [TRNG] Cannot generate random data after TRNG_DRV_DeInit() and reinit.
* [Fix] - The return value of CRC_GetProtocolWidth function is not correct
* [Fix] - [Flash] Boot swap alway fail if fail once.
* [Fix] - Lin will stop transfer when bit error occurs.
* [Fix] - PWM channel pair set error when configuring odd channel invert in complementary mode
* [Fix] - CR[DSDIS] and CR[DBGDIS] bit logic error in the WDG driver.
* [Fix] - The value of typeOfUpdate is wrong when using eTMR_DRV_UpdatePwmPeriodAndDuty function
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitPwm after calling eTMR_DRV_DeinitPwm function
* [Fix] - Flash async mode cannot get idle by FLASH_DRV_GetBusyStatus() if occur access error.
* [Fix] - It would hardfault, if callback is not NULL in flash sync mode.
* [Fix] - Data left in RxFIFO cannot be read during i2c transmission timeout
* [Fix] - i2c deinit function does not clear the flag register
* [Fix] - The value of clock out divider can not be the value+1
* [Fix] - loading source and opportunity will be changed when calling eTMR_DRV_SetChnOutMask API
* [Fix] - Remove loading source and opportunity configuration in eTMR_DRV_SetCounterInit API
* [Fix] - It will generate assert error when calling eTMR_DRV_QuadDecodeStop API
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitOutputCompare after calling eTMR_DRV_DeinitOutputCompare API
* [Fix] - Etmr channel n(n>0) interrupt can not be set correctly
* [Fix] - Assert will be triggered when to capture just one channel
* [Fix] - When using the GPIO falling edge interrupt, an interrupt will be mistakenly responded to.
* [Fix] - Dead time can not be configured correctly when configuring multiple channels
* [Fix] - Counter initial and maximal value is not appropriate when configuring output compare or timer mode
* [Fix] - Odd channel io status is always 0 in complementary mode
* [Fix] - ACMP channel output status can not be obtained in continuous mode
* [Fix] - Warning reported when compiling linflexd file
* [Fix] - Modify rtc clock out name from RTC_CLKOUT_SRC_32KHZ to RTC_CLKOUT_SRC_CLKSEL
* [Fix] - Continuous enable will not configured correctly in the ACMP_DRV_Init function
* [Fix] - In i2c slave mode, the restart interrupt event and the address match interrupt event may have an execution sequence error.
* [Fix] - [FLASH] Add nop command for FLASH_LaunchCommandSequence.
* [New] - Remove EMU_DRV_GetErrReportDataBit() API.
* [New] - Add EWDG_DeInit API
* [New] - User can disable ECC or disable debugger by configuring CTRL_OVRD register
* [New] - Add period calculation in capture period event
* [New] - Add Emu_DoubleBit_Demo.
* [New] - Add Wdg_Demo.
* [New] - Use customerized section to place ram function other than __ramfunc

---

## YTM32B1MC0

### 1_3_1_HF001 2024-09-09
* [Opt] - [DMA] Enable DMA channel interrupt separately.
* [Opt] - Add clearing interrupt flag before eTMR enable.
* [Opt] - [YCT_Demo] Remove RamCode demo.
* [Opt] - [YCT_Demo] Add duty cycle update in the MPWM pwm demo.
* [Fix] - [Toolchain] Fix IAR ALIGNED() macro issue.
* [Fix] - [DMA] Fix one dma channel starts may cause another closed channel restarts.
* [Fix] - [FlexCAN] Fix hardfault will occur when payload size exceeds 8 bytes if enable legacy rx fifo.
* [Fix] - [eTMR] Fix counter synchronization can not be disabled.
* [Fix] - [eTMR] Fix CHMASK register synchronization can not be disabled.
* [Fix] - [eTMR] Fix one channel interrupt disablement affects the other channel interrupt enablement since they share the same irq number.
* [Fix] - [eTMR] Fix the return type of eTMR_DRV_CounterRead() api function. However, it can be replaced with eTMR_DRV_GetCntVal() api in the etmr_hw_access.c file.
* [Fix] - [MPWM] Fix MPWM channel combination source selection issue.
* [Fix] - [SDK_GUI] eTMR channel pair number is 1 by default and can not be set to 0.
* [Fix] - [SDK_GUI] eTMR can not select combination channel source.
* [Fix] - [SDK_GUI] There's no quadrature decoder state variable in config.c/h.
* [Fix] - [SDK_GUI] MPWM channel pair number is 1 by default and can not be set to 0.
* [Fix] - [SDK_GUI] ADC external channel name error.
* [New] - [Toolchain] Add Green Hills tool chain support.
* [New] - [eTMR] Add eTMR_DRV_GetInputCaptureValue() api.

### 1_3_1 2024-07-18
* [Opt] - [SDK_GUI] Freertos and secure boot demo are generated through YCT.
* [Opt] - Add FXOSC gain scan when enabling FXOSC or waking up from low power mode.
* [Opt] - Display WDG, EWDG, EMU, pTMR instance index for ipc clock name and irq name.
* [Opt] - Put DMA_DRV_ClearIntStatus api in front of callback in the DMA_DRV_IRQHandler function.
* [Opt] - Remove Timeout enable and disable operation in linflexd uart isr handler, and set to configurable in the linflexd yct demo by user.
* [Opt] - Dummy wait to ensure transfer complete signal is 0 for linflexd module.
* [Opt] - Add ipc module reset api for module reset function.
* [Fix] - RTOS would cause LPACK reset after wakeup.
* [Fix] - The pcs continuous function of spi is only valid for the first transmission.
* [Fix] - I2C timeout flag is set abnormally.
* [Fix] - I2C bus busy condition process in the I2C_DRV_MasterInit api.
* [Fix] - UART Tx/Rx DMA enablement can not be clear.
* [Fix] - ECC error when viewing register value of flexcan in the debug mode.
* [Fix] - DEV_ASSERT error in can pal layer when using enhanced FIFO.
* [Fix] - The input size of flash api check issue.
* [Fix] - MPWM disable counter api can not disable the counter of mpwm.
* [Fix] - The variable type of monitorIrqn in the intm_config_t will cause compile warning when using iar project.
* [Fix] - [SDK_GUI] The sequence of pins which have been configured will change when modifying the feature of pinout.
* [Fix] - [SDK_GUI] There's no value in parameter pullConfig of pinmux.c when just enabling pull enable.
* [Fix] - [SDK_GUI] Interrupt vector table update.
* [Fix] - [SDK_GUI] Data section is not initialzied completely when power on reset.
* [Fix] - [SDK_GUI] Configuration issue of driveSelect in pinout configuration.
* [Fix] - [SDK-GUI] DMA callback function is not declared externally when generating dma configuration.
* [Fix] - [SDK-GUI] FlexCAN payload size label is incorrect.
* [Fix] - [SDK_GUI] eTMR measurement configuration in input capture mode is missing.
* [Fix] - [SDK_GUI] ADC trigger type does not support hardware trigger.
* [New] - [SDK_GUI] Add api highlight for pTMR_DRV_InitChannel api.
* [New] - [SDK_GUI] Add mpwm module configuration ui.
* [New] - [YCT_Demo] Add flash disable debugger and flash ecc demos.
* [New] - [YCT_Demo] Add mpwm pwm demo.
* [New] - Open debug disable bit of CTRL_OVRD register.
* [New] - Add EMU_DRV_GetChannelSBInterruptFlag api.
* [New] - Add EWDG_DRV_DisableInterrupt api.

### 1_3_0 2024-04-19
* [Fix] - [FLEXCAN] Add enhanceRxFifoState.state init when init flexcan.
* [Fix] - I2C won't detect line status when init function calls.
* [Fix] - I2C won't process SDA SCL low timeout event, user may need this callback for bus error process.

### 1_2_1 2024-03-28
* [Opt] - [FEE] Loosen the restriction that pflash cannot use fee
* [Fix] - [FEE] Fix sync mode donot launch command in RAM.
* [Fix] - [DMA] It issues BTCE error, if call DMA_CTSClearReg() when dma request enable.
* [Fix] - YCT Demos YT_LINK issue and the size of generated binary file is overlarge.

### 1_2_0 2024-03-12
* [Opt] - DMA_REQ_Disable change to DMA_REQ_DISABLED
* [Opt] - Uart adds fifo function, one wire mode function and rts cts function
* [Opt] - Add API for customers to modify rx fifo watermark
* [Opt] - In Ewdg_Demo, replace the disabled interrupt with the deinit function.
* [Opt] - Optimize lin demo for MC0
* [Opt] - Add uart continuous transmission demo for MC0
* [Opt] - Optimize YTC uart demo for MC0
* [Opt] - Optimize YTC i2c demo for MC0
* [Opt] - Add a button to toggle led for Gpio_Interrupt_Demo.
* [Opt] - Optimize Etmr_Pwm_Demo
* [Opt] - Optimize Etmr_Pwm_Complementary_Demo
* [Opt] - Optimize Etmr_Input_Capture_Demo
* [Opt] - Add a step to clear injection error for Emu_Demo.
* [Opt] - [YCT_Demo] Add recovery method in CMU demo.
* [Opt] - [YCT_Demo] Add async flash demo.
* [Opt] - [YCT_Demo] Simplify Fee demo
* [Opt] - [YCT_Demo] Add lowest power demo for YTM32B1MC0 and YTM32B1LE0.
* [Opt] - Change the rx fifo watermark of uart to 0
* [Opt] - Optimize ACMP YCT demos
* [Opt] - In Lin mode, the user layer's modification of txbuff will cause a readback error event generated by the driver layer exception.
* [Opt] - [FLASH] Add erase quick for YTM32B1MC0.
* [Fix] - The return value of CRC_GetProtocolWidth function is not correct
* [Fix] - UART compilation generates warnings
* [Fix] - MPWM Set LDOK task error
* [Fix] - MPWM will set PWM to error frequency if frequency too low.
* [Fix] - PWM channel pair set error when configuring odd channel invert in complementary mode
* [Fix] - CR[DSDIS] and CR[DBGDIS] bit logic error in the WDG driver.
* [Fix] - Readme file need to update.
* [Fix] - The value of typeOfUpdate is wrong when using eTMR_DRV_UpdatePwmPeriodAndDuty function
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitPwm after calling eTMR_DRV_DeinitPwm function
* [Fix] - Flash async mode cannot get idle by FLASH_DRV_GetBusyStatus() if occur access error.
* [Fix] - It would hardfault, if callback is not NULL in flash sync mode.
* [Fix] - Data left in RxFIFO cannot be read during i2c transmission timeout
* [Fix] - i2c deinit function does not clear the flag register
* [Fix] - The value of clock out divider can not be the value+1
* [Fix] - loading source and opportunity will be changed when calling eTMR_DRV_SetChnOutMask API
* [Fix] - Remove eTMR_DRV_SetCounterInit API
* [Fix] - eTMR_IPC_CLOCK is wrong, it leads the failure of supporting ipc clock for etmr clock source
* [Fix] - Add measurementPeriod state element for YTM32B1MC0 input capture
* [Fix] - It can't get channel flag for etmr1 in the input capture handler
* [Fix] - Lack of callback function handling
* [Fix] - It will generate assert error when calling eTMR_DRV_QuadDecodeStop API
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitOutputCompare after calling eTMR_DRV_DeinitOutputCompare API
* [Fix] - heap and stack size is a bit large in the YTM32B1MC0 gcc link file
* [Fix] - lpTMR init parameter error in Wdg_Demo，so it starts timing when it initializes.
* [Fix] - When using the GPIO falling edge interrupt, an interrupt will be mistakenly responded to.
* [Fix] - Counter initial and maximal value is not appropriate when configuring output compare or timer mode
* [Fix] - Odd channel io status is always 0 in complementary mode
* [Fix] - ACMP channel output status can not be obtained in continuous mode
* [Fix] - ACMP output flag is not correct in non-continuous mode
* [Fix] - In i2c slave mode, the restart interrupt event and the address match interrupt event may have an execution sequence error.
* [New] - Remove EMU_DRV_GetErrReportDataBit() API.
* [New] - Add EWDG_DeInit API
* [New] - Add Emu_DoubleBit_Demo
* [New] - The hardfault interrupt function declaration (naked) of Emu_DoubleBit_Demo reports an error in keil.
* [New] - Add new API MPWM_DRV_EnableDma() to enable channel dma feature
* [New] - Use customerized section to place ram function other than __ramfunc

---

## YTM32B1LE0

### 1_3_1_HF001 2025-06-09

该版本是YTM32B1LE0 1.3.1版本的第一版修复版本YTM32B1LE0 1.3.1 hotfix 001。以下是SDK各个模块的更新说明。

- CMSIS更新
  - 增加了对Green Hills的支持。增加了cmsis_ccarm.h文件，并在cmsis_compiler.h中加入该文件的包含。
- platform更新
  - 修复了ALIGNED(x)宏在IAR 8.4版本之后不支持的问题。手动移植时移植core_common.h即可。
  - 修复了进低功耗之后systick挂起的中断会立即唤醒MCU的问题。涉及power_manager_YTM32B1Lx.c，在进低功耗之前清除pending状态。
  - PINS更新
    - 将PINS_DRV_WritePin和PINS_DRV_WritePinsAPI的底层实现改为操作PSOR和PCOR寄存器。涉及pins_gpio_hw_access.h文件，修改了PINS_GPIO_WritePin函数的底层实现。
    - 增加了PINS_DRV_ReadPin API。涉及pins_driver.c/.h文件。
  - DMA更新
    - 修复了ram reload功能不会在最后一个cts结构体禁掉的问题。涉及dma_driver.c。
    - 修复了开启一个DMA通道可能会导致另一个通道重新开启的问题。涉及dma_hw_access.c文件中DMA_SetDmaRequestCmd函数的修改。
    - 修复了初始化DMA后自动开启所有通道的中断，而是在通道初始化中开启。涉及dma_driver.c文件中DMA_DRV_Init和DMA_DRV_ChannelInit函数的修改。
  - Flash时钟更新
    - 修复了EFM预分频值，使得EFM时钟等于slow bus时钟。涉及clock_YTM32B1Lx.c文件中CLOCK_SYS_SetScuConfiguration函数里EFM prescaler的配置，以及YTM32B1LE0_features.h文件中FEATURE_EFM_OPERATION_CLOCK_SOURCE宏的定义。
    - 增加了EFM CTRL寄存器中READONLY位的功能，其使用详见论坛，链接如下https://forum.ytmicro.com/topic/1054/efm-readonly-bit%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E
  - FEE更新
    - 修复了FEE在同步模式下使用多个sector作为一个cluster时不能擦除下一个sector的问题。
    - 增加了Fee_Cancel API。
  - FlexCAN更新
    - 修复了读取Legacy FIFO数据时，当对DLC进行错误注入（playload长度大于8）测试时，会发生数组越界的问题。涉及到flexcan_hw_access.c文件中FLEXCAN_ReadRxFifo函数的改动。
    - 修复了当存在模块RAM ECC错误时，FLEXCAN_Enable若被中断打断，可能会造成状态判断卡死。涉及到flexcan_hw_access.c文件中FLEXCAN_Enable函数的改动。
    - 修复了数据扩展帧发送时邮箱SRR位配置出错的问题，由显性改为隐性使其严格遵从协议规范，防止兼容性问题，建议修改。涉及到flexcan_hw_access.c文件中FLEXCAN_SetTxMsgBuff函数的改动。
  - UART更新
    - 修复了当PID等于numOfClassicPID时，UART会发送错误LIN帧数据的问题。
    - 修复了LIN模式在接收同步，发送/接收PID状态下不支持超时处理的问题。
    - 修复了在修改INTE寄存器时被中断重入造成中断中的INTE修改不成功的问题。因此在UART_SetIntMode函数中加入原子操作，保证INTE的配置修改都能成功。
  - SPI更新
    - 修复了在主机下使用DMA单线模式收不到数据的问题。涉及SPI_DRV_MasterStartTransfer和SPI_DRV_SlaveTransfer两个函数的修改。
    - 修复了当FrameSize=0时，TXCFG寄存器会被写入错误值的问题。涉及spi_hw_access.c文件中SPI_SetTxCommandReg函数的修改，以及spi_master_driver.c文件中对于bitcount=0的处理。
  - eTMR更新
    - 修复了通道之间interruptEnable配成true或者false的时候会互相干扰的问题。会把已使能的通道中断向量关掉。涉及到eTMR_DRV_InitInputCapture 和 eTMR_DRV_InitOutputCompare这两个API。
    - 修复了输入捕获信号宽度不准确和callback不只在捕获完成后进的问题。另外为了方便用户获取实时捕获值，增加了eTMR_DRV_GetInputCaptureValue API。涉及到etmr_ic_driver.c/.h。关于其使用参考Etmr_Input_Capture_Demo例程。
    - 修复了通过配置结构体参数不能禁用计数器同步的问题。涉及到etmr_common.h文件中etmr_cnt_sync_sel_t的改动。
    - 修复了通过配置结构体参数不能禁用CHMASK寄存器同步的问题。涉及到etmr_common.h文件中etmr_mask_sync_sel_t的改动。
    - 修复了当配置结构体中加载频率或者输出脉冲频率为0时，写入寄存器值为0xF的问题。当配置为0时，会报错，配置时数值为n，那么就代表每n次加载一次，或者每n个脉冲就输出一个脉冲。涉及到etmr_common.c文件中eTMR_DRV_SetSync和eTMR_DRV_SetOutputTrigger两个函数的改动，以及status.h中增加了错误状态。
    - 优化了相关寄存器配置的操作。涉及到etmr_hw_access.c文件。
    - 在使能eTMR之前增加清除STS寄存器的操作。见YTM32B1LE0 Errata Sheet E503005。涉及到eTMR_DRV_Enable API。
    - 增加了eTMR_DRV_SetGlobalTimeBase 和 eTMR_DRV_GenGlobalSignal 两个API。涉及到etmr_common.c/.h文件的改动。关于其使用见Etmr_Global_Time_Base_Demo例程。
  - ADC更新
    - 修复了当ADC功能时钟远小于总线时钟时，ADC会多次进中断的问题。涉及到ADC中断处理清标志位函数的修改。
  - WDG更新
    - 修复了YTM32B1LE0_features.h文件中CR和TOVR寄存器复位值错误的问题。即FEATURE_WDG_TOVR_RESET_VALUE和FEATURE_WDG_CR_RESET_VALUE两个宏定义的修改。 
- YCT例程更新
  - 在所有例程中增加了时钟初始化状态的判断检查。
  - 修复了Etmr_Pwm_Demo例程中占空比切换到100%的问题。
  - 修复了Etmr_Pwm_Complementary_Demo例程中更新占空比时通道参数错误的问题。
  - 更新了Etmr_Input_Capture_Demo。
  - 修复了Etmr_Input_Capture_Dma_Demo中DMA软件触发的问题。
  - 修复了ADC例程中ADC功能时钟远小于总线时钟，ADC会多次进中断的问题。因此IPC不分频，在ADC内部进行2分频。
  - 修复了WDG例程中延时会在循环中超时的问题。
  - 增加了以下例程。
    - Etmr_Trigger_Adc_Demo 
    - Etmr_Global_Time_Base_Demo
    - Flexcan_BusOff_Demo
    - Flexcan_WakeUp_Demo
    - TempSenser_Demo
- YCT配置工具界面更新
  - 增加了多组管脚配置的功能。
  - 修改了ADC内部通道宏名。


This version is the hotfix 001 version for YTM32B1LE0_1_3_1. The description of each module is as below.

- CMSIS updates
  - [New] - [CMSIS] Add Green Hills support.
- Platform updates
  - [Fix] - [TOOLCHAIN] ALIGN macro does not support IAR builder.
  - [Fix] - [LOWPOWER] Systick pending will wake up MCU immediately after entering low power mode.
  - Pins updates
    - [Fix] - [PINS] The underlying implementation of the PINS_DRV_WritePin and PINS_DRV_WritePins functions expects to use the PSOR and PCOR registers.
    - [New] - [PINS] Add PINS_DRV_ReadPin API.
  - DMA updates
    - [Fix] - [DMA] DMA auto cts reload won't disable reload feature on last cts word.
    - [Fix] - [DMA] One dma channel starts may cause another closed channel restarts.
    - [Fix] - [DMA] When DMA is enabled, interrupts for all channels should not be enabled by default.
  - Flash updates
    - [Fix] - [EFM] Fix the prescaler of EFM to make sure the frequency is equal to slow bus frequency.
    - [Fix] - [FEE] FEE could not erase the next sector when using multi-sector as cluster by sync mode.
    - [New] - [FEE] Add Fee_Cancel API.
    - [New] - [EFM] Add the READONLY bit control logic for flash driver.
  - FlexCAN updates
    - [Fix] - [FlexCAN] Access is out of bounds if dlc more than 8 bytes.
    - [Fix] - [FlexCAN] Flexcan will block in wait RDY, if there is a CAN RAM ECC error and it is interrupted after switching to normal mode.
    - [Fix] - [FlexCAN] The SRR bit in the mailbox of the extended frame was incorrectly set to 0.
  - UART updates
    - [Fix] - [UART] UART may send error LIN frame data when PID is equal to numOfClassicPID.
    - [Fix] - [UART] UART LIN mode does not support timeout under receive sync, send/rec PID state.
    - [Fix] - [UART] UART_SetIntMode may have been modified by mistake on re-entrant.
  - SPI updates
    - [Fix] - [SPI] SPI master cannot receive data using DMA in single-wire mode.
    - [Fix] - [SPI] TXCFG register will be set to incorrect value if frameSize is set to 0.
  - eTMR updates
    - [Fix] - [eTMR] One channel interrupt disablement will affect the other channel interrupt enablement for channel pair.
    - [Fix] - [eTMR] Overflow of captured duty cycle or period occurred randomly.
    - [Fix] - [eTMR] eTMR counter synchronization can not be disabled.
    - [Fix] - [eTMR] CHMASK register synchronization can not be disabled.
    - [Fix] - [eTMR] When the frequency of register load or trigger output is set to 0, the register value will be 0xF.
    - [Opt] - [eTMR] Improve eTMR fault related register configuration.
    - [Opt] - [eTMR] Restore to enable channel irq number before eTMR enable, and add clearing interrupt flag before eTMR enable.
    - [New] - [eTMR] Add eTMR_DRV_SetGlobalTimeBase and eTMR_DRV_GenGlobalSignal APIs. For usage, please refer to the demo named Etmr_Global_Time_Base_Demo.
    - [New] - [eTMR] Add eTMR_DRV_GetInputCaptureValue API.
  - ADC updates
    - [Fix] - [ADC] ADC will enter interrupt multiple times when ADC functional clock is further less than bus clock.
  - WDG updates
    - [Fix] - [WDG] CR and TOVR reset value in the YTM32B1LE0_features.h file is wrong.
- YCT Demos
  - [New] - [DEMO] Add Clock Init status check in demos.
  - [Fix] - [DEMO] Fail to switch to 100% duty cycle through configuring channel VAL0 and VAL1 in the Etmr_Pwm_Demo.
  - [Fix] - [DEMO] In the Etmr_Pwm_Complementary_Demo, the updated channel is wrong in the eTMR0 overflow interrupt handler.
  - [Fix] - [DEMO] Update Etmr_Input_Capture_Demo to complement multiple channels capture.
  - [Fix] - [DEMO] Fix Etmr_Input_Capture_Dma_Demo dma trigger issue.
  - [Fix] - [DEMO] ADC will enter interrupt multiple times when ADC functional clock is further less than bus clock.
  - [Fix] - [DEMO] WDG demo delay will time out in the loop which cause the MCU reset in the first time.
  - [New] - [DEMO] Add Etmr_Trigger_Adc_Demo.
  - [New] - [DEMO] Add Etmr_Global_Time_Base_Demo.
  - [New] - [DEMO] Add Flexcan_BusOff_Demo.
  - [New] - [DEMO] Add Flexcan_WakeUp_Demo.
  - [New] - [DEMO] Add TempSenser_Demo.
- YCT SDK GUI
  - [New] - [SDK_GUI] Add the feature of multiple groups of pins configuration in the Configuration Tool.
  - [Fix] - [SDK_GUI] Update the internal channel macro name.

### 1_3_1 2024-07-18
* [Opt] - [SDK_GUI] Freertos and secure boot demo are generated through YCT.
* [Opt] - Add FXOSC gain scan when enabling FXOSC or waking up from low power mode.
* [Opt] - Display WDG, EMU, pTMR instance index for ipc clock name and irq name.
* [Opt] - Put DMA_DRV_ClearIntStatus() api in front of callback in the DMA_DRV_IRQHandler() function.
* [Opt] - Dummy wait to ensure transfer complete signal is 0 for uart and lin.
* [Opt] - Add ipc module reset api, update clock initialize flow.
* [Opt] - Clear the count of capture when measurement complete.
* [Opt] - [YCT_Demo] Accomplish multiple continuous transfer in the Etmr_Input_Capture_Dma_Demo.
* [Fix] - RTOS would cause LPACK reset after wakeup.
* [Fix] - The pcs continuous function of spi is only valid for the first transmission.
* [Fix] - I2C timeout flag is set abnormally.
* [Fix] - I2C bus busy condition process in the I2C_DRV_MasterInit api.
* [Fix] - UART Tx/Rx DMA enablement can not be clear.
* [Fix] - ECC error when viewing register value of flexcan in the debug mode.
* [Fix] - DEV_ASSERT error in can pal layer when using enhanced FIFO.
* [Fix] - [SDK_GUI] The sequence of pins which have been configured will change when modifying the feature of pinout.
* [Fix] - [SDK_GUI] There's no value in parameter pullConfig of pinmux.c when just enabling pull enable.
* [Fix] - [SDK_GUI] Interrupt vector table update.
* [Fix] - [SDK_GUI] Data section is not initialzied completely when power on reset.
* [Fix] - [SDK_GUI] Configuration issue of driveSelect in pinout configuration.
* [Fix] - [SDK-GUI] DMA callback function is not declared externally when generating dma configuration.
* [Fix] - [SDK-GUI] FlexCAN payload size label is incorrect.
* [Fix] - [SDK_GUI] eTMR measurement configuration in input capture mode is missing.
* [Fix] - [SDK_GUI] eTMR overflow and fault handler name both mismatch.
* [Fix] - [SDK_GUI] ADC trigger type does not support hardware trigger.
* [New] - [SDK_GUI] Add FEE module.
* [New] - [SDK_GUI] Add api highlight for pTMR_DRV_InitChannel api.
* [New] - [YCT_Demo] Add flash disable debugger and flash ecc demos.
* [New] - [YCT_Demo] Add etmr input capture demo without dma.
* [New] - Open debug disable bit of CTRL_OVRD register.
* [New] - Add EMU_DRV_GetChannelSBInterruptFlag api.

### 1_3_0 2024-04-19
* [Fix] - [FLEXCAN] Add enhanceRxFifoState.state init when init flexcan.
* [Fix] - I2C won't detect line status when init function calls.
* [Fix] - I2C won't process SDA SCL low timeout event, user may need this callback for bus error process.

### 1_2_1 2024-03-28
* [Opt] - [FEE] Loosen the restriction that pflash cannot use fee
* [Fix] - [DMA] It issues BTCE error, if call DMA_CTSClearReg() when dma request enable.
* [Fix] - YCT Demos YT_LINK issue and the size of generated binary file is overlarge.

### 1_2_0 2024-03-12
* [Opt] - Optimize lin demo for LE0
* [Opt] - Optimize uart demo for LE0
* [Opt] - Optimize i2c demo for LE0
* [Opt] - [YCT_Demo] Add recovery method in CMU demo.
* [Opt] - [YCT_Demo] Add lowest power demo for YTM32B1MC0 and YTM32B1LE0.
* [Opt] - Change the rx fifo watermark of uart to 0
* [Opt] - Added steps to clear injection errors for Emu_Demo.
* [Opt] - Delete unnecessary comments for Wdg_Demo.
* [Opt] - Add a button to toggle led for Gpio_Interrupt_Demo.
* [Opt] - Optimize ACMP YCT demos
* [Opt] - In Lin mode, the user layer's modification of txbuff will cause a readback error event generated by the driver layer exception.
* [Fix] - UART compilation generates warnings
* [Fix] - PWM channel pair set error when configuring odd channel invert in complementary mode
* [Fix] - CR[DSDIS] and CR[DBGDIS] bit logic error in the WDG driver.
* [Fix] - Readme file need to update.
* [Fix] - The value of typeOfUpdate is wrong when using eTMR_DRV_UpdatePwmPeriodAndDuty function
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitPwm after calling eTMR_DRV_DeinitPwm function
* [Fix] - Flash async mode cannot get idle by FLASH_DRV_GetBusyStatus() if occur access error.
* [Fix] - It would hardfault, if callback is not NULL in flash sync mode.
* [Fix] - Data left in RxFIFO cannot be read during i2c transmission timeout
* [Fix] - i2c deinit function does not clear the flag register
* [Fix] - The value of clock out divider can not be the value+1
* [Fix] - when channel number of different instance is not same, writing register will generate error
* [Fix] - loading source and opportunity will be changed when calling eTMR_DRV_SetChnOutMask API
* [Fix] - Remove eTMR_DRV_SetCounterInit API
* [Fix] - It will generate assert error when calling eTMR_DRV_QuadDecodeStop API
* [Fix] - VAL0 and VAL1 can't sync when using eTMR_DRV_InitOutputCompare after calling eTMR_DRV_DeinitOutputCompare API
* [Fix] - Etmr channel n(n>0) interrupt can not be set correctly
* [Fix] - When using the GPIO falling edge interrupt, an interrupt will be mistakenly responded to.
* [Fix] - Counter initial and maximal value is not appropriate when configuring output compare or timer mode
* [Fix] - Odd channel io status is always 0 in complementary mode
* [Fix] - eTMR instance 2 will be traversed from 0 to 7 for YTM32B1LE0
* [Fix] - ACMP channel output status can not be obtained in continuous mode
* [Fix] - In i2c slave mode, the restart interrupt event and the address match interrupt event may have an execution sequence error.
* [New] - Add API for customers to modify rx fifo watermark
* [New] - Remove EMU_DRV_GetErrReportDataBit() API.
* [New] - Add uart continuous transmission demo for LE0
* [New] - Improve input capture feature for YTM32B1LE0
* [New] - Add Emu_DoubleBit_Demo.
* [New] - FlexCAN: add safety library
* [New] - Modify link and startup file to support SafLib.
* [New] - Use customerized section to place ram function other than __ramfunc
