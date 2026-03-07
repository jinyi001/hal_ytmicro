<template>
  <div>
    <el-form
      @submit.prevent
      ref="ruleFormRef"
      :model="data"
      :rules="rules"
      label-width="150px"
      class="demo-ruleForm"
      size="small"
    >
      <el-form-item
        label="Name"
        prop="name"
      >
        <el-input v-model="data.name" />
      </el-form-item>
      
      <el-form-item
        label="Boundary"
        prop="boundary"
        v-if="props.level == 1"
      >
        <template #label="{ label }">
          <span style="display: flex; align-items: center">
            <span>{{ label }}</span>
            <el-tooltip
              :show-after="500"
              effect="dark"
              raw-content
              content="Memory Region Blocks can be mapped either to the lower (lower address value) or upper (higher address value) end of the Memory Region.<br/>
Their start address is then calculated based on the boundary value, their position and the size of their preceding blocks."
              placement="bottom-start"
            >
              <el-icon style="margin-left: 5px;"><InfoFilled /></el-icon>
            </el-tooltip>
          </span>
        </template>
        <el-select
          v-model="data.boundary"
          style="width: 100%"
        >
          <el-option
            label="UPPER"
            value="UPPER"
          />
          <el-option
            label="LOWER"
            value="LOWER"
          />
        </el-select>
      </el-form-item>
      <el-form-item
        label="Order"
        prop="order"
        v-if="
          props.level == 1 ||
            props.level == 2 ||
            props.level == 3 ||
            props.level == 4
        "
      >
        <template #label="{ label }">
          <span style="display: flex; align-items: center">
            <span>{{ label }}</span>

            <el-tooltip
              :show-after="500"
              effect="dark"
              raw-content
              content="Specifies the relative order of the Memory Region Block within its parent Memory Region.<br/>
If multiple blocks have the same order, they are sorted by their names.
"
              placement="bottom-start"
            >
              <el-icon style="margin-left: 5px;"><InfoFilled /></el-icon>
            </el-tooltip>
          </span>
        </template>
        <el-input-number
          v-model="data.order"
          :min="0"
          controls-position="right"
        />
      </el-form-item>
      <el-form-item
        label="Alignment[Byte]"
        prop="align"
        v-if="props.level == 3 || props.level == 4"
      >
        <el-input-number
          v-model="data.align"
          :min="0"
          controls-position="right"
        />
      </el-form-item>
      <el-form-item
        label="End Alignment[Byte]"
        prop="endAlign"
        v-if="props.level == 3"
      >
        <el-input-number
          v-model="data.endAlign"
          :min="0"
          controls-position="right"
        />
      </el-form-item>
      <el-form-item
        label="Section Size"
        prop="groupSize"
        v-if="props.level == 3"
      >
        <el-input v-model="data.groupSize" />
      </el-form-item>
      <el-form-item
        label="Memory Size"
        prop="size"
        v-if="props.level == 1"
      >
        <el-input v-model="data.size" />
      </el-form-item>
      <el-form-item
        label="Flag"
        prop="flags"
        v-if="props.level == 4 || props.level == 3"
      >
        <el-select v-model="data.flags">
          <el-option
            label="NULL"
            value="NULL"
          />
          <el-option
            label="KEEP"
            value="KEEP"
            v-if="props.level == 4"
          />
          <el-option
            label="NOLOAD"
            value="NOLOAD"
            v-if="props.level == 3"
          />
        </el-select>
      </el-form-item>
      <el-form-item
        label="CopyFrom"
        prop="copyFrom"
        v-if="props.level == 3 && !data.isFlash"
      >
        <el-select
          v-model="data.copyFrom"
          style="width: 100%"
          clearable
        >
          <el-option
            v-for="item in copyFromList"
            :key="item.id"
            :label="item.name"
            :value="item.id"
            :disabled="item.disabled"
          />
        </el-select>
      </el-form-item>
      <el-form-item
        label="Filenames"
        prop="binFile"
        v-if="props.level == 4"
      >
        <template #label="{ label }">
          <span style="display: flex; align-items: center">
            <span>{{ label }}</span>

            <el-tooltip
              :show-after="500"
              effect="dark"
              raw-content
              content="
              Specifies the name of one ore more files from which the content of the Linker Section is to be linked.<br/>
Whereby the names of individual files or archives (libraries) with the files they contain can be specified.<br/>
So it is possible to place the content of a linker section only from one or more files to a certain position.<br/>

If several files or archives are to be specified, they must be entered as a semicolon-separated list.<br/>
Where an archive is defined by a name followed by parentheses. Within these parentheses, members of the archive can optionally be specified as a comma-separated list.
See examples below.<br/>

From this, <strong>YT-LINK</strong> generates, if necessary for the selected compiler, individual selection commands for each file or archive.<br/>
For individual selection commands, the file names are generated according to their specified order.<br/>

<strong>Examples:</strong>
<ul>
<li>file.o</li>
<li>file_a.o;file_b.o;file_c.o</li>
<li>lib.a()</li>
<li>lib.a(file_a.o,file_b.o)</li>
<li>lib1.a(file_*); lib2.a(*); file_?.o</li>
<li>/usr/folder/lib/lib.a(/usr/folder/source/file1.o)</li>
<li>C:/folder/lib/lib.a(C:/folder/source/file1.o)</li>
</ul>


<strong>Note</strong>:
<ul>
  <li>The specified file or archive names will be generated as they are. The string is only split and whitespace are removed.</li>
  <li>That means for most compilers it is also possible to specify wildcards (*, ?) and complete paths.</li>
  <li>Ensure that UNIX style file paths do not contain parentheses.</li>
</ul>
"
              placement="bottom-start"
            >
              <el-icon style="margin-left: 5px;"><InfoFilled /></el-icon>
            </el-tooltip>
          </span>
        </template>
        <el-input v-model="data.binFile" />
      </el-form-item>
      <el-form-item
        v-if="props.level == 4 || props.level ==3"
        label-width="0"
      >
        <el-row>
          <el-col :span="12">
            <el-form-item
              label="Alt Symbol Start"
              prop="altSymbolStart"
              v-if="props.level == 4 || props.level ==3"
            >
              <el-input v-model="data.altSymbolStart" />
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item
              label="Alt Symbol End"
              prop="altSymbolEnd"
              v-if="props.level == 4 || props.level ==3"
            >
              <el-input v-model="data.altSymbolEnd" />
            </el-form-item>
          </el-col>
        </el-row>
      </el-form-item>
      <el-form-item
        label="Remove Dot"
        prop="removeDot"
        v-if="props.level == 4"
      >
        <template #label="{ label }">
          <span style="display: flex; align-items: center">
            <span>{{ label }}</span>

            <el-tooltip
              :show-after="500"
              effect="dark"
              raw-content
              content="
              remove dot in region name, when disabled, the region name only ${name}, if enabled, the region name will be .${name} 
"
              placement="bottom-start"
            >
              <el-icon style="margin-left: 5px;"><InfoFilled /></el-icon>
            </el-tooltip>
          </span>
        </template>
        <el-checkbox
          v-model="data.removeDot"
        />
      </el-form-item>
      <el-form-item
        label="Add Wildcard"
        prop="wildcard"
        v-if="props.level == 4"
      >
        <template #label="{ label }">
          <span style="display: flex; align-items: center">
            <span>{{ label }}</span>

            <el-tooltip
              :show-after="500"
              effect="dark"
              raw-content
              content="
              add wildcard (*) to the region name, when disabled, the region name only .${name}, if enabled, the region name will be .${name} .${name}*
"
              placement="bottom-start"
            >
              <el-icon style="margin-left: 5px;"><InfoFilled /></el-icon>
            </el-tooltip>
          </span>
        </template>
        <el-checkbox
          v-model="data.wildcard"
        />
      </el-form-item>
      <el-form-item
        label="Clear Section"
        prop="clear"
        v-if="props.level == 3 && !data.isFlash"
      >
        <el-checkbox
          v-model="data.clear"
        />
      </el-form-item>
      <el-form-item
        label="Init Policy"
        prop="init"
        v-if="!data.isFlash&&props.level == 1"
      >
        <template #label="{ label }">
          <span style="display: flex; align-items: center">
            <span>{{ label }}</span>

            <el-tooltip
              :show-after="500"
              effect="dark"
              raw-content
              content="
              <ul>
                <li><strong>NULL</strong>: No initialization</li>
                <li><strong>POR_ONLY</strong>: Initialization only after Power-On Reset</li>
                <li><strong>NORMAL</strong>: Normal initialization</li>
                </ul>
              "
              placement="bottom-start"
            >
              <el-icon style="margin-left: 5px;"><InfoFilled /></el-icon>
            </el-tooltip>
          </span>
        </template>
        <el-select
          v-model="data.init"
        >
          <el-option
            label="NULL"
            value="NULL"
          />
          <el-option
            label="POR_ONLY"
            value="POR"
          />
          <el-option
            label="NORMAL"
            value="NORMAL"
          />
        </el-select>
      </el-form-item>
      <el-form-item>
        <div style="text-align: right">
          <el-button
            type="warning"
            @click="EditNode"
            v-if="id"
            style="width: 100px"
          >
            Save
          </el-button>
          <el-button
            type="primary"
            @click="AddNode"
            v-else
            style="width: 100px"
          >
            Add
          </el-button>
        </div>
      </el-form-item>
    </el-form>
  </div>
</template>
<script setup lang="ts">
import {
  computed,
  inject,
  nextTick,
  onMounted,
  reactive,
  ref,
  toRef,
  watch,
} from "vue";
import { Ceil, LinkView,generateUUID,LinkItem} from "./view.ts";
import { FormRules } from "element-plus";
import { validLevel1, validLevel2, validLevel3, validLevel4 } from "./valid.ts";
import {
  InfoFilled,
} from '@element-plus/icons-vue'
const ruleFormRef = ref();
const props = defineProps<{
  level: number;
  parentId?: string;
  id?: string;
  top:LinkItem[]
}>();


const parentId = toRef(props, "parentId");
const id = toRef(props, "id");
const rules = ref<FormRules>({});
const emits = defineEmits(["add"]);
const linkView = inject("linkView") as LinkView;

const copyFromList = computed(() => {
  const list: {
    name: string;
    id: string;
    disabled: boolean;
  }[] = [];
  const findFlash = (link: LinkItem) => {
    if (link.isFlash && link.level == 3) {
      const exist = linkView.linkMap.has(link.id);
      list.push({
        name: link.name,
        id: link.id,
        disabled: exist ? true : false,
      });
    }
    if (link.children) {
      link.children.forEach((item) => {
        findFlash(item);
      });
    }
  };
  props.top.forEach((item: LinkItem) => {
    findFlash(item);
  });
  return list;
});

// function refreshLevel1(parentId?: string) {
//   if (!parentId) {
//     return;
//   }
//   const parent = linkView.ceilMap.get(parentId);
//   if (parent) {
//     if (parent.data.level == 0) {
//       const children = parent.children;
//       if (children) {
//         const upper = children.filter((item) => item.data.boundary == "UPPER");
//         const lower = children.filter((item) => item.data.boundary == "LOWER");
//         /* sort by order, if order is same, sort by name, store the sort result in new val*/
//         const upperSort = _.sortBy(upper, ["data.order", "data.name"]);
//         let totalSIze = 0;
//         /* calculate the start address of each block */
//         let startAddress = Number(parent.data.startAddress);
//         for (let i = 0; i < upperSort.length; i++) {
//           totalSIze += Number(upperSort[i].data.size);
//           const strAddr= '0x'+startAddress.toString(16);
//           upperSort[i].data.startAddress = strAddr
//           upperSort[i].changeAddrSize(
//             strAddr,
//             upperSort[i].data.size || "0"
//           );
//           startAddress += Number(upperSort[i].data.size);
//         }

//         const lowerSort = _.sortBy(lower, ["data.order", "data.name"]);

//         startAddress =
//           Number(parent.data.startAddress) + Number(parent.data.size);
//         for (let i = lowerSort.length - 1; i >= 0; i--) {
//           totalSIze += Number(lowerSort[i].data.size);
//           startAddress -= Number(lowerSort[i].data.size);
//           const strAddr= '0x'+startAddress.toString(16);
//           lowerSort[i].data.startAddress = strAddr;
//           lowerSort[i].changeAddrSize(
//             strAddr,
//             lowerSort[i].data.size || "0"
//           );
//         }
//         /* change percent in size, with xx% format */
//         const percent = (totalSIze / Number(parent.data.size)) * 100;
//         parent.changeFlag(percent.toFixed(2) + "%");
//       }
//     }
//   }
// }
const formErrors = ref<Record<string, string>>({});
watch(formErrors, (val: Record<string, string>) => {
  rules.value = {};
  const keys = Object.keys(val);
  for (const key of keys) {
    rules.value[key] = {
      validator: () => false,
      message: val[key],
    };
  }
  ruleFormRef.value?.clearValidate();
  void nextTick(() => {
    void ruleFormRef.value?.validate().catch(() => {
      null;
    });
  });
});

async function valid(parent: LinkItem) {
  let ret = 0;
  if (data.value.level == 1) {
    const v = await validLevel1(data.value, parent);
    if (Object.keys(v).length > 0) {
      formErrors.value = v;
      ret = -1;
    }
  } else if (data.value.level == 2) {
    const v = await validLevel2(data.value, parent);
    if (Object.keys(v).length > 0) {
      formErrors.value = v;
      ret = -1;
    }
  } else if (data.value.level == 3) {
    const v = await validLevel3(data.value, parent, linkView.itemMap);
    if (Object.keys(v).length > 0) {
      formErrors.value = v;
      ret = -1;
    }
  } else if (data.value.level == 4) {
    const v = await validLevel4(data.value, parent);
    if (Object.keys(v).length > 0) {
      formErrors.value = v;
      ret = -1;
    }
  }

  return ret;
}

async function AddNode() {
  let Parent: Ceil | undefined;
  if (parentId.value) {
    Parent = linkView.ceilMap.get(parentId.value);
    if (Parent) {
      const ret = await valid(Parent.data);
      if (ret < 0) {
        return;
      }
    }
  }
  data.value.isFlash = Parent?.data.isFlash||false;
  const v = _.cloneDeep(data.value);
  let ceil
  if (Parent) {
    if (Parent.data.children == undefined) {
      Parent.data.children = [];
    }
    Parent.data.children.push(v);
    ceil = linkView.addCeil(Parent.data.children.at(-1) as any, Parent).ceil;
  }
  linkView.refreshLevel1(parentId.value);
  if (props.level == 3 && !data.value.isFlash) {
    if (data.value.copyFrom) {
      linkView.addLink(data.value.copyFrom, data.value.id);
    }
  }
  if (ceil&&ceil.data.level == 4) {
    ceil.changeFlag(data.value.flags || "NULL");
  }
  emits("add");
}

async function EditNode() {
  if (parentId.value) {
    const parent = linkView.ceilMap.get(parentId.value);
    if (parent) {
      const ret = await valid(parent.data);
      if (ret < 0) {
        return;
      }
    }
  }
  if (id.value) {
    const ceil = linkView.ceilMap.get(id.value);
    if (ceil) {
      if (props.level == 3 && !data.value.isFlash) {
        if (ceil.data.copyFrom) {
          linkView.removeLink(ceil.data.copyFrom);
        }

        if (data.value.copyFrom) {
          linkView.addLink(data.value.copyFrom, data.value.id);
        }
      }
      delete data.value.children;
      _.assign(ceil.data, data.value);
      ceil.changeName(data.value.name);
      if (
        ceil.data.level == 1 ||
        ceil.data.level == 2 ||
        ceil.data.level == 3 ||
        ceil.data.level == 4
      ) {
        ceil.changeOrder(data.value.order || 0);
      }
      if (ceil.data.level == 4) {
        ceil.changeFlag(data.value.flags || "NULL");
      }
    }
  }
  linkView.refreshLevel1(parentId.value);
  emits("add");
}

const data = ref<LinkItem>({
  name: "",
  level: props.level,
  id: generateUUID(),
  size: "",
  order: 0,
  startAddress: "",
  flags: "",
});

onMounted(() => {
  ruleFormRef.value?.clearValidate();
  rules.value = {};
  if (id.value) {
    const ceil = linkView.ceilMap.get(id.value);
    if (ceil) {
      data.value = _.cloneDeep(ceil.data);
    }
  }
  /* is flash asign */
  if (parentId.value) {
    const Parent = linkView.ceilMap.get(parentId.value);
    if (Parent) {
      data.value.isFlash = Parent?.data.isFlash;
    }
  }
});
</script>

<style scoped></style>
