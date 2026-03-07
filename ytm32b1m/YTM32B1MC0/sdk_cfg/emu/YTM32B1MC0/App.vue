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
                </span><span v-if="scope.row.name">{{ scope.row.name }}</span><span v-else>{{ scope.$index }}</span>
              </template>
            </el-table-column>
          </el-table>
        </el-col>
        <el-col :span="18">
          <div v-if="data[activeMasterInst]">
            <el-form
              ref="masterRef"
              :model="data[activeMasterInst]"
              label-width="200px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
              :disabled="lock"
            >
              <el-form-item
                label="Name"
                prop="name"
                :rules="nameCheck"
              >
                <el-input v-model="data[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="Readonly"
                prop="readonly"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>
              <el-form-item
                label="Channel"
                prop="channel"
              >
                <el-input
                  v-model.number="data[activeMasterInst].channel"
                />
              </el-form-item>
              <el-form-item
                label="Inject address"
                prop="injectErrAddr"
              >
                <el-input v-model="data[activeMasterInst].injectErrAddr" />
              </el-form-item>
              <el-form-item
                label="Interrupt enable"
                prop="interruptEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].interruptEnable" />
              </el-form-item>
              <el-form-item
                label="Inject bits type"
                prop="injectErrSetInfo.injectBitsType"
              >
                <el-select
                  v-model="data[activeMasterInst].injectErrSetInfo.injectBitsType"
                  style="width: 100%;"
                  filterable
                >
                  <el-option
                    v-for="item in emu_inject_bits_type"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Inject data bit"
                prop="injectErrSetInfo.dataBit"
              >
                <el-select
                  v-model="data[activeMasterInst].injectErrSetInfo.dataBit"
                  style="width: 100%;"
                  filterable
                >
                  <el-option
                    v-for="item in databit"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Inject check bit"
                prop="injectErrSetInfo.chkBit"
              >
                <el-select
                  v-model="data[activeMasterInst].injectErrSetInfo.chkBit"
                  style="width: 100%;"
                  filterable
                >
                  <el-option
                    v-for="item in checkbit"
                    :key="item"
                    :value="item"
                    :label="item"
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
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick, isProxy } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { FormInstance, FormRules, FormItemRule, ElNotification } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const databit = [
  'EMU_DATABIT_0',
  'EMU_DATABIT_1',
  'EMU_DATABIT_2',
  'EMU_DATABIT_3',
  'EMU_DATABIT_4',
  'EMU_DATABIT_5',
  'EMU_DATABIT_6',
  'EMU_DATABIT_7',
  'EMU_DATABIT_8',
  'EMU_DATABIT_9',
  'EMU_DATABIT_10',
  'EMU_DATABIT_11',
  'EMU_DATABIT_12',
  'EMU_DATABIT_13',
  'EMU_DATABIT_14',
  'EMU_DATABIT_15',
  'EMU_DATABIT_16',
  'EMU_DATABIT_17',
  'EMU_DATABIT_18',
  'EMU_DATABIT_19',
  'EMU_DATABIT_20',
  'EMU_DATABIT_21',
  'EMU_DATABIT_22',
  'EMU_DATABIT_23',
  'EMU_DATABIT_24',
  'EMU_DATABIT_25',
  'EMU_DATABIT_26',
  'EMU_DATABIT_27',
  'EMU_DATABIT_28',
  'EMU_DATABIT_29',
  'EMU_DATABIT_30',
  'EMU_DATABIT_31',

]
const emu_inject_bits_type = [
  'EMU_INJECT_BITS_TYPE_DATA',
  'EMU_INJECT_BITS_TYPE_CHK',
  'EMU_INJECT_BITS_TYPE_MIXTURE'
]
const checkbit = [
  'EMU_CHKBIT_0',
  'EMU_CHKBIT_1',
  'EMU_CHKBIT_2',
  'EMU_CHKBIT_3',
  'EMU_CHKBIT_4',
  'EMU_CHKBIT_5',
  'EMU_CHKBIT_6',
]

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()



const defChannelParam = {
  readonly: true,
  name: 'EMU_Config0',
  channel: 0,
  injectErrAddr: '0x20000000',
  injectErrSetInfo: {
    injectBitsType: 'EMU_INJECT_BITS_TYPE_DATA',
    dataBit: 'EMU_DATABIT_0',
    chkBit: 'EMU_CHKBIT_0'
  },
  interruptEnable: false
}



const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref([defChannelParam])


function resizeHandler() {
  null
}

watch(activeMasterInst, () => {
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
})



function masterRowClick(row) {
  const index = data.value.indexOf(row)
  activeMasterInst.value = index
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
}

function namelValid(rule: any, value: any, callback: any) {
  const err = false;
  const cnt = {}

  for (let i = 0; i < data.value.length; i++) {
    if (cnt[data.value[i].name] == undefined) {
      cnt[data.value[i].name] = 1
    } else {
      cnt[data.value[i].name]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1 && value == key) {
      callback(new Error(`Duplicate ${key}`))
    }
  }
  callback()
}
const nameCheck: FormItemRule[] = [
  { required: true, message: 'Please input a name', trigger: 'change' },
  { validator: namelValid, trigger: 'change' }
]


function addrValid(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  const addr = Number(value)
  if (Number.isNaN(addr)) {
    callback(new Error('Please input correct format'))
  }
  if (val.channel == 0) {
    if (addr >= 0x20000000 && addr <= 0x20007FFF) {
      callback()
    } else {
      callback(new Error('Addr should from 0x2000_0000 - 0x2000_7FFF'))
    }
  }
  callback()
}




const MasterRules = reactive<FormRules>({
  name: nameCheck,
  channel: [
    { type: 'number', required: true, message: 'Please input a channel', trigger: 'change' },
    { type: 'number', min: 0, max: 0, message: 'Channel 0', trigger: 'change' }
  ],
  injectErrAddr: [
    { required: true, message: 'Please input a address', trigger: 'change' },
    { validator: addrValid, trigger: 'change' }
  ]
})

function addItem(key: string) {

  if (key == 'master') {
    if (data.value.length == 2) {
      ElNotification({
        title: 'Warning',
        message: 'Max channel number is 2',
        type: 'warning',
      })
    } else {
      data.value.push(JSON.parse(JSON.stringify(defChannelParam)))
      activeMasterInst.value = data.value.length - 1
    }

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

const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  masterRef.value?.validate().catch(()=>{null})
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  const masterValidator = new Schema(MasterRules);
  for (const i in data.value) {
    masterValidator.validate(data.value[i],(errors, fields) => {
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
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
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