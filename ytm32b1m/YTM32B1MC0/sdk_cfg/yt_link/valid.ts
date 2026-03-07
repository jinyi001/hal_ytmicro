import { LinkItem } from "./view";
import Schema from 'async-validator';
import { Rules } from 'async-validator'

Schema.warning = function () { null };


export async function validLevel1(item: LinkItem, parent: LinkItem): Promise<Record<string, string>> {
    const ret: Record<string, string> = {};
    function nameCheck(rule: any, value: any, callback: any) {
        /* check name is unique in parent, if id is same, skip*/
        if (parent.children) {
            if (parent.children.find((v) => (v.name == value && v.id != item.id))) {
                callback(new Error('Name is not unique in parent'));
            }
        }
        callback()
    }
    function initCheck(rule: any, value: any, callback: any) {
        /* check name is unique in parent, if id is same, skip*/
        if(item.isFlash){
            callback()
        }else{
            if(value){
                callback()
            }else{
                callback(new Error('Init policy is necessary'))
            }
        }
    }
    if (item.level == 1) {
        const Rules: Rules = {
            name: [
                { required: true, message: 'Name is necessary' },
                { validator: nameCheck }
            ],
            init: [
                { validator: initCheck  }
            ]
        }

        Rules.boundary = [{
            required: true, message: 'Boundary is necessary'
        }]
        Rules.size = [{
            required: true, message: 'Size is necessary'
        }, {
            validator: (rule: any, value: any, callback: any) => {
                const t = Number(value);
                if (Number.isNaN(t)) {
                    callback(new Error('Size must be number, support hex, octal, decimal and binary'));
                }
                let parentTotalSize = 0;
                parent.children?.forEach((v) => {
                    if (v.id != item.id)
                        parentTotalSize += Number(v.size);
                })
                if ((parentTotalSize + Number(value)) > Number(parent.size)) {
                    callback(new Error('Total size of children can not be greater than parent size'));
                }
                callback()
            }
        }]
        const validator = new Schema(Rules);
        const p = new Promise<Record<string, string>>((resolve, reject) => {
            validator.validate(item).then(() => {
                resolve(ret)
            }).catch(({ errors, fields }) => {
                for (const key of Object.keys(fields)) {
                    /* join all message */
                    ret[key] = fields[key].map((v: any) => v.message).join('\n')
                }
                resolve(ret)
            });
        })
        return p;
    } else {
        return new Promise<Record<string, string>>((resolve, reject) => {
            resolve(ret)
        })
    }
}


export async function validLevel2(item: LinkItem, parent: LinkItem): Promise<Record<string, string>> {
    const ret: Record<string, string> = {};
    function nameCheck(rule: any, value: any, callback: any) {
        /* check name is unique in parent, if id is same, skip*/
        if (parent.children) {
            if (parent.children.find((v) => (v.name == value && v.id != item.id))) {
                callback(new Error('Name is not unique in parent'));
            }
        }
        callback()
    }
    if (item.level == 2) {
        const Rules: Rules = {
            name: [
                { required: true, message: 'Name is necessary' },
                { validator: nameCheck }
            ]
        }
        Rules.align = [{
            validator: (rule: any, value: any, callback: any) => {
                if(value){
                    const t = Number(value);
                    if (Number.isNaN(t)) {
                        callback(new Error('Size must be number, support hex, octal, decimal and binary'));
                    }
                    if(t%2){
                        callback(new Error('Align must be multiple of 2'));
                    }
                }
                callback()
               
            }
        }]
        const validator = new Schema(Rules);
        const p = new Promise<Record<string, string>>((resolve, reject) => {
            validator.validate(item).then(() => {
                resolve(ret)
            }).catch(({ errors, fields }) => {
                for (const key of Object.keys(fields)) {
                    /* join all message */
                    ret[key] = fields[key].map((v: any) => v.message).join('\n')
                }
                resolve(ret)
            });
        })
        return p;
    } else {
        return new Promise<Record<string, string>>((resolve, reject) => {
            resolve(ret)
        })
    }
}


export async function validLevel3(item: LinkItem, parent: LinkItem, ceilMap:Map<string,LinkItem>): Promise<Record<string, string>> {
    const ret: Record<string, string> = {};
    function nameCheck(rule: any, value: any, callback: any) {
        /* check name is unique in parent, if id is same, skip*/
        if (parent.children) {
            if (parent.children.find((v) => (v.name == value && v.id != item.id))) {
                callback(new Error('Name is not unique in parent'));
            }
        }
        callback()
    }
    if (item.level == 3) {
        const Rules: Rules = {
            name: [
                { required: true, message: 'Name is necessary' },
                { validator: nameCheck }
            ]
        }
        Rules.copyFrom= [{
            validator: (rule: any, value: any, callback: any) => {
                if(value){
                    /* copy section can't have children */
                    if(ceilMap.get(value)){
                        const t = ceilMap.get(value);
                        if(t?.children?.length){
                            callback(new Error('Copy from section can not have children'));
                        }
                    }else{
                        callback(new Error('Copy from section not exist'));
                    }
                }
                callback()
               
            }
        }]
        Rules.align = [{
            validator: (rule: any, value: any, callback: any) => {
                if(value){
                    const t = Number(value);
                    if (Number.isNaN(t)) {
                        callback(new Error('Size must be number, support hex, octal, decimal and binary'));
                    }
                    if(t%2){
                        callback(new Error('Align must be multiple of 2'));
                    }
                }
                callback()
               
            }
        }]
        Rules.endAlign = [{
            validator: (rule: any, value: any, callback: any) => {
                if(value){
                    const t = Number(value);
                    if (Number.isNaN(t)) {
                        callback(new Error('Size must be number, support hex, octal, decimal and binary'));
                    }
                    if(t%2){
                        callback(new Error('EndAlign must be multiple of 2'));
                    }
                }
                callback()
               
            }
        }]
        Rules.groupSize = [{
            validator: (rule: any, value: any, callback: any) => {
                if(value){
                    const t = Number(value);
                    if (Number.isNaN(t)) {
                        callback(new Error('Size must be number, support hex, octal, decimal and binary'));
                    }
                    
                }
                callback()
               
            }
        }]
        const validator = new Schema(Rules);
        const p = new Promise<Record<string, string>>((resolve, reject) => {
            validator.validate(item).then(() => {
                resolve(ret)
            }).catch(({ errors, fields }) => {
                for (const key of Object.keys(fields)) {
                    /* join all message */
                    ret[key] = fields[key].map((v: any) => v.message).join('\n')
                }
                resolve(ret)
            });
        })
        return p;
    } else {
        return new Promise<Record<string, string>>((resolve, reject) => {
            resolve(ret)
        })
    }
}

export async function validLevel4(item: LinkItem, parent: LinkItem): Promise<Record<string, string>> {
    const ret: Record<string, string> = {};
    function nameCheck(rule: any, value: any, callback: any) {
        /* check name is unique in parent, if id is same, skip*/
        if (parent.children) {
            if (parent.children.find((v) => (v.name == value && v.id != item.id))) {
                callback(new Error('Name is not unique in parent'));
            }
        }
        callback()
    }
    if (item.level == 4) {
        const Rules: Rules = {
            name: [
                { required: true, message: 'Name is necessary' },
                { validator: nameCheck }
            ]
        }
        Rules.flags = [{
         required: true, message: 'Flags is necessary'
        }]
        Rules.align = [{
            validator: (rule: any, value: any, callback: any) => {
                if(value){
                    const t = Number(value);
                    if (Number.isNaN(t)) {
                        callback(new Error('Size must be number, support hex, octal, decimal and binary'));
                    }
                    if(t%2){
                        callback(new Error('Align must be multiple of 2'));
                    }
                }
                callback()
               
            }
        }]
        const validator = new Schema(Rules);
        const p = new Promise<Record<string, string>>((resolve, reject) => {
            validator.validate(item).then(() => {
                resolve(ret)
            }).catch(({ errors, fields }) => {
                for (const key of Object.keys(fields)) {
                    /* join all message */
                    ret[key] = fields[key].map((v: any) => v.message).join('\n')
                }
                resolve(ret)
            });
        })
        return p;
    } else {
        return new Promise<Record<string, string>>((resolve, reject) => {
            resolve(ret)
        })
    }
}