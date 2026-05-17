# Flexcan Enhancefifo 示例代码

## 项目名称: Flexcan_Enhancefifo_Demo

---

## 功能描述

该示例演示了 FlexCAN Enhance Fifo 的使用，以及自测试。

- 短接板子上的 CAN0_H 与 CAN1_H，CAN0_L 与 CAN1_L 来完成自测试
- 波特率为 仲裁域500Kbps，数据域2Mbps
- CAN0 配置如下
  - 用邮箱-1 交替发送 ID 为 0x4FE 的标准帧，以及 ID 为 0x0CF003FE 的扩展帧
  - 用 Enhance FIFO 接收 ID 为 0x200 的标准帧，以及 ID 为 0x180F1000 的扩展帧，详见例程
- CAN1 配置如下
  - 用邮箱-1 交替发送 ID 为 0x200 的标准帧，以及 ID 为 0x180F1000 的扩展帧
  - 用邮箱-1 接收 ID 为 0x4FE 的标准帧
  - 用邮箱-2 接收 ID 为 0x0CF003FE 的扩展帧  
- CAN0 与 CAN1 均会打印当前收到的报文内容
- 当 CAN1 发送完 11 条报文后，结束自测试
---

## 硬件需求
+ **开发板**: YTM32B1MD1-EVB-Q100
+ **调试器**: J-LINK v9.2 或更高版本，需要安装 YUNTU YTM32B1MD1 补丁（支持包）
+ **连接**: 
  - J11: 连接 PIN1 到 PIN2(CAN0_STB)
  - J11: 连接 PIN3 到 PIN4(CAN0_TXD)
  - J11: 连接 PIN5 到 PIN6(CAN0_RXD)
  - J6: 连接 PIN1 到 PIN2(CAN1_STB)
  - J6: 连接 PIN3 到 PIN4(CAN1_TXD)
  - J6: 连接 PIN5 到 PIN6(CAN1_RXD)
---
A
## 软件需求

+ **配置与代码生成工具**
  + **SDK**:       云途配置工具(YCT) v2.6.3 或更高版本
+ **编译器工具链**
  + **GCC***     : GNU GCC v10.3 或更高版本
  + **Keil/MDK***: ARM Keil/MDK v5.36 或更高版本
  + **IAR***     : IAR v8.4 或更高版本
  + **VS Code**  : Microsoft Visual Studio Code IDE v1.76 或更高版本（C/C++ 支持，GNU 工具链，CMake v3.25，ninja，Cortex-Debug 扩展安装）
+ **调试器驱动**
    + **J-LINK***              : SEGGER JLINK v6.96 或更高版本，需要安装 YUNTU YTM32B1MD1 补丁（支持包）
    + **Ozone 调试器工具**  : SEGGER Ozone v3.28 或更高版本

    ***注意***: * 需要安装 **YTM32B1MD1** 补丁，可通过 **YCT** 获取或联系 ***云途销售/FAE团队***。
---

## 设置步骤

+ ***步骤1***: 使用 YCT 工具打开 project.yct，根据您的需求配置工具链（默认使用 VS code + GCC）
+ ***步骤2***: 生成代码，如果需要可以修改配置（以适应您自己的硬件设计），如果修改，请确保所有修改正确无误后再生成代码
+ ***步骤3***: 使用您选择的 VS code/IAR/Keil IDE 打开生成的项目
+ ***步骤4***: 编译项目，给开发板上电，并通过可用的调试器（如 J-LINK 调试器探头）下载到开发板
+ ***步骤5***: 最后，享受演示示例
---

1. 配置 PTE_12 引脚的复用功能为 UART2_TX（发送引脚），PTD_17 引脚的复用功能为 UART2_RX（接收引脚）
2. 启用并配置 UART2_CLK、FlexCAN0_CLK 和 FlexCAN1_CLK 时钟
3. 启用并配置 Uart 和 Utility Print 模块
4. 启用并配置 Flexcan 增强 FIFO 模式
5. 短接 CAN0 和 CAN1 的 CANH&CANL 线路
---