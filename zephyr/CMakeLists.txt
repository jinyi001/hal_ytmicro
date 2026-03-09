zephyr_library()

if(CONFIG_SOC_FAMILY_YTMICRO_YTM32)

set(YTM32_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ytm32b1m/YTM32B1MC0/YTM32Bx_SDK)

zephyr_include_directories(
  ${YTM32_SDK_DIR}/platform/devices
  ${YTM32_SDK_DIR}/platform/devices/common
  ${YTM32_SDK_DIR}/platform/devices/YTM32B1MC0/include
  ${YTM32_SDK_DIR}/platform/devices/YTM32B1MC0/startup
  ${YTM32_SDK_DIR}/platform/drivers/inc
  ${YTM32_SDK_DIR}/rtos/osif
)

# Some driver headers are in module-specific subdirectories inside inc/ and src/
file(GLOB INC_SUBDIRS ${YTM32_SDK_DIR}/platform/drivers/inc/*)
foreach(dir ${INC_SUBDIRS})
  if(IS_DIRECTORY ${dir})
    zephyr_include_directories(${dir})
  endif()
endforeach()

file(GLOB SRC_SUBDIRS ${YTM32_SDK_DIR}/platform/drivers/src/*)
foreach(dir ${SRC_SUBDIRS})
  if(IS_DIRECTORY ${dir})
    zephyr_include_directories(${dir})
  endif()
endforeach()

zephyr_library_sources(
  ${YTM32_SDK_DIR}/platform/devices/startup.c
  ${YTM32_SDK_DIR}/platform/devices/YTM32B1MC0/startup/system_YTM32B1MC0.c
)

# Glob all driver sources so they can be called by Zephyr drivers
file(GLOB_RECURSE YTM32_DRIVER_SOURCES ${YTM32_SDK_DIR}/platform/drivers/src/*.c)
# Exclude ee_emulation since it requires application-provided ee_user_config.h
list(FILTER YTM32_DRIVER_SOURCES EXCLUDE REGEX ".*/ee_emulation/.*")
# Exclude fee since it requires application-provided fee_config.h
list(FILTER YTM32_DRIVER_SOURCES EXCLUDE REGEX ".*/fee/.*")
# Exclude sent since it contains missing definitions (SDK bug)
list(FILTER YTM32_DRIVER_SOURCES EXCLUDE REGEX ".*/sent/.*")
# Exclude rtc since it contains missing definitions for this MCU variant
list(FILTER YTM32_DRIVER_SOURCES EXCLUDE REGEX ".*/rtc/.*")
# Exclude interrupt_manager.c since Zephyr provides native interrupt management
list(FILTER YTM32_DRIVER_SOURCES EXCLUDE REGEX ".*/interrupt_manager\\.c$")
zephyr_library_sources(${YTM32_DRIVER_SOURCES})

# Include baremetal OSIF by default if needed by the HAL (Zephyr apps can override or provide osif_zephyr.c later)
zephyr_library_sources(
  ${YTM32_SDK_DIR}/rtos/osif/osif_baremetal.c
)

endif()
