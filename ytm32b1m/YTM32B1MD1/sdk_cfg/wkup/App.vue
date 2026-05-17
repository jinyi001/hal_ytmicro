<template>
  <div>
    <div>
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
                    :disabled="lock"
                    :icon="Plus"
                    size="small"
                    @click="addItem('master')"
                  />
                  <el-button
                    type="primary"
                    plain
                    :disabled="lock"
                    :icon="Delete"
                    size="small"
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
                </span><span v-if="scope.row.resetName">{{ scope.row.resetName }}</span><span v-else>{{ scope.$index }}</span>
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
              label-width="200px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
            >
              <el-form-item
                label="Reset Pin Name"
                prop="resetName"
              >
                <el-input v-model="data[activeMasterInst].resetName" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>

              <el-form-item
                label="Reset wakeup"
                prop="wakeupEn"
              >
                <el-tooltip
                  content="Reset wakeup request enable"
                  :show-after="1000"
                >
                  <el-checkbox v-model="data[activeMasterInst].wakeupEn" />
                </el-tooltip>
              </el-form-item>
              <el-form-item
                label="Reset pin filter"
                prop="filterEn"
              >
                <el-tooltip
                  content="Reset pin wakeup filter enable"
                  :show-after="1000"
                >
                  <el-checkbox
                    v-model="data[activeMasterInst].filterEn"
                    @change="filterEnNotify"
                  />
                </el-tooltip>
              </el-form-item>
              <el-form-item
                label="Enable internal wake"
                prop="interWake"
              >
                <el-checkbox v-model="data[activeMasterInst].interWake" />
              </el-form-item>
              <el-form-item
                label="Internal wake source"
                prop="moduleId"
              >
                <el-select
                  v-model.number="data[activeMasterInst].moduleId"
                  style="width:100%"
                  :disabled="!data[activeMasterInst].interWake"
                >
                  <el-option
                    :value="0"
                    label="RTC_IRQ"
                  />
                  <el-option
                    :value="1"
                    label="RTC_Seconds_IRQ"
                  />
                  <el-option
                    :value="2"
                    label="lpTMR0_IRQ"
                  />
                  <el-option
                    :value="3"
                    label="ACMP0_IRQ"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Filter clock"
                prop="filterClkSrc"
                v-show="FEATURE_WKU_FILTER_CLK_SRC"
              >
                <el-tooltip
                  content="Reset pin wakeup filter clock source"
                  :show-after="1000"
                >
                  <el-select
                    v-model.number="data[activeMasterInst].filterClkSrc"
                    style="width:100%"
                  >
                    <el-option
                      value="WKU_FILTER_CLK_SIRC"
                      label="SIRC"
                    />
                    <el-option
                      value="WKU_FILTER_CLK_SXOSC"
                      label="SXOSC"
                    />
                  </el-select>
                </el-tooltip>
              </el-form-item>

              <el-divider content-position="left">
                Wakeup pins configuration
              </el-divider>
              <el-form-item
                label="Wakeup Pins Name"
                prop="pinsName"
              >
                <el-input v-model="data[activeMasterInst].pinsName" />
              </el-form-item>
              <el-form-item
                label-width="0"
                prop="wkupPins"
              >
                <el-table-v2
                  :columns="columns"
                  :data="data[activeMasterInst].wkupPins"
                  :estimated-row-height="30"
                  :width="winW * 0.75 - 5"
                  :header-height="50"
                  :height="200"
                  fixed
                  :footer-height="40"
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
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, h, reactive, watch, onBeforeUnmount, nextTick, toRaw } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { FormInstance, FormRules, FormItemRule, ElNotification } from 'element-plus'
import Schema from 'async-validator';
import { ElButton } from 'element-plus'
import type { Column } from '@element-plus/components/table-v2'

Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()

const defMasterParam: any = {
  resetName: 'wkup_resetConfig0',
  pinsName:'wkup_pinsConfig0',
  readonly: true,
  wakeupEn: true,
  filterEn: false,
  filterClkSrc: 'WKU_FILTER_CLK_SIRC',
  wkupPins: [],
}
const filterError = ref('')
function deleteFilter(index, rowData) {
  data.value[activeMasterInst.value].wkupPins.splice(index, 1)
}

function filterEnNotify(val){
  if(val){
    ElNotification.warning('Please make sure filter clock is enable in low power mode.')
  }
}

function addFilter() {
  data.value[activeMasterInst.value].wkupPins.push(JSON.parse(JSON.stringify({
    hwChannel: '',
    wakeupEn: true,
    interWake: false,
    moduleId:0,
    edgeEvent: 'WKU_EDGE_RISING',
    filterEn: false,
    filterClkSrc: 'WKU_FILTER_CLK_SIRC'
  })))
}
const wkuppins = ref(['PTE_11', 'PTE_10', 'PTE_5', 'PTE_4', 'PTE_3', 'PTE_1', 'PTE_0', 'PTE_9', 'PTE_8', 'PTC_3', 'PTC_2', 'PTD_5', 'PTC_1', 'PTC_0', 'PTC_15', 'PTC_14', 'PTB_1', 'PTB_0', 'PTA_7', 'PTA_6', 'PTB_13', 'PTB_12', 'PTD_3', 'PTD_2', 'PTA_3', 'PTA_2', 'PTC_7', 'PTC_6', 'PTE_6', 'PTE_2', 'PTA_13', 'PTA_12'])
const edges = ['WKU_EDGE_NONE', 'WKU_EDGE_RISING', 'WKU_EDGE_FALLING', 'WKU_EDGE_BOTH']
const columns: Column<any>[] = reactive([
  {
    key: 'column-n-1',
    width: 100,
    title: 'Row No.',
    cellRenderer: ({ rowIndex }) => `${rowIndex + 1}`,
    align: 'center',
  },
  {
    key: 'hwChannel',
    title: 'Channel',
    dataKey: 'hwChannel',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled:lock.value,
        value: rowData.hwChannel,
        'onChange': (event) => rowData.hwChannel = event.target.value,
        style: { width: '100%' }
      }, wkuppins.value.map((i, index) => {
        return h('option', {
          label: i,
          value: index
        })
      }))
    }
  },
  {
    key: 'wakeupEn',
    title: 'Enable',
    dataKey: 'wakeupEn',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'checkbox',
        disabled:lock.value,
        checked: rowData.wakeupEn,
        'onInput': (event) => {
          rowData.wakeupEn = event.target.checked
        }
      })
    }
  },
  {
    key: 'edgeEvent',
    title: 'Edge event',
    dataKey: 'edgeEvent',
    width: 180,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        value: rowData.edgeEvent,
        disabled:lock.value,
        'onChange': (event) => rowData.edgeEvent = event.target.value,
        style: { width: '100%' }
      }, edges.map((i) => {
        return h('option', {
          label: i,
          value: i
        })
      }))
    }
  },
  {
    key: 'filterEn',
    title: 'Filter enable',
    dataKey: 'filterEn',
    width: 120,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'checkbox',
        disabled:lock.value,
        checked: rowData.filterEn,
        'onInput': (event) => {
          rowData.filterEn = event.target.checked
        }
      })
    }
  },
  {
    key: 'filterClkSrc',
    title: 'Filter clock source',
    dataKey: 'filterClkSrc',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled:lock.value,
        value: rowData.filterClkSrc,
        'onChange': (event) => rowData.filterClkSrc = event.target.value,
        style: { width: '100%' }
      }, [{
        value:'WKU_FILTER_CLK_SIRC',
        label:'SIRC'
      },{
        value:'WKU_FILTER_CLK_SXOSC',
        label:'SXOSC'
      }].map((i) => {
        return h('option', {
          label: i.label,
          value: i.value
        })
      }))
    }
  },
  {
    width: 100,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElButton, {
        size: 'small',
        disabled:lock.value,
        type: "primary",
        plain: true,
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
)

const FEATURE_WKU_FILTER_CLK_SRC=ref(false)
const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref([defMasterParam])
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
    data.value.push(JSON.parse(JSON.stringify(defMasterParam)))
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
function pinsCheck(rule: any, value: any, callback: any) {
  filterError.value=''
  /* hwChannel check*/
  const hwChannelArray:any[]=[]
  for(const i in value){
    if(value[i].hwChannel==''){
      filterError.value=`Wakeup pin index${i} channel is empty`
      callback(new Error(filterError.value))
    }else{
      hwChannelArray.push(value[i].hwChannel)
    }
  }
  if(hwChannelArray.length!=[...new Set(hwChannelArray)].length){
    filterError.value=`Duplicate wakeup pin config`
    callback(new Error(filterError.value))
  }
  callback()
}

function wakeupEnCheck(rule: any, value: any, callback: any) {
  if(!value){
    plugin.addLog('warn','wkupEn',"Please enable it,otherwise chip can't be wakeup by reset from debugger")
  }else{
    plugin.deleteLog('warn','wkupEn');
  }
  callback()
}
const MasterRules = reactive<FormRules>({
  resetName: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  pinsName: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  wakeupEn:[
    { type:'boolean',validator: wakeupEnCheck, trigger: 'change' }
  ],
  wkupPins:[
  { validator: pinsCheck, trigger: 'change' }
  ]
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
          const id = `${errors[j].field}-${i}-${j}`
          plugin.addLog('error', id, errors[j].message)
          errorid.push(id)
        }
      }

    });
  }
}

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      FEATURE_WKU_FILTER_CLK_SRC.value=plugin.getMeta('FEATURE_WKU_FILTER_CLK_SRC')
      wkuppins.value = plugin.getMeta("wkuppins")
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }

      if (!FEATURE_WKU_FILTER_CLK_SRC.value) {
        const index=columns.findIndex((i)=>i.key=='filterClkSrc')
        if (index>=0) {
          columns.splice(index, 1)
        }
      }

      nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
          null
        })
      })
      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(data.value);
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(data.value);
      }, {
        deep: true
      })

    })
    .catch((e: any) => {
      console.error(e);
    });


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
<style>
.el-table .active-row {
  color: #409EFF
}
</style>