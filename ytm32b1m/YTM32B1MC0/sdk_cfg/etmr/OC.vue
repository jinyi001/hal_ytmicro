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
          <div
            v-for="(item, index) in data"
            :key="index"
          >
            <el-form
              :ref="el => { masterRef[index] = el }"
              :model="item"
              label-width="280px"
              :disabled="lock"
              size="small"
              v-show="index == activeMasterInst"
              :rules="MasterRules"
              hide-required-asterisk
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
                label="Number of output compare channels"
                prop="nNumOutputChannels"
              >
                <el-input-number
                  v-model="item.nNumOutputChannels"
                  :min="1"
                  :max="8"
                />
              </el-form-item>
              <el-form-item
                
                label="Counter initial value from INIT register(INIT=0)"
                prop="counterInitValFromInitReg"
              >
                <el-checkbox
                  v-model.Boolean="item.counterInitValFromInitReg"
                  :disabled="FEATURE_eTMR_HAS_CNT_INIT_SRC==0"
                />
              </el-form-item>
              <el-form-item
                label="Counter initial value from CNT register"
                prop="cntVal"
              >
                <el-input
                  v-model.number="item.cntVal"
                  :disabled="item.counterInitValFromInitReg==true"
                />
              </el-form-item>
              <el-form-item
                label="Maximum counter value"
                prop="maxCountValue"
              >
                <el-input v-model.number="item.maxCountValue" />
              </el-form-item>
              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  v-for="(e, i) in item.nNumOutputChannels"
                  :key="i"
                  :title="`Output compare channels configuration ${i}`"
                  :name="'col' + i"
                >
                  <el-form-item
                    label="Hardware channel ID"
                    :rules="hwChannelIdRule"
                    :prop="`outputChannelConfig[${i}].hwChannelId`"
                  >
                    <el-input v-model.number="item.outputChannelConfig[i].hwChannelId" />
                  </el-form-item>
                  <el-form-item
                    label="Init value"
                    :prop="`outputChannelConfig[${i}].channelInitVal`"
                  >
                    <el-input v-model.number="item.outputChannelConfig[i].channelInitVal" />
                  </el-form-item>
                  <el-form-item label="Matching output mode (VAL0)">
                    <el-select
                      v-model.number="item.outputChannelConfig[i].val0CmpMode"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_OUTPUT_CLR"
                        label="Clear on match"
                      />
                      <el-option
                        value="eTMR_OUTPUT_SET"
                        label="Set on match"
                      />
                      <el-option
                        value="eTMR_OUTPUT_TOGGLE"
                        label="Toggle on match"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item label="Compared value (VAL0)">
                    <el-input v-model.number="item.outputChannelConfig[i].cmpVal0" />
                  </el-form-item>
                  <el-form-item label="Matching output mode (VAL1)">
                    <el-select
                      v-model.number="item.outputChannelConfig[i].val1CmpMode"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_OUTPUT_CLR"
                        label="Clear on match"
                      />
                      <el-option
                        value="eTMR_OUTPUT_SET"
                        label="Set on match"
                      />
                      <el-option
                        value="eTMR_OUTPUT_TOGGLE"
                        label="Toggle on match"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item label="Compared value (VAL1)">
                    <el-input v-model.number="item.outputChannelConfig[i].cmpVal1" />
                  </el-form-item>
                  <el-form-item label="External trigger (VAL0)">
                    <el-checkbox v-model="item.outputChannelConfig[i].enableExternalTrigger0" />
                  </el-form-item>
                  <el-form-item label="External trigger (VAL1)">
                    <el-checkbox v-model="item.outputChannelConfig[i].enableExternalTrigger1" />
                  </el-form-item>
                  <el-form-item label="Enable interrupt">
                    <el-checkbox v-model="item.outputChannelConfig[i].interruptEnable" />
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
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, inject } from "vue";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'

const activeMasterInst = ref(0);
const masterRef = ref<Record<number, FormInstance>>({})
const col = ref(['1'])
const plugin = inject("plugin") as any;
const chDef = {
  hwChannelId: 0,
  channelInitVal:0,
  val0CmpMode: 'eTMR_OUTPUT_CLR',
  val1CmpMode: 'eTMR_OUTPUT_CLR',
  cmpVal0: 0,
  cmpVal1: 0,
  enableExternalTrigger0: false,
  enableExternalTrigger1: false,
  interruptEnable:false
}

const defMasterParam = {

  name: 'ETMR_OC_Config',
  readonly: false,
  counterInitValFromInitReg:false,
  nNumOutputChannels: 1,
  cntVal: 0,
  maxCountValue: 0,
  outputChannelConfig: [chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef]
}

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

let dataCheckIndex=0


function hwChannelId0RuleCheck(rule: any, value: any, callback: any, source: any) {
  const err = false;
  const val = data.value[dataCheckIndex]


  const cnt = {}
  for (let i = 0; i < val.nNumOutputChannels; i++) {
    if (cnt[val.outputChannelConfig[i].hwChannelId] == undefined) {
      cnt[val.outputChannelConfig[i].hwChannelId] = 1
    } else {
      cnt[val.outputChannelConfig[i].hwChannelId]++
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


const hwChannelIdRule: FormItemRule[] | FormItemRule = 
[
  { type: 'number', required: true, min: 0, max: 7, message: 'Hardware channel ID from 0-8', trigger: 'change' },
  { validator: hwChannelId0RuleCheck },
]



const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  nNumOutputChannels: [
    { type: 'number', required: true, min: 0, max: 8, message: 'Please input a channel num 0-8', trigger: 'change' },
  ],
})



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
            const id = `OC-${val[j].field}-${i}-${j}`
            plugin.addLog('error', id, val[j].message)
            errorid.push(id)
          }
        }
    }
    dataCheckIndex=activeMasterInst.value
  }
}

const emits = defineEmits(['change'])


const FEATURE_eTMR_HAS_CNT_INIT_SRC = ref(false)
onMounted(() => {
  FEATURE_eTMR_HAS_CNT_INIT_SRC.value = plugin.getMeta('FEATURE_eTMR_HAS_CNT_INIT_SRC')
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
  width: v-bind(winW * 0.75 - 60 + 'px');
  height: 160px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}
</style>