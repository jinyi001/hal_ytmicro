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
    {{this.name}}   {{#if this.isFlash}}(RX){{else}}(RW){{/if}} : ORIGIN = 0x{{nToString this.startAddress 16}}, LENGTH = 0x{{nToString this.size 16}}
{{/each}}
}
{{setVar2 'memoryUsed' (createObject)~}}
/* SECTIONS */
SECTIONS
{
{{#each @root.sectionList}}
{{setVar2 'i' (get this @root.sections)~}}
{{setVar2 'xx' (itemAt @root.sectionList (add @index 1))}}
{{#if @root.xx~}}
{{setVar2 'ni' (get @root.xx @root.sections)~}}
{{/if~}}
{{#if @root.i.needCopy}}
{{#if @root.i.align}}
    {{@root.i.name}}_rom_start_not_align = {{#if @root.atAddr~}}{{@root.atAddr~}}{{else~}}{{#if (get @root.i.parent.name @root.memoryUsed)}}.{{else}}{{@root.i.parent.startAddress}}{{/if}}{{/if~}};
    {{@root.i.name}}_rom_start = {{@root.i.name}}_rom_start_not_align + ({{@root.i.align}} - {{@root.i.name}}_rom_start_not_align % {{@root.i.align}});
{{else}}
    {{@root.i.name}}_rom_start = {{#if @root.atAddr~}}{{@root.atAddr~}}{{else~}}{{#if (get @root.i.parent.name @root.memoryUsed)}}.{{else}}{{@root.i.parent.startAddress}}{{/if}}{{/if~}};
{{/if}}
{{else}}
    .{{@root.i.name}} {{#if (and @root.i.isFlash @root.atAddr)}}{{#eq @root.lastParent @root.i.parent.id}}{{#if (and @root.atAddr @root.i.align)}}ALIGN({{@root.atAddr}},{{@root.i.align}}){{else}}{{@root.atAddr}}{{/if}}{{/eq}}{{/if}} {{#isTruthy @root.i.flags~}}({{@root.i.flags}}){{/isTruthy~}}: {{#if @root.i.copyFrom~}} AT({{@root.i.name}}_rom_start) {{/if~}}
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
{{#isTruthy this.flags}}
        {{this.flags}}(*({{#if (not this.removeDot)}}.{{/if}}{{this.name}}))
{{#if this.wildcard}}
        {{this.flags}}(*({{#if (not this.removeDot)}}.{{/if}}{{this.name}}*))
{{/if}}
{{else}}
        *({{#if (not this.removeDot)}}.{{/if}}{{this.name}})
{{#if this.wildcard}}
        *({{#if (not this.removeDot)}}.{{/if}}{{this.name}}*)
{{/if}}
{{/isTruthy}}
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
   
{{#if @root.i.needCopy}}
    {{@root.i.name}}_rom_end = {{@root.i.name}}_rom_start + {{@root.i.name}}_ram_end - {{@root.i.name}}_ram_start;
    {{#if (and @root.ni (eq @root.ni.parent.id @root.i.parent.id))~}}
        {{setVar2 'atAddr' (add @root.i.name '_rom_end')~}}
    {{else~}}
         {{setVar2 'atAddr' ''~}}
    {{/if~}}
{{else}}
    {{setVar2 'atAddr' ''~}}
{{/if}}

{{#if @root.i.copyFrom}}
    ASSERT(({{@root.i.name}}_ram_end - {{@root.i.name}}_ram_start) == ({{@root.i.name}}_rom_end - {{@root.i.name}}_rom_start), "Copy Section {{@root.i.name}} Size non-aligned")
{{/if}}

{{setVar2 'lastParent' @root.i.parent.id}}
{{/each}}
    {{#each @root.level1}}
    {{this.name}}_memory_start = ORIGIN({{this.name}});
    {{this.name}}_memory_end = ORIGIN({{this.name}}) + LENGTH({{this.name}});
    {{this.name}}_memory_size = LENGTH({{this.name}});
{{/each}}
}