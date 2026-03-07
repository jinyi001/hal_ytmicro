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
                </span><span>{{
                  'ClusterGroup-' + scope.$index
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
              label-width="150px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
            >
              <el-form-item
                label="CLuster list"
                prop="clusterList"
              >
                <div class="table">
                  <el-auto-resizer>
                    <template #default="{ height, width }">
                      <el-table-v2
                        :columns="columns"
                        :data="data[activeMasterInst].clusterList"
                        :estimated-row-height="30"
                        :width="width"
                        :header-height="30"
                        :height="height"
                        :footer-height="40"
                        fixed
                      >
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
  FolderOpened,
  QuestionFilled
} from '@element-plus/icons-vue'
import { ElButton, ElMessageBox, ElNotification, ElOption, ElSelect } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
import type { Column } from '@element-plus/components/table-v2'
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const defMasterParam = {
  ReservedSize: 0,
  clusterList: []
}

const columns: Column<any>[] = [
  {
    key: 'StartAddr',
    title: 'Start Address',
    dataKey: 'StartAddr',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'input',
        value: rowData.StartAddr,
        style: { width: '100%' },
        disabled: lock.value,
        'onInput': (event) => rowData.StartAddr = Number.isNaN(Number(event.target.value)) ? '' : '0x' + Number(event.target.value).toString(16)
      })
    }
  },
  {
    key: 'Length',
    title: 'Length',
    dataKey: 'Length',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'input',
        disabled: lock.value,
        value: rowData.Length,
        style: { width: '100%' },
        'onInput': (event) => rowData.Length = Number.isNaN(Number(event.target.value)) ? '' : '0x' + Number(event.target.value).toString(16)
      })
    }
  },
  {

    title: 'Sector',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h(ElButton, {
        size: 'small',
        type: "primary",
        disabled: lock.value,
        plain: true,
        'onClick': () => sectorChoose(rowData),
      }, () => 'Choose')
    }
  },
  {
    width: 100,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElButton, {
        size: 'small',
        disabled: lock.value,
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
        disabled: lock.value,
        plain: true,
        'onClick': () => deleteFilter(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]
const filterError = ref('')
function deleteFilter(index, rowData) {
  data.value[activeMasterInst.value].clusterList.splice(index, 1)
}

function addFilter() {
  if (data.value[activeMasterInst.value].clusterList.length == 128) {
    ElNotification.warning('Max cluster group is 128')
  } else {
    data.value[activeMasterInst.value].clusterList.push(JSON.parse(JSON.stringify({
      StartAddr: '',
      Length: ''
    })))
  }

}

function sectorChoose(rowData) {
  const index = ref('')
  ElMessageBox({
    title: 'Choose a sector',
    // Should pass a function if VNode contains dynamic props
    message: () =>
      h(ElSelect, {
        style:{width:'100%'},
        popperClass:"popperClass",
        modelValue: index.value,
        'onUpdate:modelValue': (val) => {
          index.value = val
        },
      }, () => {
        const slist: any[] = []
        for (const i in sectorList.value) {
          slist.push(
            h(ElOption, {
              value: i,
              key: sectorList.value[i].sectorId,
              label: `Sector:${i},StartAddr:${sectorList.value[i].sectorStartAddress},Size:${sectorList.value[i].sectorSize}`
            })
          )
        }
        return slist
      }),
  }).then(() => {
    if(index.value!=''){
      for(const val of data.value[activeMasterInst.value].clusterList){
        if(val.StartAddr==sectorList.value[index.value].sectorStartAddress){
          ElNotification.warning("The sector has been choosed")
          break;
        }
      }
      rowData.StartAddr = sectorList.value[index.value].sectorStartAddress
      rowData.Length = sectorList.value[index.value].sectorSize
    }
    
  }).catch(() => {
    null
  })
}


const plugin = new Plugin();
const props = defineProps<{
  data: any;
  lock: boolean;
  sector: any;
}>();
const sectorList = toRef(props, 'sector')
const lock = toRef(props, 'lock')
const data = toRef(props, 'data')
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

function clusterCheck(rule: any, value: any, callback: any) {
  filterError.value = ''
  let len=0;
  let clusterLen=0;
  for (const i in value) {
    if(value[i].StartAddr==''||Number.isNaN(Number(value[i].StartAddr))){
      filterError.value=`Please input correct start address in ${i}`
      callback(filterError.value)
    }
    if(value[i].Length==''||Number.isNaN(Number(value[i].Length))){
      filterError.value=`Please input correct Length in ${i}`
      callback(filterError.value)
    }
    
    if(len==0){
      clusterLen=Number(value[i].Length)
    }else{
      if(Number(value[i].Length)!=clusterLen){
        filterError.value=`The length of all clusters must be the same`
      }
    }
    len++
  }

  for(let i=0;i<value.length;i++){
    for(let j=i+1;j<value.length;j++){
      if(Number(value[i].StartAddr)+Number(value[i].Length)>Number(value[j].StartAddr)){
        filterError.value=`The area of cluster: ${i} and ${j} is overlapped`
        callback(filterError.value)
        break;
      }
    }
  }

  if(len<2){
    filterError.value=`One Cluster Group need at least tow clusters`
      callback(filterError.value)
  }
  callback()
}


const MasterRules = reactive<FormRules>({
  ReservedSize: [
    { type: 'number', required: true, message: 'Please input ReservedSize', trigger: 'change' },
    { type: 'number', min: 0, max: 1024, message: 'value from 0-1024', trigger: 'change' },],
  clusterList: {validator:clusterCheck, trigger:'change'}
})



const errorid: string[] = []
let logList: any = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  plugin.deleteLogs('error', logList);
  logList = [];

  const masterValidator = new Schema(MasterRules);
  for (const i in data.value) {
    masterValidator.validate(data.value[i], (errors, fields) => {
      if (errors) {
        // validation failed, errors is an array of all errors
        // fields is an object keyed by field name with an array of
        // errors per field
        for (const j in errors) {
          const id = `Cluster-${errors[j].field}-${i}-${j}`
          logList.push({
            id: id,
            val: errors[j].message
          });
        }
      }
    });
  }
  plugin.addLogs('error', logList);


}

const emits = defineEmits(['change'])
let dataCheck_handle: any = undefined;

onMounted(() => {
  window.addEventListener("resize", resizeHandler);

  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
  dataCheck();
  watch(data, () => {
    if (dataCheck_handle != undefined) {
      clearTimeout(dataCheck_handle);
    }
    dataCheck_handle = setTimeout(()=>{
      dataCheck();
      emits('change')
    }, 200)
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

.popperClass{
  height: 300px;
}
.table {
  width: v-bind(winW*0.75-60+'px');
  height: 450px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}
</style>