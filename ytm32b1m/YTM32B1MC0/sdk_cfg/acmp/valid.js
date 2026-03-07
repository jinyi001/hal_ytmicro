
import Schema from 'async-validator';
import workerpool from 'workerpool';

export async function valid(data) {
    /* focus on change the validator function, you can access data object in the function */
    function masterNameCheck(rule, value, callback) {
        const cnt = {}

        for (let i = 0; i < data.length; i++) {
            if (cnt[data[i].name] == undefined) {
                cnt[data[i].name] = 1
            } else {
                cnt[data[i].name]++
            }
        }

        for (const key of Object.keys(cnt)) {
            if (cnt[key] > 1) {
                callback(new Error(`Duplicate ${key}`))
            }
        }
        callback()
    }

    const rules = {
        name: [
            { required: true, message: 'Please input name' },
            { validator: masterNameCheck }
        ],
        comparator: {
            type: 'object',
            required: true,
            fields: {
                filterSamplePeriod: [
                    { type: 'number', required: true, message: 'Please input filter sample period', },
                    { type: 'number', min: 0, max: 255, message: 'Filter sample period from 0-255', },
                ],
                filterSampleCount: [
                    { type: 'number', required: true, message: 'Please input filter sample count', },
                    { type: 'number', min: 0, max: 255, message: 'Filter sample count from 0-255', },
                ]
            }
        },
        mux: {
            type: 'object',
            required: true,
            fields: {
                negativeInputChnSel: [
                    { type: 'number', required: true, message: 'Please input negative mux', },
                    { type: 'number', min: 0, max: 7, message: 'Negative mux from 0-7' },
                ],
                positiveInputChnSel: [
                    { type: 'number', required: true, message: 'Please input positive mux' },
                    { type: 'number', min: 0, max: 7, message: 'Positive mux from 0-7' },
                ]
            }
        },
        dac: {
            type: 'object',
            required: true,
            fields: {
                voltage: [
                    { type: 'number', required: true, message: 'Please input DAC digital value' },
                    { type: 'number', min: 0, max: 255, message: 'DAC digital value from 0-255', },
                ],
            }
        },
        continuous: {
            type: 'object',
            required: true,
            fields: {
                samplePeriod: [
                    { type: 'number', required: true, message: 'Please input sample period' },
                    { type: 'number', min: 0, max: 255, message: 'Sample period from 0-255' },
                ],
                samplePosition: [
                    { type: 'number', required: true, message: 'Please input sample position' },
                    { type: 'number', min: 0, max: 255, message: 'Sample position from 0-255' },
                ]
            }
        }
    }
    /* doesn't change below code */
    const ret = {}
    const masterValidator = new Schema(rules);
    for (let i = 0; i < data.length; i++) {
        try {
            await masterValidator.validate(data[i])
        } catch ({ errors, fields }) {
            for (const e of errors) {
                const id = `${i}.${e.field}`
                ret[id] = e.message
            }
        }

    }

    return ret
}

workerpool.worker({
    valid: valid,
});