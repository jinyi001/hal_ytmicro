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
              label-width="250px"
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
                label="Number of input capture channels"
                prop="channelNum"
              >
                <el-input-number
                  v-model="item.channelNum"
                  :min="0"
                  :max="mpwm_channel_num"
                />
              </el-form-item>
              
              <el-form-item
                v-show="FEATURE_MPWM_HAS_COMBINATION_CAPTURE"
                label="Number of input capture channel pairs"
                prop="channelPairNum"
              >
                <el-input-number
                  v-model="item.channelPairNum"
                  :min="0"
                  :max="mpwm_channel_num/2"
                />
              </el-form-item>
              
              
              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  :title="`Input capture channels configuration ${i}`"
                  :name="'col'+i"
                  v-for="(e, i) in item.channelNum"
                  :key="i"
                >
                  <!-- channel index -->
                  <el-form-item
                    label="Hardware channel ID"
                    :prop="`inputCaptureChannelConfig[${i}].hwChannelId`"
                  >
                    <el-input v-model.number="item.inputCaptureChannelConfig[i].hwChannelId" />
                  </el-form-item>

                  <!-- clock source prescaler -->
                  <el-form-item
                    label="Clock source prescaler"
                    :prop="`inputCaptureChannelConfig[${i}].clkPrs`"
                  >
                    <el-select
                      v-model.number="item.inputCaptureChannelConfig[i].clkPrs"
                      style="width:100%"
                    >
                      <el-option
                        label="1"
                        value="MPWM_CLK_DIV_BY_1"
                      />

                      <el-option
                        label="2"
                        value="MPWM_CLK_DIV_BY_2"
                      />

                      <el-option
                        label="4"
                        value="MPWM_CLK_DIV_BY_4"
                      />

                      <el-option
                        label="8"
                        value="MPWM_CLK_DIV_BY_8"
                      />

                      <el-option
                        label="16"
                        value="MPWM_CLK_DIV_BY_16"
                      />

                      <el-option
                        label="32"
                        value="MPWM_CLK_DIV_BY_32"
                      />

                      <el-option
                        label="64"
                        value="MPWM_CLK_DIV_BY_64"
                      />

                      <el-option
                        label="128"
                        value="MPWM_CLK_DIV_BY_128"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- cnt auto clear -->
                  <el-form-item
                    label="Auto clear"
                    :prop="`inputCaptureChannelConfig[${i}].cntAutoClr`"
                  >
                    <el-checkbox v-model.number="item.inputCaptureChannelConfig[i].cntAutoClr" />
                  </el-form-item>

                  <!-- CVAL overwrite -->
                  <el-form-item
                    label="Overwrite"
                    :prop="`inputCaptureChannelConfig[${i}].overwrite`"
                  >
                    <el-checkbox v-model.number="item.inputCaptureChannelConfig[i].overwrite" />
                  </el-form-item>

                  <el-form-item
                    label="Capture edge selection"
                  >
                    <el-select
                      v-model.number="item.inputCaptureChannelConfig[i].edge"
                      style="width:100%"
                    >
                      <el-option
                        value="MPWM_CAP_DISABLE"
                        label="No edge, disable capture"
                      />
                      <el-option
                        value="MPWM_CAP_RISING_EDGE"
                        label="Rising edge"
                      />
                      <el-option
                        value="MPWM_CAP_FALLING_EDGE"
                        label="Falling edge"
                      />
                      <el-option
                        value="MPWM_CAP_BOTH_EDGES"
                        label="Dual edges"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- enable dma -->
                  <el-form-item
                    label="DMA enable"
                    :prop="`inputCaptureChannelConfig[${i}].dmaEn`"
                  >
                    <el-checkbox v-model.number="item.inputCaptureChannelConfig[i].dmaEn" />
                  </el-form-item>

                  <!-- Count mode -->
                  <el-form-item
                    label="Count mode"
                    :prop="`inputCaptureChannelConfig[${i}].countMode`"
                  >
                    <el-select
                      v-model.number="item.inputCaptureChannelConfig[i].countMode"
                      style="width:100%"
                    >
                      <el-option
                        label="Continuous mode"
                        value="MPWM_CONTINUOUS_MODE"
                      />
                      <el-option
                        label="One-shot mode"
                        value="MPWM_ONESHOT_MODE"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- debug mode -->
                  <el-form-item
                    label="Debug mode"
                    :prop="`inputCaptureChannelConfig[${i}].debugMode`"
                  >
                    <el-checkbox v-model.number="item.inputCaptureChannelConfig[i].debugMode" />
                  </el-form-item>

                  <!-- final value of channel counter -->
                  <el-form-item
                    label="Final value of channel counter"
                    :prop="`inputCaptureChannelConfig[${i}].maxCountValue`"
                  >
                    <el-input v-model.number="item.inputCaptureChannelConfig[i].maxCountValue" />
                  </el-form-item>

                  <!-- channel interrupt enable -->
                  <el-form-item
                    label="Channel interrupt enable"
                    :prop="`inputCaptureChannelConfig[${i}].intEn`"
                  >
                    <el-checkbox v-model.number="item.inputCaptureChannelConfig[i].intEn" />
                  </el-form-item>

                  <!-- Timer overflow interrupt enable -->
                  <el-form-item
                    label="Overflow interrupt enable"
                    :prop="`inputCaptureChannelConfig[${i}].timerOverflowIntEn`"
                  >
                    <el-checkbox v-model.number="item.inputCaptureChannelConfig[i].timerOverflowIntEn" />
                  </el-form-item>
                </el-collapse-item>

                <el-collapse-item
                  v-show="FEATURE_MPWM_HAS_COMBINATION_CAPTURE"
                  :title="`Input capture channel pairs configuration ${i}`"
                  :name="'col_pair'+i"
                  v-for="(e, i) in item.channelPairNum"
                  :key="i"
                >
                  <el-form-item
                    label="Hardware channel pair ID"
                    :prop="`inputCaptureChannelPairConfig[${i}].hwChannelPairId`"
                  >
                    <el-input v-model.number="item.inputCaptureChannelPairConfig[i].hwChannelPairId" />
                  </el-form-item>
                  <el-form-item
                    label="Combination enable"
                  >
                    <el-checkbox v-model.Boolean="item.inputCaptureChannelPairConfig[i].combEn" />
                  </el-form-item>
                  <el-form-item
                    label="Combination source"
                  >
                    <el-input v-model.number="item.inputCaptureChannelPairConfig[i].combSrc" />
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
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, nextTick, inject, onBeforeMount, computed } from "vue";
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
const mpwm_channel_num_div2=computed(()=>mpwm_channel_num.value/2)
const FEATURE_MPWM_HAS_COMBINATION_CAPTURE = ref(false)
const mpwm_channel_num = ref(0)
const chDef = {
  hwChannelId: 0,
  clkPrs: 'MPWM_CLK_DIV_BY_1',
  maxCountValue: 0xFFFF,
  edge: 'MPWM_CAP_DISABLE',
  cntAutoClr: false,
  overwrite: false,
  countMode: 'MPWM_CONTINUOUS_MODE',
  debugMode: false,
  dmaEn:false,
  intEn: false,
  timerOverflowIntEn: false,
}

const chPairDef = {
    hwChannelPairId: 0,
    combEn: true,
    combSrc: 'MPWM_COMB_SRC_EVEN',
}

const defMasterParam = {
    name: 'MPWM_IC_Config',
    readonly: false,
    channelNum: 1,
    inputCaptureChannelConfig: [chDef,chDef,chDef,chDef,chDef,chDef,chDef,chDef],
    channelPairNum: 0,
    inputCaptureChannelPairConfig: [chPairDef,chPairDef,chPairDef,chPairDef],
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
    inputCaptureChannelConfig: {
        type: 'object',
        required: true,
        fields: {
          channelsCallbacksParams: paramsRule,
          channelsCallbacks:callbackRule
        }

    }
})

let dataCheckIndex=0

// function hwChannelPairId0RuleCheck(rule: any, value: any, callback: any, source: any) {
//   const err = false;
//   const val = data.value[dataCheckIndex]

//   const cnt = {}
//   for (let i = 0; i < val.channelPairNum; i++) {
//     if (cnt[val.inputCaptureChannelPairConfig[i].hwChannelPairId] == undefined) {
//       cnt[val.inputCaptureChannelPairConfig[i].hwChannelPairId] = 1
//     } else {
//       cnt[val.inputCaptureChannelPairConfig[i].hwChannelPairId]++
//     }
//   }

//   for (const key of Object.keys(cnt)) {
//     if (cnt[key] > 1) {
//       callback(new Error(`Duplicate hardware channel pair ${key}`))
//     }
//   }
//   if (!err) {
//     callback()
//   }
// }

// function hwChannelId0RuleCheck(rule: any, value: any, callback: any, source: any) {
//   const err = false;
//   const val = data.value[dataCheckIndex]

//   const cnt = {}
//   for (let i = 0; i < val.channelNum; i++) {
//     if (cnt[val.inputCaptureChannelConfig[i].hwChannelId] == undefined) {
//       cnt[val.inputCaptureChannelConfig[i].hwChannelId] = 1
//     } else {
//       cnt[val.inputCaptureChannelConfig[i].hwChannelId]++
//     }
//   }

//   for (const key of Object.keys(cnt)) {
//     if (cnt[key] > 1) {
//       callback(new Error(`Duplicate hardware channel ${key}`))
//     }
//   }
//   if (!err) {
//     callback()
//   }
// }

// const hwChannelPairIdRule: FormItemRule[] | FormItemRule =
// [
//   { type: 'number', required: true, min: 0, max: mpwm_channel_num.value/2, message: 'Hardware channel pair ID from 0-3', trigger: 'change' },
//   { validator: hwChannelPairId0RuleCheck },
// ]

// const hwChannelIdRule: FormItemRule[] | FormItemRule = 
// [
//   { type: 'number', required: true, min: 0, max: mpwm_channel_num.value, message: 'Hardware channel ID from 0-7', trigger: 'change' },
//   { validator: hwChannelId0RuleCheck },
// ]


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

onBeforeMount(() => {
  FEATURE_MPWM_HAS_COMBINATION_CAPTURE.value = plugin.getMeta('FEATURE_MPWM_HAS_COMBINATION_CAPTURE')
  mpwm_channel_num.value = plugin.getMeta('mpwm_channel_num')
})
onMounted(() => {
   
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