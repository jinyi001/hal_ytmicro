<template>
  <div>
    <div>
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data"
            style="width: 80%"
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
              ref="masterRef"
              :disabled="lock"
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
                label="Counter Clock"
                prop="etmrClockSource"
              >
                <el-select
                  v-model.number="data[activeMasterInst].etmrClockSource"
                  style="width:100%"
                >
                  <el-option
                    value="eTMR_CLOCK_SOURCE_INTERNALCLK"
                    label="Internal Clock"
                  />
                  <el-option
                    value="eTMR_CLOCK_SOURCE_EXTERNALCLK"
                    label="External Clock"
                  />
                  <el-option
                    value="eTMR_CLOCK_SOURCE_EXT_IPC_CLK"
                    label="IPC clock"
                    :disabled="FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE==false"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Divider Value"
                prop="etmrPrescaler"
              >
                <el-input v-model.number="data[activeMasterInst].etmrPrescaler" />
              </el-form-item>

              <!-- debug disable -->
              <el-form-item
                label="Debug Mode"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="<span>false: counter stops under debug mode<br>true: counter keeps running under debug mode</span>"
                    raw-content
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-checkbox v-model.number="data[activeMasterInst].debugMode" />
              </el-form-item>

              <el-form-item
                label="Overflow Interrupt Enable"
                prop="isTofIntEnabled"
              >
                <el-checkbox v-model.number="data[activeMasterInst].isTofIntEnabled" />
              </el-form-item>
              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  title="PWM Sync Config"
                  name="1"
                >
                  <el-form-item
                    label="Register loading frequency"
                    prop="syncMethod.regSyncFreq"
                    :rules="regSyncFreqRule"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>
                              每满足n次加载事件，就实际加载一次，即加载频率
                      </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-input v-model.number="data[activeMasterInst].syncMethod.regSyncFreq" />
                  </el-form-item>

                  <el-form-item
                    label="Register Loading Opportunity"
                    prop="syncMethod.regSyncSel"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>
                              INIT，MOD，MID，CH[x].VAL0，CH[x].VAL1等寄存器加载时机
                      </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-select
                      v-model.number="data[activeMasterInst].syncMethod.regSyncSel"
                      style="width:100%"
                    >
                      <el-option
                        value="REG_SYNC_DISABLED"
                        label="Synchronized disabled"
                      />
                      <el-option
                        value="REG_SYNC_WITH_MOD"
                        label="Synchronized with counter matching MOD"
                      />
                      <el-option
                        value="REG_SYNC_WITH_MID"
                        v-show="FEATURE_eTMR_HAS_MID==true"
                        label="Synchronized with counter matching MID"
                      />
                      <el-option
                        value="REG_SYNC_WITH_TRIG"
                        label="Synchronized with register loading trigger source"
                      />
                    </el-select>
                  </el-form-item>

                  <el-form-item
                    label="Counter Loading Opportunity"
                    prop="syncMethod.cntInitSyncSel"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>
                              计数器计数值加载时机
                      </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-select
                      v-model.number="data[activeMasterInst].syncMethod.cntInitSyncSel"
                      style="width:100%"
                    >
                      <el-option
                        value="CNT_SYNC_WITH_REG"
                        label="Synchronized with register loading"
                      />
                      <el-option
                        value="CNT_SYNC_WITH_MID"
                        v-show="FEATURE_eTMR_HAS_MID==true"
                        label="Synchronized with counter matching MID"
                      />
                      <el-option
                        value="CNT_SYNC_WITH_TRIG"
                        label="Synchronized with counter loading trigger source"
                      />
                      <el-option
                        value="CNT_SYNC_DISABLED"
                        label="Synchronized disabled"
                      />
                    </el-select>
                  </el-form-item>

                  <el-form-item
                    label="Mask Output Loading Opportunity"
                    prop="syncMethod.maskOutputSyncSel"
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].syncMethod.maskOutputSyncSel"
                      style="width:100%"
                    >
                      <el-option
                        value="CHMASK_SYNC_WITH_REG"
                        label="Synchronized with register loading"
                      />
                      <el-option
                        value="CHMASK_SYNC_WITH_MOD"
                        label="Synchronized with counter matching MOD"
                      />
                      <el-option
                        value="CHMASK_SYNC_WITH_MID"
                        v-show="FEATURE_eTMR_HAS_MID==true"
                        label="Synchronized with counter matching MID"
                      />
                      <el-option
                        value="CHMASK_SYNC_WITH_TRIG"
                        label="Synchronized with mask output loading trigger source"
                      />
                      <el-option
                        value="CHMASK_SYNC_DISABLED"
                        v-show="FEATURE_eTMR_HAS_MID==false"
                        label="Synchronized disabled"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Register Loading Trigger Source"
                    prop="syncMethod.regSyncTrigSrc"
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].syncMethod.regSyncTrigSrc"
                      style="width:100%"
                    >
                      <el-option
                        value="DISABLE_TRIGGER"
                        label="Disable trigger"
                      />
                      <el-option
                        value="SW_TRIGGER"
                        label="Software trigger to load"
                      />
                      <el-option
                        value="HW_TRIGGER"
                        label="Hardware trigger to load"
                      />
                      <el-option
                        value="SW_AND_HW_TRIGGER"
                        label="Hardware amd software trigger to load"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Counter Loading Trigger Source"
                    prop="syncMethod.cntInitSyncTrigSrc"
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].syncMethod.cntInitSyncTrigSrc"
                      style="width:100%"
                    >
                      <el-option
                        value="DISABLE_TRIGGER"
                        label="Disable trigger"
                      />
                      <el-option
                        value="SW_TRIGGER"
                        label="Software trigger to load"
                      />
                      <el-option
                        value="HW_TRIGGER"
                        label="Hardware trigger to load"
                      />
                      <el-option
                        value="SW_AND_HW_TRIGGER"
                        label="Hardware amd software trigger to load"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Mask Output Loading Trigger Source"
                    prop="syncMethod.maskOutputSyncTrigSrc"
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].syncMethod.maskOutputSyncTrigSrc"
                      style="width:100%"
                    >
                      <el-option
                        value="DISABLE_TRIGGER"
                        label="Disable trigger"
                      />
                      <el-option
                        value="SW_TRIGGER"
                        label="Software trigger to load"
                      />
                      <el-option
                        value="HW_TRIGGER"
                        label="Hardware trigger to load"
                      />
                      <el-option
                        value="SW_AND_HW_TRIGGER"
                        label="Hardware amd software trigger to load"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Hardware trigger from TMU"
                    prop="syncMethod.hwTrigRegLoadEnable"
                  >
                    <el-checkbox
                      v-model.number="data[activeMasterInst].syncMethod.hwTrigRegLoadEnable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Hardware trigger from CIM"
                    prop="syncMethod.hwTrigCounterLoadEnable"
                  >
                    <el-checkbox
                      v-model.number="data[activeMasterInst].syncMethod.hwTrigCounterLoadEnable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Hardware trigger from pad"
                    prop="syncMethod.hwTrigMaskLoadEnable"
                  >
                    <el-checkbox
                      v-model.number="data[activeMasterInst].syncMethod.hwTrigMaskLoadEnable"
                    />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="Trigger source"
                  name="2"
                >
                  <el-form-item
                    label="Trigger source"
                    prop="outputTrigConfig.trigSrc"
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].outputTrigConfig.trigSrc"
                      style="width:100%"
                    >
                      <el-option
                        value="TRIGGER_FROM_MATCHING_EVENT"
                        label="Trigger source from matching event"
                      />
                      <el-option
                        value="TRIGGER_FROM_PWM"
                        label="Trigger source from PWM"
                      />
                    </el-select>
                  </el-form-item>

                  <el-form-item
                    label="Output trigger frequency"
                    v-show="data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_MATCHING_EVENT'"
                    prop="outputTrigConfig.outputTrigFreq"
                    :rules="outputTrigFreqRule"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].outputTrigConfig.outputTrigFreq"
                    />
                  </el-form-item>

                  <el-form-item
                    label="PWM channel ID"
                    v-show="data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_PWM'"
                    prop="outputTrigConfig.pwmOutputChannel"
                    :rules="pwmOutputChannelRule"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].outputTrigConfig.pwmOutputChannel"
                    />
                  </el-form-item>

                  <el-form-item
                    label="Output trigger width"
                    v-show="FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH==true && data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_MATCHING_EVENT'"
                    prop="outputTrigConfig.outputTrigWidth"
                    :rules="outputTrigWidthRule"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].outputTrigConfig.outputTrigWidth"
                    />
                  </el-form-item>

                  <el-form-item
                    label="Enable MOD match trigger"
                    v-show="data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_MATCHING_EVENT'"
                    prop="outputTrigConfig.modMatchTrigEnable"
                  >
                    <el-checkbox
                      v-model.number="data[activeMasterInst].outputTrigConfig.modMatchTrigEnable"
                    />
                  </el-form-item>

                  <el-form-item
                    label="Enable MID match trigger"
                    v-show="FEATURE_eTMR_HAS_MID==true && data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_MATCHING_EVENT'"
                    prop="outputTrigConfig.midMatchTrigEnable"
                  >
                    <el-checkbox
                      v-model.Boolean="data[activeMasterInst].outputTrigConfig.midMatchTrigEnable"
                    />
                  </el-form-item>

                  <el-form-item
                    label="Enable INIT match trigger"
                    v-show="FEATURE_eTMR_HAS_INIT_REG==true && data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_MATCHING_EVENT'"
                    prop="outputTrigConfig.initMatchTrigEnable"
                  >
                    <el-checkbox
                      v-model.number="data[activeMasterInst].outputTrigConfig.initMatchTrigEnable"
                    />
                  </el-form-item>

                  <el-form-item
                    label-width="0"
                    v-show="data[activeMasterInst].outputTrigConfig.trigSrc=='TRIGGER_FROM_MATCHING_EVENT'"
                    prop="outputTrigConfig.channelTrigParamConfig"
                  >
                    <div class="table">
                      <el-auto-resizer>
                        <template #default="{ height, width }">
                          <el-table-v2
                            :columns="columns"
                            :data="data[activeMasterInst].outputTrigConfig.channelTrigParamConfig"
                            :estimated-row-height="30"
                            :width="width"
                            :header-height="30"
                            :height="height"
                            :footer-height="40"
                            fixed
                          >
                            <!-- <template #empty>
              <el-empty :image-size="80" />
            </template> -->
                            <template #footer>
                              <el-alert
                                v-if="filterError != ''"
                                center
                                show-icon
                                :title="filterError"
                                type="error"
                                :closable="false"
                              />
                            </template>
                          </el-table-v2>
                        </template>
                      </el-auto-resizer>
                    </div>
                  </el-form-item>
                </el-collapse-item>
              </el-collapse>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, nextTick, inject } from "vue";
import { Plugin } from "plugin";
import {
    Plus,
    Delete,
    FolderOpened,
    QuestionFilled
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
import type { Column } from '@element-plus/components/table-v2'
Schema.warning = function () { null };

const FEATURE_eTMR_HAS_MID = ref(false)
const FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH = ref(false)
const FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE = ref(true)
const FEATURE_eTMR_HAS_INIT_REG = ref(false)
const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const col = ref([])
const defMasterParam = {

    name: 'ETMR_CM_Config',
    readonly: false,
    etmrClockSource: 'eTMR_CLOCK_SOURCE_INTERNALCLK',
    etmrPrescaler:1,
    debugMode: false,
    isTofIntEnabled: false,
    syncMethod: {
        regSyncFreq: 1,
        regSyncSel: 'REG_SYNC_WITH_MOD',
        cntInitSyncSel: 'CNT_SYNC_WITH_REG',
        maskOutputSyncSel: 'CHMASK_SYNC_WITH_REG',
        regSyncTrigSrc: 'DISABLE_TRIGGER',
        cntInitSyncTrigSrc: 'DISABLE_TRIGGER',
        maskOutputSyncTrigSrc: 'DISABLE_TRIGGER',
        hwTrigRegLoadEnable: false,
        hwTrigCounterLoadEnable: false,
        hwTrigMaskLoadEnable: false,
    },
    outputTrigConfig: {
        trigSrc: 'TRIGGER_FROM_MATCHING_EVENT',
        pwmOutputChannel: 0,
        outputTrigWidth: 0,
        outputTrigFreq: 1,
        modMatchTrigEnable: false,
        midMatchTrigEnable: false,
        initMatchTrigEnable: false,
        channelTrigParamConfig: []
    }
}

const columns: Column<any>[] = [
    {
        key: 'channelId',
        title: 'Channel ID',
        dataKey: 'channelId',
        width: 100,
        align: 'center',
        cellRenderer: ({ rowData }) => {
            return h('input', {
                type: 'number',
                value: rowData.channelId,
                min: 0,
                disabled:lock.value,
                max: 7,
                'onInput': (event) => rowData.channelId = Number(event.target.value)
            })
        }
    },
    {
        key: 'channelVal0MatchTrigEn',
        title: 'Val0 Trigger Enable',
        dataKey: 'channelVal0MatchTrigEn',
        width: 200,
        align: 'center',
        cellRenderer: ({ rowData }) => {
            return h('input', {
                type: 'checkbox',
                disabled:lock.value,
                checked: rowData.channelVal0MatchTrigEn,
                'onInput': (event) => {
                    rowData.channelVal0MatchTrigEn = event.target.checked
                }
            })
        }
    },
    {
        key: 'channelVal1MatchTrigEn',
        title: 'Val1 Trigger Enable',
        dataKey: 'channelVal1MatchTrigEn',
        width: 200,
        align: 'center',
        cellRenderer: ({ rowData }) => {
            return h('input', {
                type: 'checkbox',
                disabled:lock.value,
                checked: rowData.channelVal1MatchTrigEn,
                'onInput': (event) => {
                    rowData.channelVal1MatchTrigEn = event.target.checked
                }
            })
        }
    },
    {
        width: 100,
        align: 'center',
        fixed: 'right',
        headerCellRenderer: () => {
            return h(ElButton, {
                size: 'small',
                type: "primary",
                plain: true,
                disabled:lock.value,
                icon: Plus,
                'onClick': () => addFilter(),
            })
        },
        cellRenderer: ({ rowData, rowIndex }) => {

            return h(ElButton, {
                size: 'small',
                type: "danger",
                plain: true,
                disabled:lock.value,
                'onClick': () => deleteFilter(rowIndex, rowData),
            }, () => 'Delete')
        }
    },
]
const filterError = ref('')
function deleteFilter(index, rowData) {
    data.value[activeMasterInst.value].outputTrigConfig.channelTrigParamConfig.splice(index, 1)
}

function addFilter() {
    data.value[activeMasterInst.value].outputTrigConfig.channelTrigParamConfig.push(JSON.parse(JSON.stringify({
        channelId: 0,
        channelVal0MatchTrigEn: false,
        channelVal1MatchTrigEn: false
    })))
}
const plugin = inject("plugin") as any;
const props = defineProps<{
    data: any;
    lock: boolean
}>();
const data = toRef(props, 'data')
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

const regSyncFreqRule: FormItemRule[] = [{
    type: 'number', min: 1, max: 256, required: true, message: 'Please input register loading frequency', trigger: 'change'
}]

const outputTrigWidthRule: FormItemRule[] = [
    {
        type: 'number', min: 0, max: 255, required: true, message: 'Please input output trigger width', trigger: 'change'
    }
]

const pwmOutputChannelRule: FormItemRule[] = [
    {
        type: 'number', min: 0, max: 255, required: true, message: 'Please input PWM output channel', trigger: 'change'
    }
]

const outputTrigFreqRule: FormItemRule[] = [
    {
        type: 'number', min: 1, max: 256, required: true, message: 'Please input output trigger frequency', trigger: 'change'
    }
]

const MasterRules = reactive<FormRules>({
    name: [
        { required: true, message: 'Please input name', trigger: 'change' },
        { validator: masterNameCheck, trigger: 'change' }
    ],
    etmrPrescaler:[
      { type: 'number', min: 1, max: 128, message: 'range from 1-128', trigger: 'change' },
      { type: 'number', required: true, message: 'Please input etmr Prescaler', trigger: 'change' },
    ],
    syncMethod: {
        type: 'object',
        required: true,
        fields: {
            regSyncFreq: regSyncFreqRule
        }
    },
    outputTrigConfig: {
        type: 'object',
        required: true,
        fields: {
            pwmOutputChannel: pwmOutputChannelRule,
            outputTrigWidth: outputTrigWidthRule,
            outputTrigFreq: outputTrigFreqRule,
        }

    }
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
    FEATURE_eTMR_HAS_MID.value=plugin.getMeta('FEATURE_eTMR_HAS_MID')
    FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH.value=plugin.getMeta('FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH')
    FEATURE_eTMR_HAS_INIT_REG.value=plugin.getMeta('FEATURE_eTMR_HAS_INIT_REG')
    FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE.value=plugin.getMeta('FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE')
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
    height: 300px;
    border-radius: 4px;
    border: 1px solid var(--el-border-color);
}
</style>