
{{ytCopyright "vector.S"}}
    

    SECTION {{this.data.vector_flash_region}}:CODE:ROOT(2)
    EXTERN DefaultISR
    EXTERN {{this.data.stack_top}}   /* Top of Stack for Initial Stack Pointer */
    EXTERN Reset_Handler             /* Reset Handler */
{{#each this.data.vectors}}
{{#startsWith 'Reserved' this}}
{{else}}
    EXTERN   {{this}}
{{/startsWith}}
{{/each}}

    DATA
__vector_table
    DCD {{this.data.stack_top}}   /* Top of Stack for Initial Stack Pointer */
    DCD Reset_Handler             /* Reset Handler */
{{#each this.data.vectors}}
{{#startsWith 'Reserved' this}}
    DCD DefaultISR
{{else}}
    DCD {{this}}
{{/startsWith}}
{{/each}}
    END
