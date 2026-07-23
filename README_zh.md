# YTMicro HAL 快照

> **实验性项目声明**
>
> 围绕本仓库构建的 YTM32 Zephyr 集成是实验性的，非云途官方支持项目，目前仍在持续开发中。

本仓库仅包含原厂提供的 YTMicro HAL SDK 解压快照，作为 YTM32 Zephyr 集成使用的原始输入。

## 不要修改 HAL 快照

不要修改、重构 `ytm32b1m/` 目录下的文件，也不要在其中加入应用代码。请将本仓库视为只读的原厂 HAL 输入。

需要更新原厂 HAL 时，应使用对应的原厂软件包整体替换当前快照，并在 commit message 中记录新的原厂版本或源软件包信息。移植和集成修复应放在 [`zephyr-ytm32-module`](https://github.com/jinyi001/zephyr-ytm32-module)，不要修改本仓库的 HAL 代码。

## 创建完整工作区

需要可复现的 YTM32 Zephyr 工作区时，请从 [`ytm32-zephyr-starter`](https://github.com/jinyi001/ytm32-zephyr-starter) 开始。starter manifest 会以固定 revision 同时获取本 HAL 快照、Zephyr 和 `zephyr-ytm32-module`。
