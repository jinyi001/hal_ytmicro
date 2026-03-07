<template>
  <div id="diagramMain">
    <div id="loading" v-if="loading" v-loading="loading"></div>
    <div id="diagram" />
    <div class="helpTools">
      <el-icon class="help" @click="resetTree">
        <Refresh />
      </el-icon>
      <el-icon class="help" @click="showTree">
        <List />
      </el-icon>
      <el-icon class="help" @click="scalePaper('in')">
        <Plus />
      </el-icon>
      <el-icon class="help" @click="scalePaper('out')">
        <Minus />
      </el-icon>
      <el-icon class="help" @click="fitPater">
        <FullScreen />
      </el-icon>
    </div>
    <div>
      <el-dialog
        v-model="dialogVisible"
        :title="dialogTitle"
        width="50%"
        align-center
        :close-on-click-modal="false"
        :close-on-press-escape="false"
      >
        <Edit
          v-if="dialogVisible"
          :level="editLevel"
          :parent-id="editParentId"
          :id="editId"
          :top="model"
          @add="nodeAdd"
        />
      </el-dialog>
      <el-drawer
        :show-close="false"
        v-model="drawer"
        title="Link Tree"
        direction="rtl"
      >
        <div>
          <el-tree :data="model" :props="defaultProps" default-expand-all>
            <template #default="{ node, data }">
              <span v-if="data.level == 0">
                <span
                  :style="{
                    color: colorMap[data.level].fill,
                    'margin-right': '5px',
                  }"
                  >{{ data.isFlash ? "FLASH" : "RAM" }}</span
                >{{ node.label }}
              </span>
              <span v-else
                ><span
                  :style="{
                    color: colorMap[data.level].fill,
                    'margin-right': '5px',
                  }"
                  >{{ titleMap[data.level] }}</span
                >{{ node.label }}</span
              >
            </template>
          </el-tree>
        </div>
        <template #footer>
          <div style="flex: auto">
            <el-button @click="drawer = false" plain size="small">
              close
            </el-button>
          </div>
        </template>
      </el-drawer>
    </div>
  </div>
</template>
<script setup lang="ts">
import {
  onMounted,
  ref,
  inject,
  Ref,
  onUnmounted,
  provide,
  nextTick,
  watch,
  toRaw,
} from "vue";
import {
  Ceil,
  LinkView,
  titleMap,
  colorMap,
  LinkItem,
  generateUUID,
} from "./view.ts";
import Edit from "./edit.vue";
import { ElMessage, ElMessageBox } from "element-plus";

// Import SDK default ytlink configuration
import YTM32B1LE05Def from "./le05def.json";
import YTM32B1LE04Def from "./le04def.json";
import YTM32B1LE15Def from "./le15def.json";
import YTM32B1LE14Def from "./le14def.json";
import YTM32B1MC03Def from "./mc03def.json";
import YTM32B1ME05Def from "./me05def.json";
import YTM32B1ME14Def from "./me14def.json";
import YTM32B1ME15Def from "./me15def.json";
import YTM32B1MD14Def from "./md14def.json";
import YTM32B1MD23Def from "./md23def.json";
import YTM32B1MD24Def from "./md24def.json";
import YTM32B1HA01Def from "./ha01def.json";
import YTM32Z1MD04Def from "./md04def.json";
import YTM32Z1ME05Def from "./z1me05def.json";

// Import MCAL default ytlink configuration
import YTM32B1MC03Mcal from "./mc03_mcal.json";
import YTM32B1ME05Mcal from "./me05_mcal.json";
import YTM32B1MD14Mcal from "./md14_mcal.json";
import YTM32B1HA01Mcal from "./ha01_mcal.json";
import YTM32B1MD24Mcal from "./md24_mcal.json";

const defaultProps = {
  children: "children",
  label: "name",
};
import {
  FullScreen,
  Minus,
  Plus,
  List,
  Refresh,
} from "@element-plus/icons-vue";
import { Plugin } from "plugin";
const lock = ref(false);
const plugin = new Plugin({
  lockChange: (val) => {
    lock.value = val;
  },
});
const model = ref<LinkItem[]>([]);
const drawer = ref(false);
const dialogVisible = ref(false);
const dialogTitle = ref("");
let paper: any;
const graph = new joint.dia.Graph();
const editLevel = ref(0);
const editParentId = ref();
const editId = ref();

const linkView = new LinkView(graph);
provide("linkView", linkView);
const defaultLinkMap: Record<string, LinkItem[]> = {
  YTM32B1MC03_SDK: YTM32B1MC03Def as LinkItem[],
  YTM32B1LE05_SDK: YTM32B1LE05Def as LinkItem[],
  YTM32Z1LS06_SDK: YTM32B1LE05Def as LinkItem[],
  YTM32B1LE04_SDK: YTM32B1LE04Def as LinkItem[],
  YTM32B1LE15_SDK: YTM32B1LE15Def as LinkItem[],
  YTM32B1LE14_SDK: YTM32B1LE14Def as LinkItem[],  
  YTM32B1ME05_SDK: YTM32B1ME05Def as LinkItem[],
  YTM32B1ME14_SDK: YTM32B1ME14Def as LinkItem[],
  YTM32B1ME15_SDK: YTM32B1ME15Def as LinkItem[],
  YTM32B1MD14_SDK: YTM32B1MD14Def as LinkItem[],
  YTM32B1MD23_SDK: YTM32B1MD23Def as LinkItem[],
  YTM32B1MD24_SDK: YTM32B1MD24Def as LinkItem[],
  YTM32B1HA01_SDK: YTM32B1HA01Def as LinkItem[],
  YTM32Z1MD04_SDK: YTM32Z1MD04Def as  LinkItem[],
  YTM32Z1ME05_SDK: YTM32Z1ME05Def as  LinkItem[],
  YTM32B1HA01_MCAL: YTM32B1HA01Mcal as LinkItem[],
  YTM32B1MC03_MCAL: YTM32B1MC03Mcal as LinkItem[],
  YTM32B1ME05_MCAL: YTM32B1ME05Mcal as LinkItem[],
  YTM32B1MD14_MCAL: YTM32B1MD14Mcal as LinkItem[],
  YTM32B1MD24_MCAL: YTM32B1MD24Mcal as LinkItem[],
};

function scalePaper(command: "in" | "out") {
  if (paper) {
    const step = 0.1; // Scaling step
    const currentScale = paper.scale().sx; // Assuming uniform scaling for simplicity

    // Determine the new scale based on the command
    let newScale = command === "in" ? currentScale + step : currentScale - step;

    // Limit the scale to a reasonable range to prevent too much scaling
    newScale = Math.max(0.2, Math.min(5, newScale));

    // Calculate the center of the paper
    const originalSize = paper.getComputedSize();
    const centerX = originalSize.width / 2;
    const centerY = originalSize.height / 2;

    // Calculate the new origin to keep the center
    const newOriginX =
      paper.translate().tx - centerX * (newScale - currentScale);
    const newOriginY =
      paper.translate().ty - centerY * (newScale - currentScale);

    // Update the origin and scale
    paper.translate(newOriginX, newOriginY);
    paper.scale(newScale, newScale);
  }
}
function fitPater() {
  if (paper) {
    paper.transformToFitContent({
      horizontalAlign: "middle",
      verticalAlign: "middle",
      padding: 50,
      maxScaleX: 1.5,
      maxScaleY: 1.5,
    });
  }
}

function showTree() {
  drawer.value = true;
}
let chipPn = "";
function resetTree() {
  ElMessageBox.confirm(
    `Are you sure to reset the link configuration?`,
    "Warning",
    {
      confirmButtonText: "OK",
      cancelButtonText: "Cancel",
      type: "warning",
      buttonSize: "small",
    }
  )
    .then(() => {
      graph.clear();
      linkView.clear();
      model.value = _.cloneDeep(defaultLinkMap[chipPn]);
      const pendings = linkView.buildCeilTree(model.value);
      for (const vl of pendings) {
        linkView.addLink(vl[0], vl[1]);
      }
      fitPater();
      plugin.dataChange(toRaw(model.value));
    })
    .catch(() => {
      //do nothing
    });
}

const width = ref(window.innerWidth);
const height = ref(window.innerHeight);
function resizeHandler() {
  width.value = window.innerWidth;
  height.value = window.innerHeight;
  paper?.setDimensions(width.value - 10, height.value - 10);
}
const dark = document.documentElement.className.indexOf("dark") > -1;
const dotColor = dark ? "#909399" : "#909399";

function getKeepForKeil() {
  const keepList: string[] = ["--no_startup","--diag_suppress=L6314W,L6221E,L6329W"];
  for (const l1 of model.value) {
    if (Array.isArray(l1.children)) {
      for (const l2 of l1.children) {
        if (Array.isArray(l2.children)) {
          for (const l3 of l2.children) {
            if (Array.isArray(l3.children)) {
              for (const l4 of l3.children) {
                if (Array.isArray(l4.children)) {
                  for (const l5 of l4.children) {
                    if (l5.flags == "KEEP") {
                      let i = "--keep=";
                      if (l5.wildcard) {
                        i += `*(.${l5.name}*)`;
                      } else {
                        i += `*(.${l5.name})`;
                      }
                      keepList.push(i);
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
  let changed = false;
  const newVal = {
    linkflags: keepList,
  };
  const oldVal = plugin.projectInfo.targetToolOpt["KEIL"]?.ytlink
  if (
    !_.isEqual(oldVal, newVal)
  ) {
    changed = true;
  }
  if (changed) {
    plugin.setCompOpt("ytlink", "KEIL", newVal);
    plugin.setCompOpt("ytlink", "CMakeKEIL", newVal);
  }
}
watch(
  model,
  () => {
    plugin.dataChange(toRaw(model.value));
    getKeepForKeil();
  },
  {
    deep: true,
  }
);

function waitingShow() {
  return new Promise((resolve) => {
    plugin.on("show", () => {
      nextTick(() => {
        resolve(true);
      });
      // resolve(true);
    });
  });
}
const loading = ref(true);
onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      chipPn = `${initData.projectInfo.partNumber.slice(0, 11)}_${initData.projectInfo.type.toUpperCase()}`
      if (initData.pluginData != undefined) {
        model.value = initData.pluginData.data;
      } else {
        model.value = _.cloneDeep(defaultLinkMap[chipPn]);
      }
      getKeepForKeil();
      plugin.dataChange(toRaw(model.value));
      waitingShow()
        .then(() => {
          paper = new joint.dia.Paper({
            async: false,
            el: document.getElementById("diagram"),
            model: graph,
            width: width.value - 10,
            height: height.value - 10,
            gridSize: 10,
            drawGrid: { name: "dot", color: dotColor, thickness: 1 },
            background: {
              color: "var(--el-bg-color)",
            },
            snapLabels: true,
            interactive: {
              linkMove: false,
              labelMove: true,
              arrowheadMove: false,
              vertexMove: false,
              vertexAdd: false,
              vertexRemove: false,
              useLinkTools: false,
            },
            defaultLink: () => new joint.shapes.standard.Link(),
            linkPinning: false,
            validateConnection: function (
              cellViewS,
              magnetS,
              cellViewT,
              magnetT,
              end,
              linkView
            ) {
              // Prevent linking from input ports
              if (magnetS && magnetS.getAttribute("port-group") == "in")
                return false;
              // Prevent linking from output ports to input ports within one element
              if (cellViewS === cellViewT) return false;
              // Prevent linking to output ports
              return magnetT && magnetT.getAttribute("port-group") == "in";
            },
            validateMagnet: function (cellView, magnet) {
              // Note that this is the default behaviour. It is shown for reference purposes.
              // Disable linking interaction for magnets marked as passive
              return magnet.getAttribute("magnet") !== "passive";
            },
          });
          /* https://github.com/clientIO/joint/issues/964 */
          // Object.defineProperty(paper.options, "width", {
          //   get: function () {
          //     return width.value - 10;
          //   },
          //   set: function (value) {
          //     return value;
          //   },
          // });
          paper.$el.on("wheel", function (event) {
            event.preventDefault(); // 防止页面滚动
            const delta = event.originalEvent.deltaY; // 获取滚动方向和距离
            if (event.ctrlKey) {
              // 获取鼠标位置
              const clientX = event.originalEvent.clientX;
              const clientY = event.originalEvent.clientY;
              const offset = paper.clientToLocalPoint(clientX, clientY);
              // 更新缩放比例
              const currentScale = paper.scale().sx - delta * 0.001;
              // 应用缩放
              paper.scaleUniformAtPoint(currentScale, {
                x: offset.x,
                y: offset.y,
              });
            } else {
              if (event.shiftKey) {
                paper.translate(
                  paper.translate().tx - delta * 0.2,
                  paper.translate().ty
                ); // 滚动时移动画布
              } else {
                paper.translate(
                  paper.translate().tx,
                  paper.translate().ty - delta * 0.2
                ); // 滚动时移动画布
              }
            }
          });
          linkView.setPaper(paper);
          graph.clear();

          const pendings = linkView.buildCeilTree(model.value);
          for (const vl of pendings) {
            linkView.addLink(vl[0], vl[1]);
          }
          linkView.on("add", addNewCeil);
          linkView.on("remove", removeCeil);
          linkView.on("edit", editCeil);
          setTimeout(() => {
            fitPater();
            nextTick(() => {
              loading.value = false;
            });
         
          }, 1000);
          
          
        })
        .catch((e) => {
          console.error(e);
        });
    })
    .catch((e) => {
      console.error(e);
    });

  /* add default link map */

  editParentId.value = "";
  editId.value = "";
});

function nodeAdd() {
  editParentId.value = "";
  editId.value = "";
  dialogVisible.value = false;
}

function removeCeil(ceil: Ceil) {
  ElMessageBox.confirm(
    `Are you sure to delete ${ceil.data.name} and all children item`,
    "Warning",
    {
      confirmButtonText: "OK",
      cancelButtonText: "Cancel",
      type: "warning",
    }
  )
    .then(() => {
      linkView.removeCeil(ceil.data.id);
      linkView.refreshLevel1(ceil.parent?.data.id);
    })
    .catch(() => {
      //do nothing
    });
}
function editCeil(ceil: Ceil) {
  editLevel.value = ceil.data.level;
  editParentId.value = ceil.parent?.data.id;
  editId.value = ceil.data.id;
  dialogTitle.value = titleMap[editLevel.value]
    ? `Edit ${titleMap[editLevel.value]}`
    : "Edit Region";
  nextTick(() => {
    dialogVisible.value = true;
  });
}
function addNewCeil(ceil: Ceil) {
  editLevel.value = ceil.data.level + 1;
  editParentId.value = ceil.data.id;
  editId.value = "";
  dialogTitle.value = titleMap[editLevel.value]
    ? `Add ${titleMap[editLevel.value]}`
    : "Add New Region";
  nextTick(() => {
    dialogVisible.value = true;
  });
}

onUnmounted(() => {
  linkView.destroy();
  graph.clear();
  window.removeEventListener("resize", resizeHandler);
});
</script>
<style>
.joint-tool.joint-theme-default[data-tool-name="boundary"] {
  stroke: var(--el-color-info);
}
.is-panning {
  cursor: move;
}
</style>
<style scoped>
#diagramMain {
  width: v-bind(width-10 + "px");
  height: v-bind(height-10 + "px");
  position: relative;
}
#diagram {
  margin: 5px;
}
.helpTools {
  position: absolute;
  bottom: 5px;
  right: 5px;
  z-index: 100;
}
.helpTools .help {
  margin: 3px;
}
.helpTools .help:hover {
  cursor: pointer;
  color: var(--el-color-info);
}

#loading{
  z-index: 1000;
  position: absolute;
  background-color: var(--el-color-white);
  width: v-bind(width - 5 + "px");
  height: v-bind(height - 5  + "px");
}
</style>
