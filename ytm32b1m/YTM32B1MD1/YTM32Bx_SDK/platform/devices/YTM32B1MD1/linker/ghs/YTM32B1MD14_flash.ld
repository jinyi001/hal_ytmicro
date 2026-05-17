/*
** ###################################################################
**     Processor:           YTM32B1MD1
**     Compiler:            GreenHills ARM Compiler
**
**     Abstract:
**         Linker file for the GreenHills ARM Compiler
**
**     Copyright 2020-2024 Yuntu Microelectronics Co., Ltd.
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     https://www.ytmicro.com
**
** ###################################################################
*/

/* Specify the memory areas */
MEMORY
{
  /* Flash */
  m_interrupts          (RX)  : ORIGIN = 0x00000000, LENGTH = 0x00000400
  m_text                (RX)  : ORIGIN = 0x00000400, LENGTH = 512K - 0x400

  m_custom              (RW)  : ORIGIN = 0x20000000, LENGTH = 0x00000000
  m_data                (RW)  : ORIGIN = 0x1FFF8000, LENGTH = 64K

}

/* Define output sections */
SECTIONS
{
  // Defaults
  HEAP_SIZE  = isdefined(__heap_size__)  ? __heap_size__  : 0x00000200;
  STACK_SIZE = isdefined(__stack_size__) ? __stack_size__ : 0x00000200;

/* If symbol __flash_vector_table__=1 is defined at link time
 * the interrupt vector will not be copied to RAM.
 * Warning: Using the interrupt vector from Flash will not allow
 * INT_SYS_InstallHandler because the section is Read Only.
 */
  M_VECTOR_RAM_SIZE = isdefined(__flash_vector_table__) ? 0x0 : 0x0400;

  /* The startup code goes first into internal flash */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    __interrupts_start__ = .;
    . = ALIGN(16);
    "*(.isr_vector)"         /* Startup code */
    __interrupts_end__ = .;
    . = ALIGN(16);
  } > m_interrupts


  /* The program code and other data goes into internal flash */
  .text :
  {
    . = ALIGN(4);
    "*(.text)"               /* .text sections (code) */
    "*(.text*)"              /* .text* sections (code) */
    "*(.rodata)"             /* .rodata sections (constants, strings, etc.) */
    "*(.rodata*)"            /* .rodata* sections (constants, strings, etc.) */
    "*(.init)"
    "*(.fini)"
    "*(.eh_frame)"
    . = ALIGN(4);
  } > m_text

  /* Section used by the libgcc.a library for fvp4 */
  .ARM :
  {
    __exidx_start = .;
    "*(.ARM.exidx*)"
    __exidx_end = .;
  } > m_text

  __etext = .;    /* Define a global symbol at end of code. */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization. */

  .interrupts_ram :
  {
    . = ALIGN(4);
    __VECTOR_RAM__ = .;
    __RAM_START = .;
    __interrupts_ram_start__ = .; /* Create a global symbol at data start. */
    *(.m_interrupts_ram)          /* This is a user defined section. */
    . += M_VECTOR_RAM_SIZE;
    . = ALIGN(4);
    __interrupts_ram_end__ = .;   /* Define a global symbol at data end. */
  } > m_data

  __VECTOR_RAM = isdefined(__flash_vector_table__) ? addr(m_interrupts) : __VECTOR_RAM__ ;
  __RAM_VECTOR_TABLE_SIZE = isdefined(__flash_vector_table__) ? 0x0 : (__interrupts_ram_end__ - __interrupts_ram_start__);

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* Create a global symbol at data start. */
    "*(.data)"               /* .data sections */
    "*(.data*)"              /* .data* sections */
    "*(.syscall)"
    "*(.interfunc)"
    "*(.secinfo)"
    . = ALIGN(16);
    __data_end__ = .;        /* Define a global symbol at data end. */
  } > m_data

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__);
  __CODE_ROM = __DATA_END; /* Symbol is used by code initialization. */

  .code : AT(__CODE_ROM)
  {
    . = ALIGN(4);
    __CODE_RAM = .;
    __code_start__ = .;      /* Create a global symbol at code start. */
    __code_ram_start__ = .;
    "*(.code_ram)"           /* Custom section for storing code in RAM */
    . = ALIGN(16);
    __code_end__ = .;        /* Define a global symbol at code end. */
    __code_ram_end__ = .;
  } > m_data

  __CODE_END = __CODE_ROM + (__code_end__ - __code_start__);
  __CUSTOM_ROM = __CODE_END;

  /* Custom Section Block that can be used to place data at absolute address. */
  /* Use __attribute__((section (".customSection"))) to place data here. */
  /* Use this section only when MTB (Micro Trace Buffer) is not used, because MTB uses the same RAM area, as described in S32K Reference Manual. */
  .customSectionBlock : AT(__CUSTOM_ROM)
  {
    __customSectionStart = .;
    __customSection_start__ = .;
    "*(.customSection)"      /* Keep section even if not referenced. */
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
    PROVIDE (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE (__init_array_start = .);
    KEEP(*(SORT(".init_array.*")))
    KEEP(*(.init_array))
    PROVIDE (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE (__fini_array_start = .);
    KEEP(*(SORT(".fini_array.*")))
    KEEP(*(.fini_array))
    PROVIDE(__fini_array_end = .);
    . = ALIGN(4);
    __array_ram_end__ = .;        /* Define a global symbol at array end. */
  }> m_data

  __ARRAY_END = __ARRAY_ROM + (__array_ram_end__ - __array_ram_start__);
  __rom_end   = __ARRAY_END;

  /* Uninitialized data section. */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section. */
    . = ALIGN(4);
    __BSS_START = .;
    __bss_start__ = .;
    "*(.bss)"
    "*(.bss*)"
    "*(COMMON)"
    . = ALIGN(4);
    __bss_end__ = .;
    __BSS_END = .;
  } > m_data

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
  } > m_data

  /* Initializes stack on the end of block */
  __StackTop   = addr(m_data) + sizeof(m_data);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);
  __RAM_END = __StackTop;

  .stack __StackLimit :
  {
    . = ALIGN(8);
    __stack_start__ = .;
    . += STACK_SIZE;
    __stack_end__ = .;
  } > m_data

  .ARM.attributes :
  {
    *(.ARM.attributes)
  }

}


