{{ytCopyright "vector.S"}}
    PRESERVE8
    THUMB
    AREA    |{{this.data.vector_flash_region}}|, CODE, READONLY
    IMPORT   Reset_Handler             /* Reset Handler */
    
    IMPORT   DefaultISR
    IMPORT  |Image$${{this.data.stack_top}}$$Limit|
{{#each this.data.vectors}}
{{#startsWith 'Reserved' this}}
{{else}}
    IMPORT   {{this}}
{{/startsWith}}
{{/each}}
    DCD |Image$${{this.data.stack_top}}$$Limit|   /* Top of Stack for Initial Stack Pointer */
    DCD Reset_Handler             /* Reset Handler */
{{#each this.data.vectors}}
{{#startsWith 'Reserved' this}}
    DCD DefaultISR
{{else}}
    DCD {{this}}
{{/startsWith}}
{{/each}}
    END