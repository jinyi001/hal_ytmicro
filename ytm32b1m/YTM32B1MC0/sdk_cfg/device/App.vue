<template>
  <div>
    <div>
      <el-form
        ref="masterRef"
        :model="data"
        label-width="200px"
        size="small"
        hide-required-asterisk
        :disabled="lock"
        :rules="rules"
        :validate-on-rule-change="false"
      >
        <el-form-item
          label="Part number"
          prop="pn"
        >
          <el-input
            v-model="data.pn"
            disabled
          />
        </el-form-item>
        <el-form-item
          label="SDK version"
          prop="sdkVer"
        >
          <el-input
            v-model="data.sdkVer"
            disabled
          />
        </el-form-item>
        <el-form-item
          label="STACK TOP Symbol"
          prop="stack_top"
        >
          <el-tree-select
            v-model="data.stack_top"
            :data="sectionTreeList"
            highlight-current
            default-expand-all
            style="width: 100%"
            clearable
          >
            <template #default="{ node, data }">
              <span
                style="
                  display: flex;
                  justify-content: space-between;
                  font-size: 12px;
                "
              >
                <span>{{ node.label }}</span>
                <span style="color: var(--el-color-info)">
                  {{ data.type }}
                </span>
              </span>
            </template>
          </el-tree-select>
        </el-form-item>
        <el-form-item
          item
          label-width="0"
        >
          <el-col :span="12">
            <el-form-item
              label="VECTOR Flash Symbol Start"
              prop="vector_flash"
            >
              <el-tree-select
                v-model="data.vector_flash"
                :data="sectionTreeList"
                highlight-current
                default-expand-all
                style="width: 100%"
                clearable
              >
                <template #default="{ node, data }">
                  <span
                    style="
                      display: flex;
                      justify-content: space-between;
                      font-size: 12px;
                    "
                  >
                    <span>{{ node.label }}</span>
                    <span style="color: var(--el-color-info)">
                      {{ data.type }}
                    </span>
                  </span>
                </template>
              </el-tree-select>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item
              label="VECTOR Flash Symbol End"
              prop="vector_flash_end"
            >
              <el-tree-select
                v-model="data.vector_flash_end"
                :data="sectionTreeList"
                highlight-current
                default-expand-all
                style="width: 100%"
                clearable
              >
                <template #default="{ node, data }">
                  <span
                    style="
                      display: flex;
                      justify-content: space-between;
                      font-size: 12px;
                    "
                  >
                    <span>{{ node.label }}</span>
                    <span style="color: var(--el-color-info)">
                      {{ data.type }}
                    </span>
                  </span>
                </template>
              </el-tree-select>
            </el-form-item>
          </el-col>
        </el-form-item>
        <el-form-item
          label="VECTOR Place Region"
          prop="vector_flash_region"
        >
          <el-tree-select
            v-model="data.vector_flash_region"
            :data="regionTreeList"
            highlight-current
            default-expand-all
            style="width: 100%"
            clearable
          >
            <template #default="{ node, data }">
              <span
                style="
                  display: flex;
                  justify-content: space-between;
                  font-size: 12px;
                "
              >
                <span>{{ node.label }}</span>
                <span style="color: var(--el-color-info)">
                  {{ data.type }}
                </span>
              </span>
            </template>
          </el-tree-select>
        </el-form-item>
        <el-form-item
          label="VECTOR RAM Symbol"
          prop="vector_ram"
        >
          <el-tree-select
            v-model="data.vector_ram"
            :data="sectionTreeList"
            highlight-current
            default-expand-all
            style="width: 100%"
            clearable
          >
            <template #default="{ node, data }">
              <span
                style="
                  display: flex;
                  justify-content: space-between;
                  font-size: 12px;
                "
              >
                <span>{{ node.label }}</span>
                <span style="color: var(--el-color-info)">
                  {{ data.type }}
                </span>
              </span>
            </template>
          </el-tree-select>
        </el-form-item>
        <el-form-item
          label="Using FreeRTOS"
          prop="useFreeRtos"
        >
          <el-checkbox
            v-model="data.useFreeRtos"
            @change="FreeRTOSchange"
          />
        </el-form-item>
        <el-form-item
          label="Using Secure Boot"
          prop="useSecureBoot"
          v-show="secureBootSupport"
        >
          <el-checkbox v-model="data.useSecureBoot" />
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, nextTick, reactive, watch, toRaw } from "vue";
import { Plugin } from "plugin";
import { valid } from "./valid.js";
import Schema from "async-validator";
import type { FormInstance, FormRules } from "element-plus";
Schema.warning = function () {
  null;
};
const masterRef = ref<FormInstance>();
const formErrors = ref<Record<string, string>>({});
const rules = ref<FormRules>({});
watch(formErrors, (val: Record<string, string>) => {
  rules.value = {};
  const keys = Object.keys(val);
  for (const key of keys) {
    rules.value[key] = {
      validator: () => false,
      message: val[key],
    };
  }
  nextTick(() => {
    masterRef.value?.clearValidate();
    nextTick(() => {
      masterRef.value?.validate().catch(() => {
        null;
      });
    });
  });
});

function symbolCheck(rule, value, callback) {
  if (value) {
    let found = false;
    for (const item of sectionList.value) {
      if (`${item.name}_start` == value || `${item.name}_end` == value) {
        found = true;
        break;
      }
    }
    if (!found) {
      callback(new Error("Symbol not found"));
    }
  }
  callback();
}
function symbolFlashCheck(rule, value, callback) {
  if (value) {
    let found = false;
    for (const item of sectionList.value) {
      if (`${item.name}_start` == value || `${item.name}_end` == value) {
        found = true;
        break;
      }
    }
    if (!found) {
      callback(new Error("Symbol not found"));
    }
  }
  callback();
}
function regionCheck(rule, value, callback) {
  if (value) {
    let found = false;
    for (const item of regionList.value) {
      if (`.${item.name}` == value) {
        found = true;
        break;
      }
    }
    if (!found) {
      callback(new Error("Region not found"));
    }
  }
  callback();
}
const extRules = {
  stack_top: [{ validator: symbolCheck }],
  vector_ram: [{ validator: symbolCheck }],
  vector_flash: [{ validator: symbolFlashCheck }],
  vector_flash_end: [{ validator: symbolFlashCheck }],
  vector_flash_region: [{ validator: regionCheck }],
};
function dataCheck() {
  for (const error of Object.keys(formErrors.value)) {
    plugin.deleteLog("error", error);
  }
  valid(data, extRules)
    .then((val: any) => {
      formErrors.value = val;
      for (const key of Object.keys(formErrors.value)) {
        plugin.addLog("error", key, formErrors.value[key]);
      }
    })
    .catch((val) => {
      plugin.addLog("error", "system", val.message);
    });
}

function FreeRTOSchange(val) {
  const newVal:any = {
    depends: [],
    cdefines:[]
  };
  if (val) {
    newVal.depends.push("FREERTOS");
    newVal.cdefines.push("USING_OS_FREERTOS");
  }
  let changed = false;
  const oldVal = plugin.projectInfo.targetToolOpt["KEIL"]?.device
  if(JSON.stringify(oldVal)!=JSON.stringify(newVal))
  {
    changed = true;
  }
  if(changed){
    plugin.setCompOpt("device", "KEIL", newVal);
    plugin.setCompOpt("device", "EWARM", newVal);
    plugin.setCompOpt("device", "CMakeKEIL", newVal);
    plugin.setCompOpt("device", "CMakeGCC", newVal);
    plugin.setCompOpt("device", "CMakeEWARM", newVal);
    plugin.setCompOpt("device", "CMakeGHS", newVal);
  }
  
}

function getDefaultData() {
  return {
    pn: "",
    sdkVer: "",
    useFreeRtos: false,
    useSecureBoot: false,
    stack_top: "STACK_end",
    vector_ram: "IVT_RAM_start",
    vector_flash_region: ".isr_vector",
    vector_flash: "IVT_start",
    vector_flash_end: "IVT_end",
    vectors: [],
  };
}

const lock = ref(false);
const secureBootSupport = ref(false);

const plugin = new Plugin({
  lockChange: (val) => {
    lock.value = val;
  },
});
/*Stage and Table height define*/
const data = reactive(getDefaultData());

const sectionList = ref<any[]>([]);
const regionList = ref<any[]>([]);
const sectionTreeList = ref<linkTree[]>([]);
const regionTreeList = ref<linkTree[]>([]);
interface linkTree {
  id: string;
  label: string;
  value: string;
  isFlash: boolean;
  type: string;
  children: linkTree[];
}
function getAllSubSection(top: any[]) {
  sectionList.value = [];
  regionList.value = [];
  sectionTreeList.value = [];
  regionTreeList.value = [];
  for (let i = 0; i < top.length; i++) {
    //level1
    if (top[i].children) {
      const p1: linkTree = {
        id: top[i].id,
        label: top[i].name,
        value: top[i].name,
        isFlash: top[i].isFlash,
        type: "Block",
        children: [],
      };

      const r1: linkTree = {
        id: top[i].id,
        label: top[i].name,
        value: top[i].name,
        isFlash: top[i].isFlash,
        type: "Block",
        children: [],
      };
      sectionTreeList.value.push(p1);
      regionTreeList.value.push(r1);
      for (const child of top[i].children) {
        //level2
        child.parent = top[i];
        if (child.children) {
          const p3: linkTree = {
            id: child.id,
            label: child.name,
            value: child.name,
            isFlash: child.isFlash,
            type: "Memory",
            children: [],
          };
          p1.children.push(p3);
          const r3: linkTree = {
            id: child.id,
            label: child.name,
            value: child.name,
            isFlash: child.isFlash,
            type: "Memory",
            children: [],
          };
          r1.children.push(r3);
          for (const child2 of child.children) {
            //level3

            child2.parent = child;
            if (child2.children) {
              for (const child3 of child2.children) {
                //level4
                const p41: linkTree = {
                  id: child3.id,
                  label: `${child3.name}_start`,
                  value: `${child3.name}_start`,
                  isFlash: child3.isFlash,
                  type: "Section",
                  children: [],
                };
                p3.children.push(p41);
                const p42: linkTree = {
                  id: child3.id + "1",
                  label: `${child3.name}_end`,
                  value: `${child3.name}_end`,
                  isFlash: child3.isFlash,
                  type: "Section",
                  children: [],
                };
                p3.children.push(p42);
                const r4: linkTree = {
                  id: child3.id,
                  label: child3.name,
                  value: child3.name,
                  isFlash: child3.isFlash,
                  type: "Section",
                  children: [],
                };
                r3.children.push(r4);

                child3.parent = child2;
                sectionList.value.push(child3);

                if (child3.children) {
                  for (const child4 of child3.children) {
                    child4.parent = child3;
                    regionList.value.push(child4);
                    r4.children.push({
                      id: child4.id,
                      label: `.${child4.name}`,
                      value: `.${child4.name}`,
                      isFlash: child4.isFlash,
                      type: "Region",
                    } as linkTree);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

onMounted(() => {
  plugin
    .getInitValue()
    .then((initData) => {
      // check if support secure boot
      secureBootSupport.value = plugin.getMeta("secure_boot_support");
      if (initData.pluginData != undefined) {
        lock.value = initData.pluginData.lock;
        Object.assign(data, initData.pluginData.data);
      }
      FreeRTOSchange(data.useFreeRtos);
      // if (data.useFreeRtos) {
      //   plugin.addCompOpt("cdefines", "USING_OS_FREERTOS");
      // } else {
      //   plugin.deleteCompOpt("cdefines", "USING_OS_FREERTOS");
      // }
      data.vectors = [];
      const interrupt = plugin.getMeta("interrupt");
      for (const [index, item] of interrupt.entries()) {
        if (index > 0) {
          for (let i = 0; i < item.num - interrupt[index - 1].num - 1; i++) {
            data.vectors.push("Reserved");
          }
        }
        data.vectors.push(item.callback);
      }

      /* freeze the data */
      data.pn = initData.projectInfo.partNumber;
      data.sdkVer = initData.projectInfo.sdkVersion;
      /*always push a data when init done*/
      plugin.dataChange(toRaw(data));
      if (initData.globalData.ytlink && initData.globalData.ytlink.data) {
        getAllSubSection(initData.globalData.ytlink.data);
      }
      dataCheck();
      plugin.watch("ytlink", (newData) => {
        getAllSubSection(newData.data);
        dataCheck();
      });
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(toRaw(raw));
      });
    })
    .catch((e: any) => {
      console.error(e);
    });
});
</script>
<style scoped></style>
