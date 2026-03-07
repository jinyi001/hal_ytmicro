{{ytCopyright "vector.S"}}

    .section {{this.data.vector_flash_region}}, "a"
    .align 2
    .thumb
    .globl  IVT_Table
IVT_Table:
    .long   {{this.data.stack_top}}                                      /* Top of Stack */
    .long   Reset_Handler                                                /* Reset Handler */
{{#each this.data.vectors}}
{{#startsWith 'Reserved' this}}
    .long   DefaultISR
{{else}}
    .long   {{this}}
{{/startsWith}}
{{/each}}
