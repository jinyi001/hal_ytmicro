/*
** ###################################################################
**     Processor:           YTM32B1MD1 with 64 KB SRAM
**     Compiler:            GNU C Compiler
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     https://www.ytmicro.com
**
** ###################################################################
*/

/* Entry Point */
ENTRY(Reset_Handler)

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 4K;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 4K;

/* If symbol __flash_vector_table__=1 is defined at link time
 * the interrupt vector will not be copied to RAM.
 * Warning: Using the interrupt vector from Flash will not allow
 * INT_SYS_InstallHandler because the section is Read Only.
 */
M_VECTOR_RAM_SIZE = DEFINED(__flash_vector_table__) ? 0x0 : 1K;

/* Specify the memory areas */
MEMORY
{
  /* Flash */
  m_text                (RX)  : ORIGIN = 0x00000000, LENGTH = 512K

  /* SRAM_L */
  m_data_l              (RW)  : ORIGIN = 0x1FFF8000, LENGTH = 16k
  /* CUSTOM */
  m_custom              (RW)  : ORIGIN = 0x1FFFC000, LENGTH = 0K
  /* RETENTION RAM */
  m_retention           (RW)  : ORIGIN = 0x1FFFC000, LENGTH = 32K
  /* SRAM_U */
  m_data_u              (RW)  : ORIGIN = 0x20004000, LENGTH = 16k
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into internal flash */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    __interrupts_start__ = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector))     /* Startup code */
    __interrupts_end__ = .;
    . = ALIGN(4);
  } > m_text


  /* The program code and other data goes into internal flash */
  .text :
  {
    . = ALIGN(4);
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.rodata)               /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.init)                 /* section used in crti.o files */
    *(.fini)                 /* section used in crti.o files */
    *(.eh_frame)             /* section used in crtbegin.o files */
    . = ALIGN(4);
  } > m_text

  /* Section used by the libgcc.a library for fvp4 */
  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_text

  __etext = .;    /* Define a global symbol at end of code. */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization. */

  .interrupts_ram :
  {
    . = ALIGN(4);
    __VECTOR_RAM__ = .;
    __RAM_START = .;
    __RAM_START_L = .;
    __interrupts_ram_start__ = .; /* Create a global symbol at data start. */
    *(.m_interrupts_ram)          /* This is a user defined section. */
    . += M_VECTOR_RAM_SIZE;
    . = ALIGN(4);
    __interrupts_ram_end__ = .;   /* Define a global symbol at data end. */
  } > m_data_l

  __VECTOR_RAM = DEFINED(__flash_vector_table__) ? ORIGIN(m_interrupts) : __VECTOR_RAM__ ;
  __RAM_VECTOR_TABLE_SIZE = DEFINED(__flash_vector_table__) ? 0x0 : (__interrupts_ram_end__ - __interrupts_ram_start__) ;

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* Create a global symbol at data start. */
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    . = ALIGN(4);
    __data_end__ = .;        /* Define a global symbol at data end. */
  } > m_data_l

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__);
  __CODE_ROM = __DATA_END; /* Symbol is used by code initialization. */

  .code : AT(__CODE_ROM)
  {
    . = ALIGN(4);
    __CODE_RAM = .;
    __code_start__ = .;      /* Create a global symbol at code start. */
    __code_ram_start__ = .;
    *(.code_ram)             /* Custom section for storing code in RAM */
    . = ALIGN(4);
    __code_end__ = .;        /* Define a global symbol at code end. */
    __code_ram_end__ = .;
    __RAM_END_L = .;
  } > m_data_l

  __CODE_END = __CODE_ROM + (__code_end__ - __code_start__);
  __CUSTOM_ROM = __CODE_END;

  /* Custom Section Block that can be used to place data at absolute address. */
  /* Use __attribute__((section (".customSection"))) to place data here. */
  .customSectionBlock  ORIGIN(m_custom) : AT(__CUSTOM_ROM)
  {
    __customSectionStart = .;
    __customSection_start__ = .;
    KEEP(*(.customSection))  /* Keep section even if not referenced. */
    __customSection_end__ = .;
  } > m_custom
  __CUSTOM_END = __CUSTOM_ROM + (__customSection_end__ - __customSection_start__);

  __ARRAY_ROM = __CUSTOM_END;
  .array : AT(__ARRAY_ROM)
  {
    . = ALIGN(4);
    __ARRAY_RAM = .;
    __array_ram_start__ = .;      /* Create a global symbol at array start. */
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
    __array_ram_end__ = .;        /* Define a global symbol at array end. */
  }> m_data_l

  __ARRAY_END = __ARRAY_ROM + (__array_ram_end__ - __array_ram_start__);
  __rom_end   = __RETENTION_END;

  __RETENTION_RAM = __ARRAY_END;

  /* Retention SRAM Section Block that can be used to place data at absolute address. */
  /* Use __attribute__((section (".retentionSection"))) to place data here. */
  .retentionSectionBlock : AT(__RETENTION_RAM)
  {
    . = ALIGN(4);
    __retentionSectionStart = .;
    __retentionSection_start__ = .;
    *(.retention_ram)             /* Custom section for storing code in RAM */
    KEEP(*(.retentionSection))  /* Keep section even if not referenced. */
    . = ALIGN(4);
    __retentionSection_end__ = .;
  } > m_retention
  __RETENTION_END = __RETENTION_RAM + (__retentionSection_end__ - __retentionSection_start__);

  /* Uninitialized data section. */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section. */
    . = ALIGN(4);
    __RAM_START_U = .;
    __BSS_START = .;
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __BSS_END = .;
  } > m_data_u

   /* Put heap section after the program data */
  .heap :
  {
    . = ALIGN(8);
    __end__ = .;
    __heap_start__ = .;
    PROVIDE(end = .);
    PROVIDE(_end = .);
    PROVIDE(__end = .);
    __HeapBase = .;
    . += HEAP_SIZE;
    __HeapLimit = .;
    __heap_limit = .;
    __heap_end__ = .;
  } > m_data_u

  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data_u) + LENGTH(m_data_u);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);
  __RAM_END = __StackTop;

  .stack __StackLimit :
  {
    . = ALIGN(8);
    __stack_start__ = .;
    . += STACK_SIZE;
    __stack_end__ = .;
    __RAM_END_U = .;
  } > m_data_u

  .ARM.attributes 0 : { *(.ARM.attributes) }
  
  /* Memory validation */
  ASSERT(__rom_end <= (ORIGIN(m_text) + LENGTH(m_text)), "Region m_text overflowed!")

  ASSERT(__StackLimit >= __HeapLimit, "region m_data overflowed with stack and heap")
}

