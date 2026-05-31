zephyr_library()

if(CONFIG_SOC_FAMILY_YTMICRO_YTM32)

if(CONFIG_SOC_YTM32B1MC0)
  set(YTM32_CHIP YTM32B1MC0)
elseif(CONFIG_SOC_YTM32B1MD1)
  set(YTM32_CHIP YTM32B1MD1)
else()
  message(FATAL_ERROR "Unsupported YTM32B1M SoC")
endif()

set(YTM32_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ytm32b1m/${YTM32_CHIP}/YTM32Bx_SDK)

zephyr_include_directories(
  ${YTM32_SDK_DIR}/platform/devices
  ${YTM32_SDK_DIR}/platform/devices/common
  ${YTM32_SDK_DIR}/platform/devices/${YTM32_CHIP}/include
  ${YTM32_SDK_DIR}/platform/devices/${YTM32_CHIP}/startup
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
  ${YTM32_SDK_DIR}/platform/devices/${YTM32_CHIP}/startup/system_${YTM32_CHIP}.c
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

# Use Zephyr-native OSIF shim instead of vendor baremetal OSIF.
# osif_baremetal.c reconfigures SysTick on every OSIF_SemaWait / OSIF_TimeDelay
# call, which conflicts with CONFIG_CORTEX_M_SYSTICK owned by Zephyr.
zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/osif_zephyr.c)

endif()
