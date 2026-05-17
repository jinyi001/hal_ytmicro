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
              :disabled="lock"
              ref="masterRef"
              :model="data[activeMasterInst]"
              label-width="150px"
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
                label="Baudrate"
                prop="baudrate"
              >
                <el-input v-model.number="data[activeMasterInst].baudrate" />
              </el-form-item>
              <el-form-item
                label="Role choose"
                prop="nodeFunction"
              >
                <el-select
                  v-model="data[activeMasterInst].nodeFunction"
                  style="width:100%"
                  @change="roleChangeHandle"
                >
                  <el-option
                    :value="true"
                    label="Master"
                  />
                  <el-option
                    :value="false"
                    label="Slave"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Auto baud enable"
                prop="autobaudEnable"
              >
                <el-checkbox
                  v-model="data[activeMasterInst].autobaudEnable"
                  :disabled="data[activeMasterInst].nodeFunction"
                />
              </el-form-item>
              <el-form-item
                label="Break length"
                prop="breakLength"
              >
                <el-select
                  v-model="data[activeMasterInst].breakLength"
                  style="width:100%"
                >
                  <el-option
                    v-for="b in breakOpt"
                    :key="b"
                    :value="b"
                    :label="b"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Timeout check"
                prop="timeoutEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].timeoutEnable" />
              </el-form-item>
              <el-form-item
                label="Response timeout value"
                prop="responseTimeoutValue"
              >
                <el-input v-model.number="data[activeMasterInst].responseTimeoutValue" />
              </el-form-item>
              <el-form-item
                label="Header timeout value"
                prop="headerTimeoutValue"
              >
                <el-input v-model.number="data[activeMasterInst].headerTimeoutValue" />
              </el-form-item>
              <el-form-item
                label="Filter config"
                prop="filter"
              >
                <div class="table">
                  <el-auto-resizer>
                    <template #default="{ height, width }">
                      <el-table-v2
                        :columns="columns"
                        :data="data[activeMasterInst].filter"
                        :estimated-row-height="50"
                        :width="width"
                        :header-height="30"
                        :height="height"
                        :footer-height="40"
                        fixed
                        class="table"
                      >
                        <template #empty>
                          <el-empty :image-size="80" />
                        </template>
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
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick, h } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened,
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules } from 'element-plus'
import { ElButton, ElIcon } from 'element-plus'
import Schema from 'async-validator';
import type { Column } from '@element-plus/components/table-v2'

Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const columns: Column<any>[] = [
  {
    key: 'filterEnable',
    title: 'Filter Enable',
    dataKey: 'filterEnable',
    width: 110,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'checkbox',
        disabled:lock.value,
        checked: rowData.filterEnable,
        'onInput': (event) => {
          rowData.filterEnable = event.target.checked
        }
      })
  },
  {
    key: 'maskEnable',
    title: 'Mask Enable',
    dataKey: 'maskEnable',
    width: 110,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'checkbox',
        disabled:lock.value,
        checked: rowData.maskEnable,
        'onInput': (event) => {
          rowData.maskEnable = event.target.checked
        }
      })
  },

  {
    key: 'direction',
    title: 'Direction',
    dataKey: 'direction',
    width: 130,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled:lock.value,
        value: rowData.direction,
        'onChange': (event) => rowData.direction = event.target.value,
        style: { width: '100%' }
      },
        [h('option', {
          label: 'LINFlexD_RX',
          value: 'LINFlexD_RX'
        }),
        h('option', {
          label: 'LINFlexD_TX',
          value: 'LINFlexD_TX'
        })
        ])
    }
  },
  {
    key: 'checksumType',
    title: 'Checksum',
    dataKey: 'checksumType',
    width: 130,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled:lock.value,
        value: rowData.checksumType,
        'onChange': (event) => rowData.checksumType = event.target.value,
        style: { width: '100%' }
      },
        [h('option', {
          label: 'ENHANCED',
          value: 'LIN_ENHANCED_CHECKSUM'
        }),
        h('option', {
          label: 'CLASSIC',
          value: 'LIN_CLASSIC_CHECKSUM'
        })
        ])
    }
  },
  {
    key: 'id',
    title: 'ID',
    dataKey: 'id',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'number',
        min: 0,
        max: 255,
        value: rowData.id,
        disabled:lock.value,
        'onInput': (event) => {
          rowData.id = Number(event.target.value)
        }
      })
  },
  {
    key: 'dataFieldLength',
    title: 'Data length',
    dataKey: 'dataFieldLength',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'number',
        min: 0,
        max: 8,
        disabled:lock.value,
        value: rowData.dataFieldLength,
        'onInput': (event) => {
          rowData.dataFieldLength = Number(event.target.value)
        }
      })
  },
  {
    key: 'data',
    title: 'Data',
    dataKey: 'data',
    width: 500,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('div',
        Array.from(Array(rowData.dataFieldLength).keys()).map((i) => {
          return h('input', {
            type: 'number',
            min: 0,
            max: 255,
            disabled:lock.value,
            style: { width: '50px' },
            value: rowData.data[i],
            'onInput': (event) => {
              rowData.data[i] = Number(event.target.value)
            }
          })
        })
      )

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
        disabled:lock.value,
        plain: true,
        'onClick': () => deleteFilter(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]

function deleteFilter(index, rowData) {
  data.value[activeMasterInst.value].filter.splice(index, 1)
}

function addFilter() {
  data.value[activeMasterInst.value].filter.push(JSON.parse(JSON.stringify(defaultFilter)))
}

const defaultFilter = {
  filterEnable: false,
  maskEnable: false,
  dataFieldLength: 1,
  direction: 'LINFlexD_RX',
  checksumType: 'LIN_CLASSIC_CHECKSUM',
  id: 0,
  data: [
    0
  ],
  error: false,
}

const breakOpt = [
  'LINFlexD_BREAK_10_BIT',    /*!< LIN break 10 bits */
  'LINFlexD_BREAK_11_BIT',    /*!< LIN break 11 bits */
  'LINFlexD_BREAK_12_BIT',    /*!< LIN break 12 bits */
  'LINFlexD_BREAK_13_BIT',    /*!< LIN break 13 bits */
  'LINFlexD_BREAK_14_BIT',    /*!< LIN break 14 bits */
  'LINFlexD_BREAK_15_BIT',    /*!< LIN break 15 bits */
  'LINFlexD_BREAK_16_BIT',    /*!< LIN break 16 bits */
  'LINFlexD_BREAK_17_BIT',    /*!< LIN break 17 bits */
  'LINFlexD_BREAK_18_BIT',    /*!< LIN break 18 bits */
  'LINFlexD_BREAK_19_BIT',    /*!< LIN break 19 bits */
  'LINFlexD_BREAK_20_BIT',    /*!< LIN break 20 bits */
  'LINFlexD_BREAK_21_BIT',    /*!< LIN break 21 bits */
  'LINFlexD_BREAK_22_BIT',    /*!< LIN break 22 bits */
  'LINFlexD_BREAK_23_BIT',    /*!< LIN break 23 bits */
  'LINFlexD_BREAK_36_BIT',    /*!< LIN break 36 bits */
  'LINFlexD_BREAK_50_BIT',    /*!< LIN break 50 bits */
]

const defMasterParam = {
  name: 'linflexd_lin_config0',
  baudrate: 19200,
  breakLength: 'LINFlexD_BREAK_10_BIT',
  nodeFunction: false,
  autobaudEnable: false,
  timeoutEnable: false,
  responseTimeoutValue: 0,
  headerTimeoutValue: 0,
  filter: [

  ]
}

function roleChangeHandle(val) {
  if (val) {
    data.value[activeMasterInst.value].autobaudEnable = false
  }
}


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
    if (cnt[data.value[i].name] == undefined) {
      cnt[data.value[i].name] = 1
    } else {
      cnt[data.value[i].name]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      callback(new Error(`Duplicate ${key} name`))
    }
  }

  if (!err) {
    callback()
  }
}


// function pidCal() {
//   const pidList: number[] = []
//   for (let i = 0; i <= 0x3f; i++) {
//     let ID = [0, 0, 0, 0, 0, 0]
//     for (let mask = 0; mask < 6; mask++)
//       ID[mask] = (i & (1 << mask)) >> mask;

//     const p0 = (ID[0] + ID[1] + ID[2] + ID[4]) % 2;
//     const p1 = ((ID[1] + ID[3] + ID[4] + ID[5]) % 2) ? 0 : 1
//     console.log(p0, p1)
//     pidList.push(p0 * 64 + p1 * 128 + i)
//   }
//   return pidList
// }
const pidList = [128, 193, 66, 3, 196, 133, 6, 71, 8, 73, 202, 139, 76, 13, 142, 207, 80, 17, 146, 211, 20, 85, 214, 151, 216, 153, 26, 91, 156, 221, 94, 31, 32, 97, 226, 163, 100, 37, 166, 231, 168, 233, 106, 43, 236, 173, 46, 111, 240, 177, 50, 115, 180, 245, 118, 55, 120, 57, 186, 251, 60, 125, 254, 191]

const filterError = ref('')
function filterCheck(rule: any, value: any, callback: any) {
  filterError.value = ''
  for (const i in value) {
    if ((value[i].dataFieldLength > 8) || (value[i].dataFieldLength < 0)) {
      filterError.value = `filter ${i} data length should from 0-8`
      callback(new Error(filterError.value))
    }
    if (value[i].dataFieldLength > value[i].data.length) {
      filterError.value = `filter ${i} need input data`
      callback(new Error(filterError.value))

    }
    for (const d of value[i].data) {
      const n = Number(d)
      if (n > 255 || n < 0 || Number.isNaN(d)) {
        filterError.value = `filter ${i} data from 0-255`
        callback(new Error(filterError.value))
      }
    }
    if ((Number(value[i].id))>0x3f|| Number(value[i].id)<0){
      filterError.value = `filter ${i} ID error, from 0-0x3f`
      callback(new Error(filterError.value))
    }

  }


}

const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  baudrate: [
    { type: 'number', max: 19200, message: 'baudrate from 0-19200bps', trigger: 'change' },
    { type: 'number', required: true, message: 'Please input baudrate', trigger: 'change' },
  ],
  responseTimeoutValue: [
    { type: 'number', min: 0, max: 15, message: 'timeout from 0-15', trigger: 'change' },
    { type: 'number', required: true, message: 'Please input response timeout', trigger: 'change' },
  ],
  headerTimeoutValue: [
    { type: 'number', min: 0, max: 127, message: 'timeout from 0-127', trigger: 'change' },
    { type: 'number', required: true, message: 'Please input header timeout', trigger: 'change' },
  ],
  filter: {
    type: 'array', validator: filterCheck, trigger: 'change'
  },
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

.table {
  height: 200px;
  width: v-bind(winW*0.75-160+'px');
}

.bg-red {
  background-color: red;
}
</style>