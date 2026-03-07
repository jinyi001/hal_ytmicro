{{ytCopyright "vector_table_copy.c"}}
#include <stdint.h>
#include "device_registers.h"

static volatile uint32_t * const s_vectors[NUMBER_OF_CORES] = FEATURE_INTERRUPT_INT_VECTORS;
{{#if (or (eq (getProjectInfo 'toolchain') 'CMakeKEIL') (eq (getProjectInfo 'toolchain') 'KEIL'))}}
extern uint32_t Image$${{this.data.vector_ram}}$$Base;
extern uint32_t Image$${{this.data.vector_flash}}$$Base;
extern uint32_t Image$${{this.data.vector_flash_end}}$$Limit;

{{else}}
extern uint32_t {{this.data.vector_ram}}[];
extern uint32_t {{this.data.vector_flash}}[];
extern uint32_t {{this.data.vector_flash_end}}[];
{{/if}}


void VectorTableCopy(){
{{#if (or (eq (getProjectInfo 'toolchain') 'CMakeKEIL') (eq (getProjectInfo 'toolchain') 'KEIL'))}}
    uint8_t coreId = (uint8_t)GET_CORE_ID();
    uint32_t* ramStart = (uint32_t*)&Image$${{this.data.vector_ram}}$$Base;
    const uint32_t* romStart = (const uint32_t*)&Image$${{this.data.vector_flash}}$$Base;
    uint32_t size=((uint32_t)&Image$${{this.data.vector_flash_end}}$$Limit-(uint32_t)&Image$${{this.data.vector_flash}}$$Base)/sizeof(uint32_t);
    *s_vectors[coreId] = (uint32_t)&Image$${{this.data.vector_ram}}$$Base;
{{else}}
    uint8_t coreId = (uint8_t)GET_CORE_ID();
    uint32_t* ramStart = (uint32_t*){{this.data.vector_ram}};
    const uint32_t* romStart = (const uint32_t*){{this.data.vector_flash}};
    uint32_t size=((uint32_t){{this.data.vector_flash_end}}-(uint32_t){{this.data.vector_flash}})/sizeof(uint32_t);
    *s_vectors[coreId] = (uint32_t){{this.data.vector_ram}};
{{/if}}

    for (uint32_t n = 0; n < size; n++)
    {
        ramStart[n] = romStart[n];
    }
}


void DefaultISR(void)
{
   while(1){};
}

{{#each this.data.vectors}}
{{#startsWith 'Reserved' this}}
{{else}}
__attribute__ ((weak)) void {{this}}(void) {
    DefaultISR();
}
{{/startsWith}}
{{/each}}
