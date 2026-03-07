{{ytCopyright "yt_linker.ld"}}

{{setVar2 'level1' (createArray)~}}
{{#each this.data~}}
{{#if this.children~}}
{{#each (sortBy this.children (createArray 'order' 'name'))~}}
{{#if this.isFlash}}
{{push2 @root.level1 this~}}
{{/if}}
{{/each~}}
{{/if~}}
{{/each~}}
{{#each this.data~}}
{{#if this.children~}}
{{#each (sortBy this.children (createArray 'order' 'name'))~}}
{{#if (not this.isFlash)}}
{{push2 @root.level1 this~}}
{{/if}}
{{/each~}}
{{/if~}}
{{/each~}}
{{setVar2 'sections' (createObject)~}}
{{#each @root.level1~}}
{{setVar2 'parent' this}}
{{#if this.children~}}
{{#each this.children~}}
{{#if this.children~}}
{{#each this.children~}}
{{set2 this 'parent' @root.parent~}}
{{set2 @root.sections this.id this~}}
{{/each~}}
{{/if}}
{{/each~}}
{{/if}}
{{/each~}}
{{setVar2 'sectionList' (createArray)~}}
{{#each @root.level1~}}
{{#if this.children~}}
{{#each (sortBy this.children (createArray 'order' 'name'))~}}
{{#if this.children~}}
{{#each (sortBy this.children (createArray 'order' 'name'))~}}
{{#if this.copyFrom~}}
{{set2 (get this.copyFrom @root.sections) 'needCopy' true}}
{{set2 (get this.copyFrom @root.sections) 'name' this.name}}
{{push2 @root.sectionList this.id~}}
{{else}}
{{push2 @root.sectionList this.id~}}
{{/if~}}
{{/each~}}
{{/if~}}
{{/each~}}
{{/if~}}
{{/each~}}

/* MEMORY MAP */
MEMORY
{
{{#each @root.level1}}
    {{this.name}} : ORIGIN = {{this.startAddress}}, LENGTH = {{this.size}}
{{/each}}
}
{{setVar2 'memoryUsed' (createObject)~}}
/* SECTIONS */
SECTIONS
{
{{#each @root.sectionList}}
{{setVar2 'i' (get this @root.sections)~}}
{{#if @root.i.needCopy}}
    {{@root.i.name}}_rom {{#if @root.i.align}}ALIGN({{@root.i.align}}){{/if}} ROM_NOCOPY({{@root.i.name}}) :> {{@root.i.parent.name}}
    {{@root.i.name}}_rom_start = ADDR({{@root.i.name}}_rom);
    {{@root.i.name}}_rom_end = ENDADDR({{@root.i.name}}_rom);
{{else}}
    {{@root.i.name}} {{#if @root.i.align}}ALIGN({{@root.i.align}}){{/if}} {{#isTruthy @root.i.flags~}}({{@root.i.flags}}){{/isTruthy~}}: 
    {
        {{set2 @root.memoryUsed @root.i.parent.name true}}
{{#if @root.i.align}}
        . = ALIGN({{@root.i.align}});
{{/if}}
{{#if @root.i.copyFrom}}
        {{@root.i.name}}_ram_start = .;
{{/if}}
        {{@root.i.name}}_start = .;
{{#if @root.i.altSymbolStart}}
        {{@root.i.altSymbolStart}} = .; 
{{/if}}
{{#if @root.i.children}}
{{#each (sortBy @root.i.children (createArray 'order' 'name'))}}
{{#if this.align}}
        . = ALIGN({{this.align}});
{{/if}}
        {{this.name}}_region_start = .;
{{#if this.altSymbolStart}}
        {{this.altSymbolStart}} = .; 
{{/if}}
        *({{#if (not this.removeDot)}}.{{/if}}{{this.name}})
{{#if this.wildcard}}
        "*({{#if (not this.removeDot)}}.{{/if}}{{this.name}}*)"
{{/if}}
        {{#if this.binFile}}
        {{#each (split this.binFile ';')}}
        {{{this}}} 
        {{/each}}
        {{/if}}
        {{this.name}}_region_end = .;
        {{#if this.altSymbolEnd}} {{this.altSymbolEnd}} = .; {{/if}}
        {{/each}}
        {{/if}}
        {{#if (and @root.i.groupSize (eq (length @root.i.children) 0))}}
        . += {{@root.i.groupSize}};
        {{/if}}
        {{#if @root.i.endAlign}}
        . = ALIGN({{@root.i.endAlign}});
        {{/if}}
        {{@root.i.name}}_end = .;
{{#if @root.i.altSymbolEnd}} 
        {{@root.i.altSymbolEnd}} = .; 
{{/if}}
{{#if @root.i.copyFrom}}
        {{@root.i.name}}_ram_end = .;
{{/if}}
    } > {{@root.i.parent.name}}
{{/if}}
   
{{/each}}
    {{#each @root.level1}}
    {{this.name}}_memory_start = ADDR({{this.name}});
    {{this.name}}_memory_end = ADDR({{this.name}}) + SIZEOF({{this.name}});
    {{this.name}}_memory_size = SIZEOF({{this.name}});
{{/each}}
}