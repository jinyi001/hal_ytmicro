<template>
  <div>
    <div>
      <el-form
        ref="masterRef"
        :model="data"
        label-width="250px"
        size="small"
        :rules="MasterRules"
        hide-required-asterisk
        :disabled="lock"
      >
        <el-form-item
          label="Name"
          prop="name"
        >
          <el-input v-model="data.name" />
        </el-form-item>
        <el-form-item
          label="Read-only"
          prop="readonly"
        >
          <el-checkbox v-model="data.readonly" />
        </el-form-item>
        <el-form-item
          label="Erase function name"
          prop="acEraseFunPtr"
        >
          <el-input v-model="data.acEraseFunPtr" />
        </el-form-item>
        <el-form-item
          label="Write function name"
          prop="acWriteFunPtr"
        >
          <el-input v-model="data.acWriteFunPtr" />
        </el-form-item>
        <el-form-item
          label="End notify function name"
          prop="jobEndNotificationFunPtr"
        >
          <el-input v-model="data.jobEndNotificationFunPtr" />
        </el-form-item>
        <el-form-item
          label="Error notify function name"
          prop="jobErrorNotificationFunPtr"
        >
          <el-input v-model="data.jobErrorNotificationFunPtr" />
        </el-form-item>
        <el-form-item
          label="Work mode"
          prop="eDefaultMode"
        >
          <el-select
            v-model="data.eDefaultMode"
            style="width: 100%;"
          >
            <el-option
              lable="Normal"
              value="MEMIF_MODE_SLOW"
            />
            <el-option
              lable="Fast"
              value="MEMIF_MODE_FAST"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          label="Fast mode max read bytes"
          prop="maxReadFastMode"
        >
          <template #label="{ label }">
            {{ label }}<el-tooltip
              effect="dark"
              content="The maximum number of bytes to read or compare in one cycle of the flash driver's job processing function in fast mode."
            >
              <el-button
                :icon="QuestionFilled"
                circle
                text
              />
            </el-tooltip>
          </template>
          <el-input v-model.number="data.maxReadFastMode" />
        </el-form-item>
        <el-form-item
          label="Normal mode max read bytes"
          prop="maxReadNormalMode"
        >
          <template #label="{ label }">
            {{ label }}<el-tooltip
              effect="dark"
              content="The maximum number of bytes to read or compare in one cycle of the flash driver's job processing function in normal mode."
            >
              <el-button
                :icon="QuestionFilled"
                circle
                text
              />
            </el-tooltip>
          </template>
          <el-input v-model.number="data.maxReadNormalMode" />
        </el-form-item>
        <el-form-item
          label="Fast mode max write bytes"
          prop="maxWriteFastMode"
        >
          <template #label="{ label }">
            {{ label }}<el-tooltip
              effect="dark"
              content="The maximum number of bytes to write in one cycle of the flash driver's job processing function in fast mode."
            >
              <el-button
                :icon="QuestionFilled"
                circle
                text
              />
            </el-tooltip>
          </template>
          <el-input v-model.number="data.maxWriteFastMode" />
        </el-form-item>
        <el-form-item
          label="Normal mode max write bytes"
          prop="maxWriteNormalMode"
        >
          <template #label="{ label }">
            {{ label }}<el-tooltip
              effect="dark"
              content="The maximum number of bytes to write in one cycle of the flash driver's job processing function in normal mode."
            >
              <el-button
                :icon="QuestionFilled"
                circle
                text
              />
            </el-tooltip>
          </template>
          <el-input v-model.number="data.maxWriteNormalMode" />
        </el-form-item>

        <el-form-item
          label-width="0"
          prop="sectorList"
        >
          <div class="table">
            <el-auto-resizer>
              <template #default="{ height, width }">
                <el-table-v2
                  :columns="columns"
                  :data="data.sectorList"
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
      </el-form>
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
import { ElButton, ElNotification } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
import type { Column } from '@element-plus/components/table-v2'
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const col = ref([])


const columns: Column<any>[] = [
  {
    key: 'sectorId',
    title: 'Sector ID',
    dataKey: 'sectorId',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'number',
        disabled: lock.value,
        value: rowData.sectorId,
        min: 0,
        style: { width: '100%' },
        'onInput': (event) => rowData.sectorId = Number(event.target.value)
      })
    }
  },
  {
    key: 'sectorStartAddress',
    title: 'Sector Start Address',
    dataKey: 'sectorStartAddress',
    width: 170,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        disabled: lock.value,
        value: rowData.sectorStartAddress,
        style: { width: '100%' },
        'onInput': (event) => {
          rowData.sectorStartAddress = Number.isNaN(Number(event.target.value)) ? '' : '0x' + Number(event.target.value).toString(16)
        }
      })
    }
  },
  {
    key: 'sectorSize',
    title: 'Sector Size',
    dataKey: 'sectorSize',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        disabled: true,
        value: rowData.sectorSize,
        style: { width: '100%' },
        'onInput': (event) => {
          rowData.sectorSize = Number.isNaN(Number(event.target.value)) ? '' : '0x' + Number(event.target.value).toString(16)
        }
      })
    }
  },
  {
    key: 'pageSize',
    title: 'Page Size',
    dataKey: 'pageSize',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        disabled: true,
        value: rowData.pageSize,
        style: { width: '100%' },
        'onInput': (event) => {
          rowData.pageSize = Number.isNaN(Number(event.target.value)) ? '' : '0x' + Number(event.target.value).toString(16)
        }
      })
    }
  },
  {
    key: 'sectorHwStartAddress',
    title: 'Sector HW Start Address',
    dataKey: 'sectorHwStartAddress',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        disabled: lock.value,
        value: rowData.sectorHwStartAddress,
        style: { width: '100%' },
        'onInput': (event) => {
          rowData.sectorHwStartAddress = Number.isNaN(Number(event.target.value)) ? '' : '0x' + Number(event.target.value).toString(16)
        }
      })
    }
  },
  {
    key: 'asyncAccess',
    title: 'Async Access',
    dataKey: 'asyncAccess',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'checkbox',
        disabled: lock.value,
        checked: rowData.asyncAccess,
        'onInput': (event) => {
          rowData.asyncAccess = event.target.checked
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
        disabled: lock.value,
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
  data.value.sectorList.splice(index, 1)
}

const hwFlashInfo = {
  startAddress: 0,
  sectorSize: '0x400',
  pageSize: '0x8',
}

function addFilter() {
  const len = data.value.sectorList.length
  if (len == 0) {
    data.value.sectorList.push(JSON.parse(JSON.stringify({
      sectorId: 0,
      sectorStartAddress: '0x0',
      sectorSize: hwFlashInfo.sectorSize,
      pageSize: hwFlashInfo.pageSize,
      sectorHwStartAddress: '0x' + Number(hwFlashInfo.startAddress).toString(16),
      asyncAccess: true
    })))
  } else {
    const val = JSON.parse(JSON.stringify(data.value.sectorList[len - 1]))
    val.sectorId++
    val.sectorStartAddress = '0x' + (Number(val.sectorStartAddress) + Number(val.sectorSize)).toString(16)
    val.sectorHwStartAddress = '0x' + (Number(val.sectorHwStartAddress) + Number(val.sectorSize)).toString(16)
    if(len==256){
      ElNotification.warning('Max sector size is 256')
    }else{
      data.value.sectorList.push(val)
    }
    
  }

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


function sectorCheck(rule: any, value: any, callback: any) {
  filterError.value = ''
  const sectorId:any[]=[]
  for (const i in value) {
    if(value[i].sectorStartAddress==''){
      filterError.value=`Please input correct sectorStartAddress in ${i}`
      callback(filterError.value)
    }
    if(value[i].sectorSize==''){
      filterError.value=`Please input correct sectorSize in ${i}`
      callback(filterError.value)
    }
    if(value[i].pageSize==''){
      filterError.value=`Please input correct pageSize in ${i}`
      callback(filterError.value)
    }
    if(value[i].sectorHwStartAddress==''){
      filterError.value=`Please input correct sectorHwStartAddress in ${i}`
      callback(filterError.value)
    }
    sectorId.push(value[i].sectorId)
  }
  if([...new Set(sectorId)].length!=sectorId.length){
    filterError.value=`Sector ID duplicate `
      callback(filterError.value)
  }
  callback()
}

const MasterRules = reactive<FormRules>({
  acEraseFunPtr: { required: true, message: 'Please input erase function pointer or NULL', trigger: 'change' ,pattern:/^[-_a-zA-Z0-9]+$/gi},
  name: { required: true, message: 'Please input config name', trigger: 'change' },
  acWriteFunPtr: { required: true, message: 'Please input write function pointer or NULL', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
  jobEndNotificationFunPtr: { required: true, message: 'Please input function pointer or NULL', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
  jobErrorNotificationFunPtr: { required: true, message: 'Please input function pointer', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
  maxReadFastMode: [
    { type: 'number', required: true, message: 'Please input maxReadFastMode', trigger: 'change' },
    { type: 'number', min: 1, max: 1024, message: 'value from 1-1024', trigger: 'change' },],
  maxReadNormalMode: [
    { type: 'number', required: true, message: 'Please input maxReadNormalMode', trigger: 'change' },
    { type: 'number', min: 1, max: 1024, message: 'value from 1-1024', trigger: 'change' },],
  maxWriteFastMode: [
    { type: 'number', required: true, message: 'Please input maxWriteFastMode', trigger: 'change' },
    { type: 'number', min: 8, max: 1024, message: 'value from 8-1024', trigger: 'change' },],
  maxWriteNormalMode: [
    { type: 'number', required: true, message: 'Please input maxWriteNormalMode', trigger: 'change' },
    { type: 'number', min: 8, max: 1024, message: 'value from 8-1024', trigger: 'change' },],
  sectorList: {validator:sectorCheck, trigger:'change'}
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

  masterValidator.validate(data.value, (errors, fields) => {
    if (errors) {
      // validation failed, errors is an array of all errors
      // fields is an object keyed by field name with an array of
      // errors per field
      for (const j in errors) {
        const id = `GEN-${errors[j].field}-${j}`
        plugin.addLog('error', id, errors[j].message)
        errorid.push(id)
      }
    }

  });

}

const emits = defineEmits(['change'])

onMounted(() => {
  const flashInfo = plugin.getMeta('flash');
  const fee = plugin.getMeta('fee');

  for (const i in flashInfo) {
    if (flashInfo[i].name == fee.default_hw_block) {
      hwFlashInfo.startAddress = flashInfo[i].start;
      hwFlashInfo.sectorSize = "0x"+flashInfo[i].sectorSize.toString(16);
      hwFlashInfo.pageSize = flashInfo[i].programUnit;
      break;
    }
  }
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
  width: v-bind(winW+'px');
  height: 300px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}
</style>