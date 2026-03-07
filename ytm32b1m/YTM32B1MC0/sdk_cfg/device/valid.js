
import Schema from 'async-validator';
import workerpool from 'workerpool';

export async function valid(data,extRules) {
    /* focus on change the validator function, you can access data object in the function */

    const rules = {
        stack_top: [
            { required: true, message: 'Please choose stack top symbol name' },
            ...(extRules?.stack_top || [])
        ],
        vector_ram:[
            { required: true, message: 'Please choose vector ram symbol name' },
            ...(extRules?.vector_ram || [])
        ],
        vector_flash:[
            { required: true, message: 'Please choose vector flash start symbol name' },
            ...(extRules?.vector_flash || [])
        ],
        vector_flash_end:[
            { required: true, message: 'Please choose vector flash end symbol name' },
            ...(extRules?.vector_flash_end || [])
        ],
        vector_flash_region:[
            { required: true, message: 'Please choose vector flash region name' },
            ...(extRules?.vector_flash_region || [])
        ]
    }
    /* doesn't change below code */
    const ret = {}
    const masterValidator = new Schema(rules);
   
        try {
            await masterValidator.validate(data)
        } catch ({ errors, fields }) {
            for (const e of errors) {
                const id = `${e.field}`
                ret[id] = e.message
            }
        }

    

    return ret
}

workerpool.worker({
    valid: valid,
});