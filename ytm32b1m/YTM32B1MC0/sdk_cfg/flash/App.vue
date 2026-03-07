<template>
  <div>
    <div>
      <el-form
        :model="data"
        label-width="200px"
        size="small"
        hide-required-asterisk
        :rules="MasterRules"
        :disabled="lock"
        ref="masterRef"
        >
        <el-form-item
          v-for=" (item) in flashInfo"
          :label= "item.name"
          :prop= "item.name"
          :key = "item.name"
        >
          <span><el-tag>Start:</el-tag>{{ item.start }}<el-tag>End:</el-tag>{{ item.end }}
            <el-tag>Size:</el-tag>
            <span v-if="item.size >= 1024">{{ item.size / 1024 }}KB</span>
            <span v-else>{{item.size}}B</span>
            <el-tag>SectorSize:</el-tag>
            <span v-if="item.sectorSize >= 1024">{{item.sectorSize / 1024}}KB</span>
            <span v-else>{{item.sectorSize}}B</span>
          </span>
        </el-form-item>

        <el-form-item
        label="Custom Addr Enable"
        prop="custAddr"
        >
        <el-checkbox v-model="data.custAddr" />
      </el-form-item>

      <div v-for=" (item, index) in data.cusInfo"
        :key="index"
      >
        <el-form-item
          :label="`Custom ${item.name} Start`"
          :prop="`custom_${item.name}_start`"
        >
          <el-input
            v-model.String="data.cusInfo[index].start"
            :disabled="!data.custAddr"
          />
        </el-form-item>
        <el-form-item
          :label="`Custom ${item.name} Size`"
          :prop="`custom_${item.name}_size`"
        >
          <el-input
            v-model="data.cusInfo[index].size"
            :disabled="!data.custAddr"
          />
        </el-form-item>
      </div>
        <div>
          <el-row>
            <el-col :span="6">
              <el-table
                :data="data.config"
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
                    </span><span v-if="scope.row.name">{{ scope.row.name }}</span><span v-else>{{ scope.$index }}</span>
                  </template>
                </el-table-column>
              </el-table>
            </el-col>
            <el-col :span="18">
              <div v-if="data.config[activeMasterInst]">
                <el-form-item
                  label="Name"
                  prop="name"
                >
                  <el-input v-model="data.config[activeMasterInst].name" />
                </el-form-item>
                <el-form-item
                  label="Read-only"
                  prop="readonly"
                >
                  <el-checkbox v-model="data.config[activeMasterInst].readonly" />
                </el-form-item>
                <el-form-item
                  label="Disable global interrupt"
                  prop="disGlobalInt"
                >
                  <el-checkbox v-model="data.config[activeMasterInst].disGlobalInt" />
                </el-form-item>
                <el-form-item
                  label="Async operation"
                  prop="async"
                >
                  <el-checkbox v-model="data.config[activeMasterInst].async" />
                </el-form-item>
                <el-form-item
                  label="Read verify"
                  prop="readVerify"
                >
                  <el-checkbox v-model="data.config[activeMasterInst].readVerify" />
                </el-form-item>
                <el-form-item
                  label="Callback"
                  prop="callback"
                >
                  <el-input v-model="data.config[activeMasterInst].callback" />
                </el-form-item>
              </div>
            </el-col>
          </el-row>
        </div>
      </el-form>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, nextTick, reactive, watch, toRaw } from "vue";
import { Plugin } from "plugin";
import Schema from 'async-validator';
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { FormRules } from "element-plus";
import { obtainAllFocusableElements } from "element-plus/es/utils";
Schema.warning = function () { null };

const masterRef = ref()

const defMasterParam={
      name:'flash_config0',
      readonly:false,
      async:false,
      disGlobalInt:false,
      readVerify:false,
      callback:'NULL',
    }

const flashInfo = ref([{
    name: "PFlash0",
    start: "0x00000000",
    end: "0x0001FFFF",
    size: 0x0001FFFF,
    programUnit: 8,
    sectorSize: 512
  }])

const activeMasterInst= ref(0)

function masterRowClick(row) {
  const index = data.value.config.indexOf(row)
  activeMasterInst.value = index
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
}


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


watch(activeMasterInst, () => {
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
})

function addItem(key: string) {

  if (key == 'master') {
    data.value.config.push(JSON.parse(JSON.stringify(defMasterParam)))
    activeMasterInst.value = data.value.config.length - 1
  }

}

function deleteItem(key: string) {
  if (data.value.config.length >= 0) {
    if (key == 'master') {
      data.value.config.splice(activeMasterInst.value, 1)
      activeMasterInst.value = data.value.config.length - 1
    }
  }
}


function getMiddleSubstring(str, start, end) {
  const regex = new RegExp(`${start}(.*?).*?${end}`, 'g');
  const match = str.match(regex);
  return match ? match[1] : '';
}

function flashStartCheck(rule: any, value: any, callback: any, source: any) {
  // const flashName = getMiddleSubstring(rule, "Custom_", "_")

  // if((Number(data.value.cusInfo.find(obj => obj.name === flashName).start))<Number(flashInfo.value[flashName].start))||(Number(value)>Number(data.value[flashName].end))){
  //   callback(`Address from ${data.value[flashName].start}-${data.value[flashName].end}`)
  // }

  callback()
}

function flashSizeCheck(rule: any, value: any, callback: any, source: any) {
  // if(data.value.custAddr){
  //   if(Number.isNaN(Number(value))){
  //     callback('Plese input correct format')
  //   }else{
  //     if((Number(data.value.cusPflash0Start)+Number(value)>Number(data.value.pflash0.end))){
  //       callback(`Address from ${data.value.pflash0.start}-${data.value.pflash0.end}`)
  //     }
  //   }
  // }
  callback()
}

const MasterRules = reactive<FormRules>({
})


const lock = ref(false)
const errorid: string[] = []
const plugin = new Plugin({
  lockChange: (val) => {
    lock.value = val;
  }
});
/*Stage and Table height define*/
const data = ref({
  custAddr: false,
  cusInfo: [
    {
      name: "PFlash0",
      start: "0x00000000",
      size: "0x0001FFFF"
    }
  ],
  config: [JSON.parse(JSON.stringify(defMasterParam))]
})

function dataCheck() {
  masterRef.value?.validate().catch(() => { null })
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
        const id = `${errors[j].field}-${j}`
        plugin.addLog('error', id, errors[j].message)
        errorid.push(id)
      }
    }

  });
}


onMounted(() => {

  plugin
    .getInitValue()
    .then((initData) => {
      flashInfo.value = plugin.getMeta('flash');
      let cusCount = 0;
      for (const i in flashInfo.value) {
        flashInfo.value[i].size = parseInt(flashInfo.value[i].end, 16) - parseInt(flashInfo.value[i].start, 16) + 1;
        if (flashInfo.value[i].name.indexOf("NVR") == -1) {
          data.value.cusInfo[cusCount] = {
            name: flashInfo.value[i].name,
            start: flashInfo.value[i].start,
            size: "0x"+flashInfo.value[i].size.toString(16),
          };

          MasterRules["custom_" + flashInfo.value[i].name + "_start"] = [
            { required: false, message: 'Please input value', trigger: 'change' },
            { validator: flashStartCheck, }
          ];
          MasterRules["custom_" + flashInfo.value[i].name + "_size"] = [
            { required: false, message: 'Please input value', trigger: 'change' },
            { validator: flashSizeCheck, }
          ];
          cusCount++;
        }
      }

      if (initData.pluginData != undefined) {
        data.value.cusInfo = initData.pluginData.data.cusInfo;
        lock.value = initData.pluginData.lock;
        data.value.config = initData.pluginData.data.config;
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
</script>
<style scoped>

</style>