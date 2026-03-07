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
                    :disabled="lock"
                    @click="addItem('master')"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    :disabled="lock"
                    @click="deleteItem('master')"
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
                label="Mode"
                prop="mode"
              >
                <el-select
                  v-model.number="data[activeMasterInst].mode"
                  style="width:100%"
                >
                  <el-option
                    value="eTMR_QUAD_PHASE_B_FOLLOW_A"
                    label="Phase A and B, B follow A, counter increase"
                  />
                  <el-option
                    value="eTMR_QUAD_PHASE_A_FOLLOW_B"
                    label="Phase A and B, A follow B, counter increase"
                  />
                  <el-option
                    value="eTMR_QUAD_COUNT_AND_DIR_BY_PHASE_B"
                    label="Counting and direction mode, phase B controls direction"
                  />
                  <el-option
                    value="eTMR_QUAD_COUNT_AND_DIR_BY_PHASE_A"
                    label="Counting and direction mode, phase A controls direction"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Clock prescaler"
                prop="qdClkPrs"
              >
                <el-select
                  v-model.number="data[activeMasterInst].qdClkPrs"
                  style="width:100%"
                >
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_1"
                    label="/1"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_2"
                    label="/2"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_4"
                    label="/4"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_8"
                    label="/8"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_16"
                    label="/16"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_32"
                    label="/32"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_64"
                    label="/64"
                  />
                  <el-option
                    value="eTMR_QD_CLOCK_DIV_BY_128"
                    label="/128"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Counter init value"
                prop="initVal"
                v-if="FEATURE_eTMR_HAS_INIT_REG"
              >
                <el-input v-model.number="data[activeMasterInst].initVal" />
              </el-form-item>
              <el-form-item
                label="Counter mod value"
                v-if="FEATURE_eTMR_HAS_QUAD_DECODER_MOD"
                prop="modVal"
              >
                <el-input v-model.number="data[activeMasterInst].modVal" />
              </el-form-item>
              <el-form-item
                label="Filter sample count"
                prop="qdInputFilterSampleCount"
              >
                <el-input v-model.number="data[activeMasterInst].qdInputFilterSampleCount" />
              </el-form-item>
              <el-form-item
                label="Filter sample period"
                prop="qdInputFilterSamplePeriod"
              >
                <el-input v-model.number="data[activeMasterInst].qdInputFilterSamplePeriod" />
              </el-form-item>
              <el-form-item
                label="Pashe A polarity"
                prop="phaseAPol"
              >
                <el-select
                  v-model.number="data[activeMasterInst].phaseAPol"
                  style="width:100%"
                >
                  <el-option
                    value="eTMR_QUAD_PHASE_NORMAL"
                    label="Normal"
                  />
                  <el-option
                    value="eTMR_QUAD_PHASE_INVERT"
                    label="Invert"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Pashe B polarity"
                prop="phaseBPol"
              >
                <el-select
                  v-model.number="data[activeMasterInst].phaseBPol"
                  style="width:100%"
                >
                  <el-option
                    value="eTMR_QUAD_PHASE_NORMAL"
                    label="Normal"
                  />
                  <el-option
                    value="eTMR_QUAD_PHASE_INVERT"
                    label="Invert"
                  />
                </el-select>
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, inject, reactive, watch, toRef, onBeforeUnmount, nextTick } from "vue";

import {
    Plus,
    Delete,
    FolderOpened
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };
const plugin = inject("plugin") as any;
const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const defMasterParam = {

    name: 'ETMR_QD_Config',
    readonly: false,
    mode: 'eTMR_QUAD_PHASE_B_FOLLOW_A',
    qdClkPrs: 'eTMR_QD_CLOCK_DIV_BY_1',
    initVal: 0,
    modVal: 0,
    qdInputFilterSampleCount: 0,
    qdInputFilterSamplePeriod: 0,
    phaseAPol: 'eTMR_QUAD_PHASE_NORMAL',
    phaseBPol: 'eTMR_QUAD_PHASE_NORMAL'
}

const props = defineProps<{
    data: any;
    lock: boolean
}>();
const data = toRef(props.data, 'data')
const enable = toRef(props.data, 'enable')
const lock = toRef(props, 'lock')
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

const FEATURE_eTMR_HAS_INIT_REG =ref(false)
const FEATURE_eTMR_HAS_QUAD_DECODER_MOD = ref(false)
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