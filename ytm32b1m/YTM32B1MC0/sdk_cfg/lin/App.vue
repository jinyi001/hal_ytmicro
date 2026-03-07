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
                label="Read-only"
                prop="readonly"
                v-show="false"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
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
                label="Get time interval callback"
                prop="timerGetTimeIntervalCallback"
              >
                <el-input
                  v-model="data[activeMasterInst].timerGetTimeIntervalCallback"
                />
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
// const pidList = [128, 193, 66, 3, 196, 133, 6, 71, 8, 73, 202, 139, 76, 13, 142, 207, 80, 17, 146, 211, 20, 85, 214, 151, 216, 153, 26, 91, 156, 221, 94, 31, 32, 97, 226, 163, 100, 37, 166, 231, 168, 233, 106, 43, 236, 173, 46, 111, 240, 177, 50, 115, 180, 245, 118, 55, 120, 57, 186, 251, 60, 125, 254, 191]
const pidList = ['0x3C', '0x3D']

const defMasterParam = {
  name: 'lin_config0',
  readonly: false,
  baudrate: 19200,
  nodeFunction: false,
  autobaudEnable: false,
  timerGetTimeIntervalCallback: 'NULL',
  pid:pidList
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
  let err = false;
  let cnt = {}
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






const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  baudrate: [
    { type: 'number', max: 19200, message: 'baudrate from 0-19200bps', trigger: 'change' },
    { type: 'number', required: true, message: 'Please input baudrate', trigger: 'change' },
  ],
})



let errorid: string[] = []
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