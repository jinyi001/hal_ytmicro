<template>
  <el-divider content-position="left">
    General Configuration
  </el-divider>
  <div class="table">
    <el-auto-resizer>
      <template #default="{ height, width }">
        <el-table-v2
          :columns="usercolumns"
          :data="data.user"
          :estimated-row-height="30"
          :width="width"
          :header-height="30"
          :height="height"
          fixed
          :footer-height="40"
        >
          <template #footer>
            <el-alert
              v-if="filterError1 != ''"
              center
              show-icon
              :title="filterError1"
              type="error"
              :closable="false"
            />
          </template>
          <template #empty>
            <el-empty :image-size="80" />
          </template>
        </el-table-v2>
      </template>
    </el-auto-resizer>
  </div>
  <el-divider content-position="left">
    Callback Configuration
  </el-divider>
  <div class="table">
    <el-auto-resizer>
      <template #default="{ height, width }">
        <el-table-v2
          :columns="cbcolumns"
          :data="data.callback"
          :estimated-row-height="30"
          :width="width"
          :header-height="30"
          :height="height"
          fixed
          :footer-height="40"
        >
          <template #footer>
            <el-alert
              v-if="filterError2 != ''"
              center
              show-icon
              :title="filterError2"
              type="error"
              :closable="false"
            />
          </template>
          <template #empty>
            <el-empty :image-size="80" />
          </template>
        </el-table-v2>
      </template>
    </el-auto-resizer>
  </div>
</template>
<script setup lang="ts">
import { onMounted, onBeforeUnmount, ref, h, watch } from "vue";
import { Plugin } from "plugin";
import type { Column } from '@element-plus/components/table-v2'
import {
  Plus,
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'


interface userData{
  name:string,
  powerMode:string,
  sleepOnExitValue: boolean
}
interface callbackData{
  name:string,
  callbackFunction:string,
  callbackType: string,
  callbackData: string
}

const defData: { user: userData[], callback: callbackData[] } = {
  user: [],
  callback: []
}

const data = ref(defData)

const powerOptions = [
  h('option', {
    label: 'Run',
    value: 'POWER_MANAGER_RUN'
  }),
  h('option', {
    label: 'Sleep',
    value: 'POWER_MANAGER_SLEEP'
  }),
  h('option', {
    label: 'DeepSleep',
    value: 'POWER_MANAGER_DEEPSLEEP'
  }),
//   h('option', {
//     label: 'Standby',
//     value: 'POWER_MANAGER_STANDBY'
//   }),
];

const usercolumns: Column<any>[] = [
  {
    key: 'name',
    title: 'Name',
    dataKey: 'name',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        disabled:lock.value,
        value: rowData.name,
        'onInput': (event) => rowData.name = event.target.value
      })
  },
  {
    key: 'powerMode',
    title: 'Power Mode',
    dataKey: 'powerMode',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled:lock.value,
        value: rowData.powerMode,
        'onChange': (event) => rowData.powerMode = event.target.value,
        style: { width: '100%' }
      }, powerOptions)
    }
  },
  {
    key: 'sleepOnExitValue',
    title: 'Sleep On Exit Value',
    dataKey: 'sleepOnExitValue',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'checkbox',
        disabled:lock.value,
        checked: rowData.sleepOnExitValue,
        'onInput': (event) => {
          rowData.sleepOnExitValue = event.target.checked
        }
      })
  },
  {
    width: 100,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElButton, {
        size: 'small',
        type: "primary",
        disabled:lock.value,
        plain: true,
        icon: Plus,
        'onClick': () => addUserFilter(),
      })
    },
    cellRenderer: ({ rowData, rowIndex }) => {

      return h(ElButton, {
        size: 'small',
        type: "danger",
        plain: true,
        disabled:lock.value,
        'onClick': () => deleteUserFilter(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]
const filterError1 = ref('')
const filterError2 = ref('')
function deleteUserFilter(index, rowData) {
  data.value.user.splice(index, 1)
}

function addUserFilter() {
  data.value.user.push(JSON.parse(JSON.stringify({
    name: '',
    powerMode: 'POWER_MANAGER_RUN',
    sleepOnExitValue: false,
  })))
}

const cbcolumns: Column<any>[] = [
  {
    key: 'name',
    title: 'Name',
    dataKey: 'name',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        disabled:lock.value,
        value: rowData.name,
        'onInput': (event) => rowData.name = event.target.value
      })
  },
  {
    key: 'callbackFunction',
    title: 'Callback',
    dataKey: 'callbackFunction',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        disabled:lock.value,
        value: rowData.callbackFunction,
        'onInput': (event) => rowData.callbackFunction = event.target.value
      })
  },
  {
    key: 'callbackType',
    title: 'Callback Type',
    dataKey: 'callbackType',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled:lock.value,
        size: 'small',
        value: rowData.callbackType,
        'onChange': (event) => rowData.callbackType = event.target.value,
        style: { width: '100%' }
      },
        [h('option', {
          label: 'Before callback',
          value: 'POWER_MANAGER_CALLBACK_BEFORE'
        }),
        h('option', {
          label: 'After callback',
          value: 'POWER_MANAGER_CALLBACK_AFTER'
        }),
        h('option', {
          label: 'Before-After callback',
          value: 'POWER_MANAGER_CALLBACK_BEFORE_AFTER'
        }),


        ])
    }
  },
  {
    key: 'callbackData',
    title: 'Callback Params',
    dataKey: 'callbackData',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        disabled:lock.value,
        value: rowData.callbackData,
        'onInput': (event) => rowData.callbackData = event.target.value
      })
  },
  {
    width: 100,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElButton, {
        disabled:lock.value,
        size: 'small',
        type: "primary",
        plain: true,
        icon: Plus,
        'onClick': () => addCbFilter(),
      })
    },
    cellRenderer: ({ rowData, rowIndex }) => {

      return h(ElButton, {
        disabled:lock.value,
        size: 'small',
        type: "danger",
        plain: true,
        'onClick': () => deleteCbFilter(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]

function deleteCbFilter(index, rowData) {
  data.value.callback.splice(index, 1)
}

function dataCheck() {
  /* user */
  filterError1.value=''
  filterError2.value=''
  let name:string[]=[]
  for(const i in data.value.user){
    const val=data.value.user[i]
    /*name check*/
    plugin.deleteLog('error',`user-name-${i}`)
    if(val.name==''){
      filterError1.value=`name is empty`
      plugin.addLog('error',`user-name-${i}`,filterError1.value)
      break
    }
    if(name.indexOf(val.name)!=-1){
      filterError1.value=`name ${val.name} duplicate`
      plugin.addLog('error',`user-name-${i}`,filterError1.value)
      break
    }else{
      name.push(val.name)
    }
  }
  name=[]
  for(const i in data.value.callback){
    const val=data.value.callback[i]
    /*name check*/
    plugin.deleteLog('error',`cb-name-${i}`)
    plugin.deleteLog('error',`cb-callbackFunction-${i}`)
    plugin.deleteLog('error',`cb-callbackType-${i}`)
    plugin.deleteLog('error',`cb-callbackData-${i}`)
    if(val.name==''){
      filterError2.value=`name is empty`
      plugin.addLog('error',`cb-name-${i}`,filterError2.value)
      break
    }
    if(val.callbackFunction==''||!/^[-_a-zA-Z0-9]+$/gi.test(val.callbackFunction)){
      filterError2.value=`callbackFunction is empty`
      plugin.addLog('error',`cb-callbackFunction-${i}`,filterError2.value)
      break
    }
    if(val.callbackType==''){
      filterError2.value=`callbackType is empty`
      plugin.addLog('error',`cb-callbackType-${i}`,filterError2.value)
      break
    }
    if(val.callbackData==''||!/^[-_a-zA-Z0-9]+$/gi.test(val.callbackData)){
      filterError2.value=`callbackData is empty`
      plugin.addLog('error',`cb-callbackData-${i}`,filterError2.value)
      break
    }
    if(name.indexOf(val.name)!=-1){
      filterError2.value=`name ${val.name} duplicate`
      plugin.addLog('error',`cb-name-${i}`,filterError2.value)
      break
    }else{
      name.push(val.name)
    }
  }
}

function addCbFilter() {
  data.value.callback.push(JSON.parse(JSON.stringify({
    name: '',
    callbackFunction: 'NULL',
    callbackType: 'POWER_MANAGER_CALLBACK_BEFORE',
    callbackData: 'NULL',
  })))
}

const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const height = ref(window.innerHeight)
const width = ref(window.innerWidth)

function resizeHandler() {
  height.value = window.innerHeight
  width.value = window.innerWidth
}



onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

let powerInfo: any

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      powerInfo = plugin.getMeta('power');
      if (powerInfo.FEATURE_SOC_SUPPORT_STANDBY_MODE) {
        powerOptions.push(
          h('option', {
            label: 'Standby',
            value: 'POWER_MANAGER_STANDBY'
          }))
      }
      if (powerInfo.FEATURE_SOC_SUPPORT_POWERDOWN_MODE) {
        powerOptions.push(
          h('option', {
            label: 'PowerDown',
            value: 'POWER_MANAGER_POWERDOWN'
          }))
      }
      if (powerInfo.FEATURE_SOC_SUPPORT_DEEPPOWERDOWN_MODE) {
        powerOptions.push(
          h('option', {
            label: 'DeepPowerDown',
            value: 'POWER_MANAGER_DEEPPOWERDOWN'
          }))
      }
      
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }
      dataCheck()
      /*always push a data when init done*/
      plugin.dataChange(data.value);
      watch(data, (raw) => {
        plugin.dataChange(data.value);
        dataCheck()
      }, {
        deep: true
      })

    })
    .catch((e: any) => {
      console.error(e);
    });


});



</script>
<style>
.el-table .active-row {
  color: #409EFF,
}

.table {

  margin: 10px;
  width: v-bind(width -20 + "px");
  height: 300px;
  overflow: auto;
  text-align: center;
}
</style>