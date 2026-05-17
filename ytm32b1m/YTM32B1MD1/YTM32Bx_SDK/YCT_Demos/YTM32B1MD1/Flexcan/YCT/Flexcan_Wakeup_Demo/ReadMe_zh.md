# Flexcan_Wakeup_Demo

---

## 功能描述

本项目为FlexCAN唤醒芯片示例：

- Flexcan_Wakeup_Demo 支持模式：
    - 待机模式（Standby mode）
    - 掉电模式（Powerdown mode）
    - 活动模式（Active mode）
- 配置PTE_12引脚复用为LINFlexD2_TX（发送），PTD_17引脚复用为LINFlexD2_RX（接收）
- 使能并配置LINFlexD2_CLK时钟
- 使能并配置LINFlexD串口及打印工具模块
- 配置PTB_4复用控制LED5，PTB_5控制LED6，PTE_8控制LED7，PTD_12控制CAN0_STB
- 使能并配置GPIO时钟
- 配置PTE_5复用为CAN0_TX，PTE_4复用为CAN0_RX
- 使能并配置FlexCAN0_CLK时钟
- 使能并配置CAN模块
- 使能并配置WKU_CLK时钟
- 使能并配置WKUP模块
- 使能并配置POWER模块
- 使用TX_MAILBOX定义的邮箱编号（邮箱-1）以轮询方式每秒发送ID为TX_MSG_ID(0x003)的标准帧，发送成功时LED5切换状态
- 使用RX_STD_MAILBOX定义的邮箱编号（邮箱-0）通过CAN0事件中断（FLEXCAN_EVENT_RX_COMPLETE）接收ID为RX_STD_MSG_ID(0x002)的报文，接收成功时LED6切换状态
- 使用RX_EXT_MAILBOX定义的邮箱编号（邮箱-2）通过CAN0事件中断接收ID为RX_EXTMSG_ID(0x70012)的报文，接收成功时LED7切换状态
- 用户可通过标准CAN报文选择模式：data[0] == 0x01进入待机模式，data[0] == 0x02进入掉电模式，其余数据值不影响
- 选择掉电模式时，CAN0_RX(PTE4)将切换为GPIO输入模式作为唤醒源
- 任意帧均可将芯片唤醒至活动模式：
    - 待机模式唤醒后代码从待机语句处继续运行
    - 掉电模式唤醒后系统将复位
  
---

## 硬件需求

+ **开发板**: YTM32B1MD1-EVB-Q100
+ **调试器**: J-LINK v9.2 或更高版本

---

## 软件需求

+ **SDK**     : YTM32B1MD1_1_4_0 或更高版本
+ **GCC**     : GNU GCC v10.3 或更高版本
+ **Keil/MDK**: ARM Keil/MDK v5.36 或更高版本
+ **IAR**     : IAR v8.40 或更高版本
+ **VS Code** : Microsoft Visual Studio Code IDE v1.76 或更高版本（GNU GCC 工具链，CMake v3.25，ninja，Cortex-Debug 扩展安装）
+ **J-LINK**  : SEGGER JLINK v6.96 或更高版本，需要安装 YTM32B1MD1 补丁（支持包）
+ **Ozone**   : SEGGER Ozone v3.28 或更高版本
  
---