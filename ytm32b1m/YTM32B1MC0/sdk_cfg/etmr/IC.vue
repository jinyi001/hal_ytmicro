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
          <div
            v-for="(item, index) in data"
            :key="index"
          >
            <el-form
              ref="masterRef"
              :model="item"
              label-width="280px"
              size="small"
              :disabled="lock"
              :rules="MasterRules"
              hide-required-asterisk
              v-show="index == activeMasterInst" 
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="item.name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="item.readonly" />
              </el-form-item>
              <el-form-item
                label="Number of input capture channel pairs"
                prop="numChannelPairs"
                v-show="FEATURE_eTMR_HAS_COMBINATION_CAPTURE"
              >
                <el-input-number
                  v-model="item.numChannelPairs"
                  :min="0"
                  :max="4"
                />
              </el-form-item>
              <el-form-item
                label="Number of input capture channels"
                prop="numChannels"
              >
                <el-input-number
                  v-model="item.numChannels"
                  :min="1"
                  :max="8"
                />
              </el-form-item>
              <el-form-item
                label="Maximum counter value"
                prop="countValue"
              >
                <el-input v-model.number="item.countValue" />
              </el-form-item>
              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  v-show="FEATURE_eTMR_HAS_COMBINATION_CAPTURE"
                  :title="`Input capture channel pairs configuration ${i}`"
                  :name="'col_pair'+i"
                  v-for="(e, i) in item.numChannelPairs"
                  :key="i"
                >
                  <el-form-item
                    label="Hardware channel pair ID"
                    :prop="`inputChPairConfig[${i}].hwChannelPairId`"
                    :rules="hwChannelPairIdRule"
                  >
                    <el-input v-model.number="item.inputChPairConfig[i].hwChannelPairId" />
                  </el-form-item>
                  <el-form-item
                    label="Combination enable"
                  >
                    <el-checkbox v-model.Boolean="item.inputChPairConfig[i].combEn" />
                  </el-form-item>
                  <el-form-item
                    label="Combination source"
                  >
                    <el-select
                      v-model.number="item.inputChPairConfig[i].combSrc"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_COMB_SRC_EVEN"
                        label="Even channel"
                      />
                      <el-option
                        value="eTMR_COMB_SRC_ODD"
                        label="Odd channel"
                      />
                    </el-select>
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  :title="`Input capture channels configuration ${i}`"
                  :name="'col'+i"
                  v-for="(e, i) in item.numChannels"
                  :key="i"
                >
                  <el-form-item
                    label="Hardware channel ID"
                    :prop="`inputChConfig[${i}].hwChannelId`"
                    :rules="hwChannelIdRule"
                  >
                    <el-input v-model.number="item.inputChConfig[i].hwChannelId" />
                  </el-form-item>

                  <!-- edge align -->
                  <el-form-item
                    label="Edge alignment Mode"
                  >
                    <el-select
                      v-model.number="item.inputChConfig[i].edge"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_NOT_SEL_EDGE"
                        label="Capture disable"
                      />
                      <el-option
                        value="eTMR_POS_EDGE"
                        label="Rising edge"
                      />
                      <el-option
                        value="eTMR_NEG_EDGE"
                        label="Falling edge"
                      />
                      <el-option
                        value="eTMR_DUAL_EDGES"
                        label="Dual edges"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Measurement type"
                  >
                    <el-select
                      v-model.number="item.inputChConfig[i].measurementType"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_POS_PULSE_MEASUREMENT"
                        label="Positive pulse width"
                      />
                      <el-option
                        value="eTMR_NEG_PULSE_MEASUREMENT"
                        label="Negative pulse width"
                      />
                      <el-option
                        value="eTMR_PERIOD_MEASUREMENT"
                        label="Period and duty cycle"
                      />
                    </el-select>
                  </el-form-item>

                  <el-form-item
                    label="Capture hold enable"
                    v-show="FEATURE_eTMR_HAS_CAPTURE_HOLD"
                  >
                    <el-checkbox
                      v-model.number="item.inputChConfig[i].capHoldEn" 
                    />
                  </el-form-item>

                  <el-form-item
                    label="DMA enable"
                  >
                    <el-checkbox
                      v-model="item.inputChConfig[i].dmaEnable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Filter Sample Counter"
                  >
                    <el-input
                      v-model.number="item.inputChConfig[i].filterSampleCounter"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Filter Sample Period"
                  >
                    <el-input
                      v-model.number="item.inputChConfig[i].filterSamplePeriod"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Enable interrupt"
                  >
                    <el-checkbox
                      v-model="item.inputChConfig[i].interruptEnable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Enable notification"
                  >
                    <el-checkbox
                      v-model="item.inputChConfig[i].enableNotification"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Callback"
                    :prop="`inputChConfig[${i}].channelsCallbacks`"
                    :rules="callbackRule"
                  >
                    <el-input v-model="item.inputChConfig[i].channelsCallbacks" />
                  </el-form-item>
                  <el-form-item
                    label="Callback Params"
                    :prop="`inputChConfig[${i}].channelsCallbacksParams`"
                    :rules="paramsRule"
                  >
                    <el-input
                      v-model="item.inputChConfig[i].channelsCallbacksParams"
                    />
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
    FolderOpened
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'

const activeMasterInst = ref(0);
const masterRef = ref<Record<number, FormInstance>>({})
const col = ref(['col1'])
const FEATURE_eTMR_HAS_CAPTURE_HOLD = ref(false)
const FEATURE_eTMR_HAS_COMBINATION_CAPTURE = ref(false)
const FEATURE_eTMR_HAS_HARDWARE_CAPTURE = ref(false)
const chDef = {
    hwChannelId: 0,
    edge: 'eTMR_NOT_SEL_EDGE',
    measurementType:'eTMR_POS_PULSE_MEASUREMENT',
    capHoldEn: false,
    filterSampleCounter: 0,
    filterSamplePeriod: 0,
    dmaEnable:false,
    interruptEnable: false,
    enableNotification: false,
    channelsCallbacks: 'NULL',
    channelsCallbacksParams: 'NULL'
}

const chPairDef = {
    hwChannelPairId: 0,
    combEn: false,
    combSrc: 'eTMR_COMB_SRC_EVEN',
}

const defMasterParam = {

    name: 'ETMR_IC_Config',
    readonly: false,
    numChannels: 1,
    countValue: 65535,
    inputChConfig: [chDef,chDef,chDef,chDef,chDef,chDef,chDef,chDef],
    numChannelPairs: 0,
    inputChPairConfig: [chPairDef,chPairDef,chPairDef,chPairDef],
}

const plugin = inject("plugin") as any;
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
            delete masterRef.value[activeMasterInst.value]
            activeMasterInst.value = data.value.length - 1
           
        }
    }
}

function masterRowClick(row) {
    const index = data.value.indexOf(row)
    activeMasterInst.value = index
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


const paramsRule:FormItemRule|FormItemRule[]={ required: true, message: 'Please input callback parameter', trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi}
const callbackRule:FormItemRule|FormItemRule[]={ required: true, message: 'Please input callback function', trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi}

const MasterRules = reactive<FormRules>({
    name: [
        { required: true, message: 'Please input name', trigger: 'change' },
        { validator: masterNameCheck, trigger: 'change' }
    ],
    inputChConfig: {
        type: 'object',
        required: true,
        fields: {
          channelsCallbacksParams: paramsRule,
          channelsCallbacks:callbackRule
        }

    }
})

let dataCheckIndex=0

function hwChannelPairId0RuleCheck(rule: any, value: any, callback: any, source: any) {
  const err = false;
  const val = data.value[dataCheckIndex]

  const cnt = {}
  for (let i = 0; i < val.numChannelPairs; i++) {
    if (cnt[val.inputChPairConfig[i].hwChannelPairId] == undefined) {
      cnt[val.inputChPairConfig[i].hwChannelPairId] = 1
    } else {
      cnt[val.inputChPairConfig[i].hwChannelPairId]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      callback(new Error(`Duplicate hardware channel pair ${key}`))
    }
  }
  if (!err) {
    callback()
  }
}

function hwChannelId0RuleCheck(rule: any, value: any, callback: any, source: any) {
  const err = false;
  const val = data.value[dataCheckIndex]

  const cnt = {}
  for (let i = 0; i < val.numChannels; i++) {
    if (cnt[val.inputChConfig[i].hwChannelId] == undefined) {
      cnt[val.inputChConfig[i].hwChannelId] = 1
    } else {
      cnt[val.inputChConfig[i].hwChannelId]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      callback(new Error(`Duplicate hardware channel ${key}`))
    }
  }
  if (!err) {
    callback()
  }
}

const hwChannelPairIdRule: FormItemRule[] | FormItemRule =
[
  { type: 'number', required: true, min: 0, max: 3, message: 'Hardware channel pair ID from 0-3', trigger: 'change' },
  { validator: hwChannelPairId0RuleCheck },
]

const hwChannelIdRule: FormItemRule[] | FormItemRule = 
[
  { type: 'number', required: true, min: 0, max: 7, message: 'Hardware channel ID from 0-7', trigger: 'change' },
  { validator: hwChannelId0RuleCheck },
]


const errorid: string[] = []
onBeforeUnmount(() => {
    window.removeEventListener("resize", resizeHandler);
});

async function dataCheck() {
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }

  for (const i in Object.keys(masterRef.value)) {
    dataCheckIndex=Number(i)
    try{
      await masterRef.value[i]?.validate()
    }catch(e){
      const errs =e as Record<string,any[]>
      for (const val of Object.values(errs)) {
          for (const j in val) {
            const id = `IC-${val[j].field}-${i}-${j}`
            plugin.addLog('error', id, val[j].message)
            errorid.push(id)
          }
        }
    }
    dataCheckIndex=activeMasterInst.value
  }
}

watch(activeMasterInst, () => {
  nextTick(() => {
    dataCheck()
  })
})

const emits = defineEmits(['change'])

onMounted(() => {
    FEATURE_eTMR_HAS_CAPTURE_HOLD.value = plugin.getMeta('FEATURE_eTMR_HAS_CAPTURE_HOLD')
    FEATURE_eTMR_HAS_COMBINATION_CAPTURE.value = plugin.getMeta('FEATURE_eTMR_HAS_COMBINATION_CAPTURE')
    FEATURE_eTMR_HAS_HARDWARE_CAPTURE.value = plugin.getMeta('FEATURE_eTMR_HAS_HARDWARE_CAPTURE')
    window.addEventListener("resize", resizeHandler);
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