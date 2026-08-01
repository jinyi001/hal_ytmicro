# YTMicro HAL snapshot

> **Experimental project**
>
> The YTM32 Zephyr integration around this repository is experimental, is not officially supported by YTMicro/云途, and is still under development.

This repository contains an unpacked snapshot of the original vendor-provided YTMicro HAL SDK. It is kept as a source snapshot for the YTM32 Zephyr integration.

The exact upstream package and imported hotfixes are recorded in [VENDOR_SNAPSHOT.md](VENDOR_SNAPSHOT.md).

## Do not modify the HAL snapshot

Do not modify, refactor, or add application code to the files under `ytm32b1m/`. Treat this repository as read-only vendor input.

When a new vendor HAL release is required, replace the snapshot with the corresponding original vendor package and record the new upstream version or source package in the commit message. Integration fixes belong in [`zephyr-ytm32-module`](https://github.com/jinyi001/zephyr-ytm32-module), not in this repository.

## Start a complete workspace

For a reproducible YTM32 Zephyr workspace, start with [`ytm32-zephyr-starter`](https://github.com/jinyi001/ytm32-zephyr-starter). The starter manifest fetches this HAL snapshot together with Zephyr and `zephyr-ytm32-module` at pinned revisions.
