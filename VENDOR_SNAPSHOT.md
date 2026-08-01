# Vendor snapshot provenance

## YTM32B1MD1 SDK 1.4.1 HF001

- Scope: `ytm32b1m/YTM32B1MD1` only. The YTM32B1MC0 snapshot is unchanged.
- Upstream base: YTMicro YTM32B1MD1 SDK `1.4.1`.
- Hotfix: `HF001`; the capture report identifies the hotfix date as 2026-05-28.
- Source: installed by the official YT Config Tool from the YTMicro account service, then captured as byte-for-byte before/after files. The temporary raw `YTM32B1MD1_1_4_1_HF001.zip` was removed by YCT and was not retained.
- Repository base containing the original vendor files: `b5727cf7b28101dc15496cc517639458272d8bcb`.
- Import authorization and reason: import the user-supplied official 1.4.1 hotfix into the vendor HAL layer instead of carrying those fixes in the Zephyr adaptation layer.

### Integrity records

- Capture archive `YCT-hotfix-analysis-20260801-212500-v2.zip` SHA-256: `5403c1cb5db6f1f86f231e32f3c87e25859f3bbb5698711f426d7fe3ff913415`.
- Retained official marker `ytm32b1m/YTM32B1MD1/sdk_hf001.patch` SHA-256: `72adbe74d007f13e3d7720f266beb62f1b0b4ee86ccb1ae39e215c6eedb1e121`.
- Captured complete SDK unified diff SHA-256: `8a13fded9b71c7a217711870940e3369e536682dbf226bd2c38e32471f576fa1`.
- Before manifest SHA-256: `39e4d7d2887c97a2ae31c8f6c3088b6ef1a9bfbb6e0decafd465e4e0889a0cef`.
- After manifest SHA-256: `d862e878943dc7d5429e8e89fecdfd217aac2d4ad2c7a72ab840c6eedb71275c`.
- Change manifest SHA-256: `9b1893d378c862bd924dad56280f73a2242e2cb4c7e8324cf095020691e3b998`.

Before import, all 255 changed-or-deleted files matched the captured pre-hotfix files byte for byte. All 1,302 SDK files present in this repository also matched the corresponding pre-hotfix manifest entries. The 15 absent manifest entries are the intentionally omitted nested `sdk_cfg/yt_link/.git` file and `utility_print` component.

### Imported delta

The official content delta contains 253 changed files, one added marker file, and two deleted Doxygen files. Git history is the authoritative exact repository diff. Confirmed SPI behavior changes include direct W1C status writes and corrected half-duplex slave-DMA channel/request handling; HF001 also contains broader vendor SDK and configuration changes.

### Reapplying or upgrading

Start from the clean base revision above, obtain the YTM32B1MD1 1.4.1 HF001 package through YCT, verify the retained marker hash, and verify every affected pre-image before replacing files. Keep the repository's existing exclusions (`utility_print` and the nested `.git` file), apply the hotfix only to the MD1 snapshot, and compare the result against the after manifest. For a later full SDK release, replace the snapshot from that release and re-evaluate whether HF001 is already included rather than replaying it blindly.
