<template>
  <div>
    <div style="margin:5px">
      <el-switch
        v-model="enable"
        active-text="Enable"
        inactive-text="Disable"
        @change="emits('change')"
        :disabled="lock"
      />
    </div>
    <div v-if="enable">
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data"
            style="width: 100%"
            border
            :row-class-name="tableRowClassName"
            @row-click="masterRowClick"
          >
            <el-table-column
              align="right"
              :resizable="false"
            >
              <template #header>
                <el-button-group>
                  <el-button
                    type="primary"
                    plain
                    :icon="Plus"
                    size="small"
                    @click="addItem('master')"
                    :disabled="lock"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    @click="deleteItem('master')"
                    :disabled="lock"
                  />
                </el-button-group>
              </template>
              <template #default="scope">
                <span
                  v-if="scope.$index == activeMasterInst"
                  style="margin-right:5px"
                >
                  <el-icon>
                    <FolderOpened />
                  </el-icon>
                </span><span v-if="scope.row.name">{{ scope.row.name }}</span><span v-else>{{
                  scope.$index
                }}</span>
              </template>
            </el-table-column>
          </el-table>
        </el-col>
        <el-col :span="18">
          <div v-if="data[activeMasterInst]">
            <el-form
              :disabled="lock"
              ref="masterRef"
              :model="data[activeMasterInst]"
              label-width="280px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="data[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>
              <el-form-item
                label="Counter init value"
                prop="initialValue"
              >
                <el-input v-model.number="data[activeMasterInst].initialValue" />
              </el-form-item>
              <el-form-item
                label="Counter final value"
                prop="finalValue"
              >
                <el-input v-model.number="data[activeMasterInst].finalValue" />
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, nextTick } from "vue";
import { Plugin } from "plugin";
import {
    Plus,
    Delete,
    FolderOpened
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const defMasterParam = {
    name: 'ETMR_MC_Config',
    readonly: false,
    initialValue: 0,
    finalValue: 0,
}

const plugin = new Plugin();
const props = defineProps<{
    data: any;
    lock: boolean
}>();
const data = toRef(props.data, 'data')
const lock = toRef(props, 'lock')
const enable = toRef(props.data, 'enable')
const winW = ref(window.innerWidth)
function resizeHandler() {
    winW.value = window.innerWidth
}

watch(activeMasterInst, () => {
    nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
            null
        })
    })
})

function addItem(key: string) {

    if (key == 'master') {
      const newItem=JSON.parse(JSON.stringify(defMasterParam))
      const default_name=newItem.name
      newItem.name=`${default_name}${data.value.length}`
      data.value.push(newItem)
      activeMasterInst.value = data.value.length - 1

    }

}

function deleteItem(key: string) {
    if (data.value.length >= 0) {
        if (key == 'master') {
            data.value.splice(activeMasterInst.value, 1)
            activeMasterInst.value = data.value.length - 1
        }
    }
}

function masterRowClick(row) {
    const index = data.value.indexOf(row)
    activeMasterInst.value = index
    nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
            null
        })
    })
}
function masterNameCheck(rule: any, value: any, callback: any) {
    const err = false;
    const cnt = {}

    for (let i = 0; i < data.value.length; i++) {
        if (cnt[data.value[i][rule.field]] == undefined) {
            cnt[data.value[i][rule.field]] = 1
        } else {
            cnt[data.value[i][rule.field]]++
        }
    }

    for (const key of Object.keys(cnt)) {
        if (cnt[key] > 1) {
            callback(new Error(`Duplicate ${key}`))
        }
    }
    if (!err) {
        callback()
    }
}




const MasterRules = reactive<FormRules>({
    name: [
        { required: true, message: 'Please input name', trigger: 'change' },
        { validator: masterNameCheck, trigger: 'change' }
    ],
    // syncMethod: {
    //     type: 'object',
    //     required: true,
    //     fields: {
    //         regSyncFreq: regSyncFreqRule
    //     }
    // },
    // outputTrigConfig: {
    //     type: 'object',
    //     required: true,
    //     fields: {
    //         pwmOutputChannel: pwmOutputChannelRule,
    //         outputTrigWidth: outputTrigWidthRule,
    //         outputTrigFreq: outputTrigFreqRule,
    //     }

    // }
})



const errorid: string[] = []
onBeforeUnmount(() => {
    window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
    for (const error of errorid) {
        plugin.deleteLog('error', error)
    }
    const masterValidator = new Schema(MasterRules);
    for (const i in data.value) {
        masterValidator.validate(data.value[i], (errors, fields) => {
            if (errors) {
                // validation failed, errors is an array of all errors
                // fields is an object keyed by field name with an array of
                // errors per field
                for (const j in errors) {
                    const id = `CM-${errors[j].field}-${i}-${j}`
                    plugin.addLog('error', id, errors[j].message)
                    errorid.push(id)

                }
            }

        });
    }
}

const emits = defineEmits(['change'])

onMounted(() => {
    window.addEventListener("resize", resizeHandler);

    nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
            null
        })
    })
    dataCheck();
    watch(data, () => {
        dataCheck();
        emits('change')
    }, { deep: true })
});



const tableRowClassName = ({
    row,
    rowIndex,
}: {
    row: any
    rowIndex: number
}) => {
    if (rowIndex === activeMasterInst.value) {
        return 'active-row'
    }
    return ''
}

</script>
<style scoped>
.el-table .active-row {
    color: #409EFF
}

.table {
    width: v-bind(winW*0.75-60+'px');
    height: 160px;
    border-radius: 4px;
    border: 1px solid var(--el-border-color);
}
</style>