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
                  v-if="scope.$index==activeMasterInst"
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
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>
              <el-form-item
                label="Clock source"
                prop="clockSource"
                v-show="ewdgClkShow"
              >
                <el-select
                  v-model="data[activeMasterInst].clockSource"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ewdgClockSource"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Assert logic"
                prop="assertLogic"
              >
                <el-select
                  v-model="data[activeMasterInst].assertLogic"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['EWDG_IN_ASSERT_DISABLED', 'EWDG_IN_ASSERT_ON_LOGIC_ZERO','EWDG_IN_ASSERT_ON_LOGIC_ONE']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>

              <el-form-item
                label="Enable interrupt"
                prop="interruptEnable"
              >
                <el-checkbox
                  v-model="data[activeMasterInst].interruptEnable"
                />
              </el-form-item>
              <el-form-item
                label="Clock divider"
                prop="prescaler"
              >
                <el-input
                  v-model.number="data[activeMasterInst].prescaler"
                />
              </el-form-item>
              <el-form-item
                label="Compare low value"
                prop="compareLow"
              >
                <el-input
                  v-model.number="data[activeMasterInst].compareLow"
                />
              </el-form-item>
              <el-form-item
                label="Compare high value"
                prop="compareHigh"
              >
                <el-input
                  v-model.number="data[activeMasterInst].compareHigh"
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
import { onMounted, ref, computed, reactive, watch, toRaw, onBeforeUnmount, nextTick, isProxy } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemProp, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const ewdgClockSource = ref([])
const ewdgClkShow = ref(Boolean)

const defMasterParam = {
  name: 'ewdg_config0',
  readonly: true,
  clockSource: 'EWDG_SIRC_CLOCK',
  assertLogic:'EWDG_IN_ASSERT_DISABLED',
  interruptEnable: false,
  prescaler: 0,
  compareLow: 0,
  compareHigh: 0
}



const lock=ref(false)
const ewdgMaxCompare = ref(0)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref([defMasterParam])

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

function compHighCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  if (value > ewdgMaxCompare.value) {
    callback(new Error(`high value should less than ${ewdgMaxCompare.value}`))
  }
  if (value < val.compareLow) {
    callback(new Error('high value should > low'))
  }
  callback()
}
const compHighRule: FormItemRule[] =
  [{ type: 'number', required: true, message: 'Please input compare high value', trigger: 'change' },
  { type: 'number', validator: compHighCheck, }
]

function compLowCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  if (value >= val.compareHigh) { 
    callback(new Error('low value should < high'))
  }
  if (value < 0) {
    callback(new Error('low value should > 0'))
  }
  callback()
}
const compLowRule: FormItemRule[] =
  [{ type: 'number', required: true, message: 'Please input compare low value', trigger: 'change' },
  { type: 'number', validator: compLowCheck, }
]

const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  prescaler: [
    { type: 'number', required: true, message: 'Please input prescaler', trigger: 'change' },
    // { type: 'number', min:0, max:ewdgMaxCompare.value,message: `Prescaler from 0-${ewdgMaxCompare.value}`, trigger: 'change'}
  ],
  compareHigh:compHighRule,
  compareLow:compLowRule
})



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
      /* Init eWDG clock source */
      ewdgClockSource.value = plugin.getMeta("ewdg_clk")
      ewdgClkShow.value = plugin.getMeta("ewdg_clk_sel_en")
      /* Get max compare high */
      ewdgMaxCompare.value = plugin.getMeta("ewdg_max_cmph")
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