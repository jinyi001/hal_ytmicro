<template>
  <div class="pins">
    <div class="pins-cfg-add-container">
      <el-select
        v-model="pinsData.curIdx"
        style=".el-input__wrapper {background-color: var(--el-bg-color)}"
      >
        <el-option
          v-for="(item, index) in pinsData.cfgs"
          :key="index"
          :label="item.name"
          :value="index.toString()"
        />
      </el-select>
      <el-button
        style="margin-left:10px; background-color: var(--el-bg-color)"
        @click="handleAddClick"
        :disabled="lock"
      >
        Add
      </el-button>
      <el-button
        type="danger"
        :disabled="pinsData.cfgs.length == 1 || lock"
        plain
        @click="handleDelClick"
      >
        Delete
      </el-button>
    </div>
    <div class="pins-cfg-container">
      <div>
        <div id="chip-view-container" />
        <div id="button" />
        <div
          ref="searchRef"
          class="search"
        >
          <el-select
            v-model="searchPin"
            filterable
            placeholder="Seartch by Pin"
            clearable
            @change="searchPinChange"
            @clear="searchPinClear"
          >
            <el-option
              v-for="item in pinList"
              :key="item.num"
              :label="item.label"
              :value="item.label"
            />
          </el-select>
        </div>
        <div
          ref="search1Ref"
          class="search1"
        >
          <el-select
            v-model="searchValue"
            filterable
            placeholder="Search by Feature"
            clearable
            @change="searchChange"
            @clear="searchClear"
          >
            <el-option
              v-for="item in searchList"
              :key="item"
              :label="item"
              :value="item"
            />
          </el-select>
        </div>
      </div>
      <div
        class="table"
        id="listWithHandle2"
      >
        <el-auto-resizer>
          <template #default="{ height, width }">
            <el-table-v2
              :columns="columns"
              :data="filterTableData"
              :estimated-row-height="30"
              :width="width"
              :header-height="50"
              :height="height"
              header-class="my-table-row"
              fixed
              class="table"
              :row-props="{draggable: !lock}"
              row-class="table-row"
              @scroll="tableScrollHandler"
            />
          </template>
        </el-auto-resizer>
      </div>
    </div>
    <el-color-picker
      v-model="color"
      :predefine="predefineColors"
      size="small"
      id="color-picker"
      ref="colorPickerRef"
      @change="handleColorChoose"
      @blur="handleColorBlur"
    />
  </div>
</template>

<script setup lang="ts">
import {
  onMounted,
  ref,
  computed,
  onBeforeUnmount,
  watch,
  h,
  toRef,
  nextTick
} from "vue";
import { PinDef } from "./pin.ts";
import { LQFP, HelpButtons } from "./lqfp.ts";
import { Plugin } from "plugin";
import { ElButton, ElColorPicker, ElInput, ElMessage, ElMessageBox, ElNotification, arrowMiddleware } from "element-plus";
import { Rank } from '@element-plus/icons-vue'
/*pin feature change calback*/
import DirSelect from "./DirSelect.vue";
import Konva from "konva";
import { readPinDef } from "./FeatureMap.js"
import type { Column } from '@element-plus/components/table-v2'

/*Stage and Table height define*/
const winW = ref(window.innerWidth);
const winH = ref(window.innerHeight);
const tableHeight = ref(300);

const generalDisable = (rowData:PinCfg)=>{
  /*- PIN配置为模拟功能时，除Laber及Direction之外的其他功能均不可配置；*/
  if((rowData.feature.indexOf('ADC')!=-1)||(rowData.feature.indexOf('ACMP')!=-1)){
    return true
  }
  return lock.value
}

const disableMap={
  intFlag: generalDisable,
  intCfg: generalDisable,
  pullEnable: generalDisable,
  filter: generalDisable,
  passiveFilter: generalDisable,
  strength: generalDisable,
  rateSelect: generalDisable,
  pullSelect:(rowData:PinCfg)=>{
    if(rowData.pullEnable){
      return lock.value
    }
    return true
  },
  init:(rowData:PinCfg)=>{
    if(rowData.feature=='GPIO'){
      return lock.value
    }
    return true
  },
  digitalWidth: (rowData: PinCfg) => {
    if (rowData.filter) {
      return lock.value
    }
    return true
  },
}

let globalColorPickerElement;
let colorPickerClicked = false;
const colorPickerRef = ref();
const color = ref('#FF0000')

function getColorPicker () {
  globalColorPickerElement = document.querySelector('#color-picker') as HTMLDivElement;
}
const predefineColors = ref([
  '#67c23a',
  '#b3e19d',
  '#b88230',
  '#f3d19e',
  '#c45656',
  '#fab6b6',
  '#fde2e2',
  '#90ee90',
  '#00ced1',
  '#c71585',
])

function tableScrollHandler () {
  if(colorPickerRef.value != undefined) {
    colorPickerClicked = false;
    colorPickerRef.value.hide();
  }
}

function handleColorChoose () {
  let dom = globalColorPickerElement;
  while (dom && !dom.classList.contains('table-row') && dom.parentElement) {
    dom = dom.parentElement;
  }
  const indexString = 
        (dom.children[dom.children.length-2] as HTMLDivElement)?.textContent || '';
  const targetIndex = parseInt(indexString);
  dom = globalColorPickerElement;
  while (dom && !dom.classList.contains('color-choose-container') && dom.parentElement) {
    dom = dom.parentElement;
  }
  dom.children[0].hidden = false;
  
  /**
   * This function call is necessary. 
   * ElColorPicker will not hide the color panel if user don't change any color and click change button
   * directly.
   */
  colorPickerRef.value.hide();

  nextTick(()=>{
    try {
      dom.removeChild(globalColorPickerElement);
    } catch (e) {
      /* nothing todo */
    }
  })
  if(Number.isNaN(targetIndex) || color.value == null) {
    return;
  }
  pinsData.value.cfgs[activeInst.value].pins[targetIndex].userColor = color.value;
  chip.setPinColor(
    pinsData.value.cfgs[activeInst.value].pins[targetIndex].num,
    color.value
  );
  colorPickerClicked = false;
}

function handleColorBlur () {
  // console.log('blur')
  let dom = globalColorPickerElement;
  while (dom && !dom.classList.contains('color-choose-container') && dom.parentElement) {
    dom = dom.parentElement;
  }
  dom.children[0].hidden = false;
  
  /**
   * This function call is necessary. 
   * ElColorPicker will not hide the color panel if user don't change any color and click change button
   * directly.
   */
  colorPickerRef.value.hide();

  nextTick(()=>{
    try {
      dom.removeChild(globalColorPickerElement);
    } catch (e) {
      /* nothing todo */
    }
    colorPickerClicked = false;
  })
}

const allColumn: Column<any>[] = [
  {
    key: 'dragIcon',
    title: '',
    dataKey: '',
    width: 50,
    align: 'center',
    cellRenderer: () => h('div', { class: lock.value?'table-drag-area-disabled':'table-drag-area' }, [
      h('el-icon', { class: 'rank-icon' }, h(Rank, {height: "25px", width: "25px"}))
    ])
  },
  {
    key: 'num',
    title: 'Pin Number',
    dataKey: 'num',
    width: 70,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {
        style: {
          'font-size': '12px'
        }
      }, rowData.num)
  },
  {
    key: 'label',
    title: 'Pin Name',
    dataKey: 'label',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {
        style: {
          'font-size': '12px'
        }
      }, rowData.label)
  },
  {
    key: 'feature',
    title: 'Feature',
    dataKey: 'feature',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {
        style: {
          'font-size': '12px'
        }
      }, rowData.feature)
  },
  {
    key: 'alt',
    title: 'ALT Value',
    dataKey: 'alt',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {
        style: {
          'font-size': '12px'
        }
      }, rowData.alt)
  },
  {
    key: 'userLabel',
    title: 'Label',
    dataKey: 'userLabel',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        style: { width: '100%' },
        value: rowData.userLabel,
        disabled: lock.value,
        'onInput': (event) => {
          rowData.userLabel = event.target.value;
          updatePinFeature(rowData.num, rowData.feature, rowData.userLabel, rowData.userColor);
        }
      })
  },
  {
    key: 'direction',
    title: 'Direction',
    dataKey: 'direction',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled: lock.value || rowData.feature != 'GPIO',
        value: rowData.direction,
        'onChange': (event) => {
          rowData.direction = event.target.value;
          chip.setPinDirection(rowData.num, rowData.direction);
        },
        style: { width: '100%' }
      },rowData.feature=='GPIO'?[
        h('option', {label: "Input",value: "GPIO_INPUT_DIRECTION"}),
        h('option', {label: "Output",value: "GPIO_OUTPUT_DIRECTION"}),
        h('option', {label: "Not Specified",value: "GPIO_UNSPECIFIED_DIRECTION"}),
      ]:[])
    }
  },
  {
    key: 'intFlag',
    title: 'Interrupt Status',
    dataKey: 'intFlag',
    width: 120,
    align: 'center',
    cellRenderer: ({ rowData }) => {

      return h('select', {
        disabled: disableMap['intFlag'](rowData),
        value: rowData.intFlag,
        'onChange': (event) => rowData.intFlag = event.target.value,
        style: { width: '100%' }
      }, [h('option', {
        label: "Don't modify",
        value: false
      }),
      h('option', {
        label: "Clear flag",
        value: true
      })])
    }
  },
  {
    key: 'intCfg',
    title: 'Interrupt Configuration',
    dataKey: 'intCfg',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled: disableMap['intFlag'](rowData),
        value: rowData.intCfg,
        'onChange': (event) => rowData.intCfg = event.target.value,
        style: { width: '100%' }
      }, [h('option', {
        label: "ISF Disable",
        value: 'PCTRL_DMA_INT_DISABLED'
      }),
      h('option', {
        label: "DMA Rising edge",
        value: 'PCTRL_DMA_RISING_EDGE',
      }),
      h('option', {
        label: "DMA Falling edge",
        value: 'PCTRL_DMA_FALLING_EDGE',
      }),
      h('option', {
        label: "DMA Both edge",
        value: 'PCTRL_DMA_EITHER_EDGE'
      }),
      h('option', {
        label: "INT Logic 0",
        value: 'PCTRL_INT_LOGIC_ZERO'
      }),
      h('option', {
        label: "INT Rising edge",
        value: 'PCTRL_INT_RISING_EDGE'
      }),
      h('option', {
        label: "INT Falling edge",
        value: 'PCTRL_INT_FALLING_EDGE'
      }),
      h('option', {
        label: "INT Both edge",
        value: 'PCTRL_INT_EITHER_EDGE'
      }),
      h('option', {
        label: "INT Logic 1",
        value: 'PCTRL_INT_LOGIC_ONE'
      })])
    }
  },
  {
    key: 'pullEnable',
    title: 'Pull Enable',
    dataKey: 'pullEnable',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled: disableMap['pullEnable'](rowData),
        value: rowData.pullEnable,
        'onChange': (event) => rowData.pullEnable = Number(event.target.value),
        style: { width: '100%' }
      }, [h('option', {
        label: "Disabled",
        value: 0
      }),
      h('option', {
        label: "Enabled",
        value: 1
      })])
    }
  },
  {
    key: 'pullSelect',
    title: 'Pull Select',
    dataKey: 'pullSelect',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled: disableMap['pullSelect'](rowData),
        value: rowData.pullSelect,
        'onChange': (event) => rowData.pullSelect = event.target.value,
        style: { width: '100%' }
      }, [h('option', {
        label: "Pull-Up",
        value: 'PCTRL_INTERNAL_PULL_UP_ENABLED'
      }),
      h('option', {
        label: "Pull-Down",
        value: 'PCTRL_INTERNAL_PULL_DOWN_ENABLED'
      })])
    }
  },
  {
    key: 'filter',
    title: 'Digtal Filter',
    dataKey: 'filter',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {

      return h('select', {
        disabled: disableMap['filter'](rowData),
        value: rowData.filter,
        'onChange': (event) => rowData.filter = event.target.value,
        style: { width: '100%' }
      }, [h('option', {
        label: "Disabled",
        value: false
      }),
      h('option', {
        label: 'Enabled',
        value: true
      })]
      )
    }
  },
  {
    key: 'digitalWidth',
    title: 'Digital Width',
    dataKey: 'digitalWidth',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        disabled: disableMap['digitalWidth'](rowData),
        value: rowData.digitalWidth,
        'onInput': (event) => rowData.digitalWidth = event.target.value,
        style: { width: '100%' }
      },)
    },
    when: [
      "YTM32B1LE1",
      "YTM32B1LE0",
      "YTM32Z1LS0",
      "YTM32Z1ME0"
    ]
  },
  {
    key: 'passiveFilter',
    title: 'Passive Filter',
    dataKey: 'passiveFilter',
    width: 80,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled: disableMap['passiveFilter'](rowData),
        value: rowData.passiveFilter,
        'onChange': (event) => rowData.passiveFilter = event.target.value,
        style: { width: '100%' }
      }, [h('option', {
        label: "N/A",
        value: false
      })]
      )
    },
    except: [
      "YTM32Z1MD0"
    ]

  },
  {
    key: 'strength',
    title: 'Driver Strength',
    dataKey: 'strength',
    width: 80,
    align: 'center',
    cellRenderer: ({ rowData }) => {

      return h('select', {
        disabled: disableMap['strength'](rowData),
        value: rowData.strength,
        'onChange': (event) => rowData.strength = event.target.value,
        style: { width: '100%' }
      }, [
        h('option', {
          label: "Low",
          value: 'PCTRL_LOW_DRIVE_STRENGTH'
        }),
        h('option', {
          label: "High",
          value: 'PCTRL_HIGH_DRIVE_STRENGTH'
        })]
      )
    },
    except: [
      "YTM32Z1MD0"
    ]
  },
  {
    key: 'rateSelect',
    title: 'Slew Rate',
    dataKey: 'rateSelect',
    width: 80,
    align: 'center',
    cellRenderer: ({ rowData }) => {

      return h('select', {
        disabled: disableMap['rateSelect'](rowData),
        value: rowData.rateSelect,
        'onChange': (event) => rowData.rateSelect = event.target.value,
        style: { width: '100%' }
      }, [
        h('option', {
          label: "Fast",
          value: 'PCTRL_FAST_SLEW_RATE'
        }),
        h('option', {
          label: "Slow",
          value: 'PCTRL_SLOW_SLEW_RATE'
        })]
      )
    },
    except: [
      "YTM32B1LE1",
      "YTM32Z1MD0",
      "YTM32B1MC0",
      "YTM32Z1ME0",
      "YTM32B1MD2"
    ]
  },
  {
    key: 'init',
    title: 'Init Value',
    dataKey: 'init',
    width: 80,
    align: 'center',
    cellRenderer: ({ rowData }) => {

      return h('select', {
        value: rowData.init,
        'onChange': (event) => rowData.init = event.target.value,
        style: { width: '100%' },
        disabled: disableMap['init'](rowData),
      }, [h('option', {
        label: "N/A",
        value: -1
      }),
      h('option', {
        label: "Low",
        value: 0
      }),
      h('option', {
        label: "High",
        value: 1
      })])
    }
  },
  {
    key: 'pinLock',
    title: 'Pin Lock',
    dataKey: 'pinLock',
    width: 80,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        disabled: false,
        value: rowData.pinLock,
        'onChange': (event) => rowData.pinLock = event.target.value,
        style: { width: '100%' }
      }, [h('option', {
        label: "Unlock",
        value: false
      }), h('option', {
        label: "Lock",
        value: true
      })]
      )
    },
    when: [
      "YTM32B1HA0"
    ],
  },
  {
    key: 'userColor',
    title: 'Color',
    dataKey: 'userColor',
    width: 80,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('div', {
        class: 'color-choose-container',
        modelValue: rowData.userColor?rowData.userColor:'#FF0000',
        onMouseleave: (event)=>{
          // console.log('mouseleave', colorPickerClicked)
          if (colorPickerClicked) {
            return;
          }
          let dom = event.target as Element;
          while (dom && dom.className != "color-choose-container") {
            if (dom.parentElement == null) {
              return;
            }
            dom = dom.parentElement;
          }
          
          (dom.children[0] as HTMLSelectElement).hidden = false;
          if (dom.children[1] != undefined) {
            dom.removeChild(globalColorPickerElement);
          }
        },
        onClick: (event) => {
          // console.log('click')
          if (colorPickerClicked) {
            return;
          }
                    let dom = event.target as Element;
          while (dom && dom.className != "color-choose-container") {
            if (dom.parentElement == null) {
              return;
            }
            dom = dom.parentElement;
          }

          color.value = rowData.userColor?rowData.userColor:'#67c23a';
          // console.log('dom', dom)
          if (colorPickerRef.value != undefined && dom.children.length > 1) {
            colorPickerClicked = true;
            // console.log('show', colorPickerClicked)
            colorPickerRef.value.show();
          }
        },
      }, [
        h('div', {
          class: 'color-choose-show',
          style: {
            backgroundColor: rowData.userColor?rowData.userColor:'#67c23a',
          },
          onMouseenter: (event)=>{
            /** 
             * This is used to prevent click and move to another area: 
             * 1. click one node
             * 2. choose one color
             * 3. move to another area
             */
            // console.log('mouseenter')
            if (colorPickerClicked) {
              return;
            }
            ElColorPicker
            let dom = event.target as Element;
            while (dom && dom.className != "color-choose-container") {
              if (dom.parentElement == null) {
                return;
              }
              dom = dom.parentElement;
            }
            if (dom.children.length > 1) {
              return;
            }
            color.value = rowData.userColor?rowData.userColor:'#67c23a';
            (dom.children[0] as HTMLSelectElement).hidden = true;

            dom.appendChild(globalColorPickerElement);
          },
        },[])
      ]);
    }
  },
  {
    key: '',
    title: '',
    dataKey: '',
    width: 10,
    align: 'center',
    cellRenderer: ({ v1,v2,v3,v4,rowIndex }) => {
      return h('div', {
        style: 'color:transparent'
      }, `${rowIndex}`)
    }
  },
  {
    width: 200,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElInput, {
        size: 'small',
        modelValue: tableSearch.value,
        placeholder: "Type to search",
        clearable: true,
        disabled: lock.value,
        'onInput': (value) => tableSearch.value = value
      })
    },
    cellRenderer: ({ rowData, rowIndex }) => {

      return h(ElButton, {
        size: 'small',
        type: "danger",
        disabled: lock.value,
        'onClick': () => deleteFeature(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]

const columns = ref<Column<any>[]>([])

interface PinCfg {
  base: string,
  gpioBase: string,
  num: string;
  label: string;
  feature: string;
  alt: string;
  intCfg: string;
  userLabel?: string;
  direction: string;
  pullEnable: number;
  pullSelect: string;
  init: number;
  filter: boolean;
  passiveFilter?: boolean;
  pinLock?: boolean;
  strength?: string;
  intFlag: boolean;
  rateSelect?: string,
  userColor?: string,
  digitalWidth?: number
}

interface PinCfgArrayType {
  name: string,
  pins: PinCfg[]
}

interface PinsDataType {
  ver: string,
  desc: string,
  curIdx: string,
  cfgs: PinCfgArrayType[]
}

const lock = ref(false)

const chipName = ref('YTM32B1ME0')

const plugin = new Plugin({
  lockChange: (val) => {
    lock.value = val;
    if (chip) {
      chip.lockChange(val)
    }
  }
});

const tableSearch = ref("");
const featureMapEmpty: Record<string, string[]> = {};
let featureMap: Record<string, string[]> = {};

const searchRef = ref(null);
const search1Ref = ref();
const searchValue = ref("");
const searchPin = ref("");
const searchList = ref<string[]>([]);
const pinList = ref<PinDef[]>([]);


let dragContainer: HTMLDivElement | null | undefined;

const pinsData = ref<PinsDataType>({
  ver: "1_3_3",
  desc: "multi pins cfg",
  cfgs: [{
    name: "InitConfigArr0",
    pins: []
  }],
  curIdx: "0"
})

const activeInst = computed(()=>{
  return parseInt(pinsData.value.curIdx);
})

const pinsCfgArray = computed(()=>{
  return pinsData.value.cfgs[activeInst.value].pins
})

let chip: LQFP;
let buttons: HelpButtons;

watch(()=>pinsData.value.curIdx, (newVal: string, oldVal: string)=>{
  /* Delete old feature list in chip */
  const oldPins = pinsData.value.cfgs[parseInt(oldVal)].pins;
  for (const pin of oldPins) {
    chip.deletePinFeature(pin.num);
  }
  reInitFeatureTable();
})

watch(pinsData, () => {
  if (initDone) {
    plugin.dataChange(pinsData.value);
  }
}, {deep: true})

/* table search compute*/
const filterTableData = computed(() => {
  const featureTable = pinsCfgArray.value;
  return featureTable.filter(
    (data) =>
      !tableSearch.value ||
      JSON.stringify(data)
        .toLowerCase()
        .includes(tableSearch.value.toLowerCase())
  )
});

/* table resize*/
// eslint-disable-next-line no-undef
interact(".table").resizable({
  // resize from all edges and corners
  edges: { left: false, right: false, bottom: false, top: true },

  listeners: {
    move(event) {
      tableHeight.value += -event.deltaRect.top;
      resizeHandler();
    },
  },
  modifiers: [
    // keep the edges inside the parent
    // interact.modifiers.restrictEdges({
    //   outer: 'parent'
    // }),

    // minimum size
    // eslint-disable-next-line no-undef
    interact.modifiers.restrictSize({
      min: { height: 200 },
    }),
  ],

  inertia: true,
});


function getGroup(name: string) {
  const p = /PT(.*?)_/gm
  const res = p.exec(name)
  if (res != null) {
    return res[1]
  }
}
let warnLabelMsg:undefined|Record<string, {title: string, msg: string, test: (val:string)=>boolean}> = undefined;
const getWarningObj = () => {
  // console.log('JSON.stringify(chipName.value)', JSON.stringify(chipName.value));
  switch(chipName.value) {
    case 'YTM32Z1ME0': {
      const warnLabelMsg = {
        'PTA_0': {
          title: 'SWD CLK Pin Config',
          msg: "The MCU can't debug by JTAG or SWD!",
          test: (val)=>{
            return !val.endsWith('SWD_CLK');
          }
        },
        'PTA_1': {
          title: 'SWD IO Pin Config',
          msg: "The MCU can't debug by JTAG or SWD!",
          test: (val)=>{
            return !val.endsWith('SWD_IO');
          }
        },
        'PTA_2': {
          title: 'Reset Pin Config',
          msg: "The reset pin config to other func, the mcu can't be reset by extern reset",
          test: (val)=>{
            return !val.endsWith('Reset_b');
          }
        }
      }
      return warnLabelMsg;
    }
    case 'YTM32Z1MD0': {
      const warnLabelMsg = {
        'PTA_5': {
          title: 'Reset Pin Config',
          msg: "The reset pin config to other func, the mcu can't be reset by extern reset",
          test: (val)=>{
            return val.toLowerCase(val) != 'reset_b';
          }
        },
        'PTA_6': {
          title: 'SWD IO Pin Config',
          msg: "The MCU can't debug by JTAG or SWD!",
          test: (val)=>{
            return !val.endsWith('SWD_IO');
          }
        },
        'PTA_7': {
          title: 'SWD CLK Pin Config',
          msg: "The MCU can't debug by JTAG or SWD!",
          test: (val)=>{
            return !val.endsWith('SWD_CLK');
          }
        }
      }
      return warnLabelMsg;
    }
    default: {
      const warnLabelMsg = {
        'PTA_5': {
          title: 'Reset Pin Config',
          msg: "The reset pin config to other func, the mcu can't be reset by extern reset",
          test: (val)=>{
            return val.toLowerCase(val) != 'reset_b';
          }
        },
        'PTC_4': {
          title: 'SWD CLK Pin Config',
          msg: "The MCU can't debug by JTAG or SWD!",
          test: (val)=>{
            return !val.endsWith('SWD_CLK');
          }
        },
        'PTA_4': {
          title: 'SWD IO Pin Config',
          msg: "The MCU can't debug by JTAG or SWD!",
          test: (val)=>{
            return !val.endsWith('SWD_IO');
          }
        },
      }
      return warnLabelMsg;
    }
  }
}

async function featureChange(num: string, lable: string, val: string, alt: string) {
  let err = false;
  let direction = '';
  /* open direction chose */
  try {
    if (val && val == 'GPIO') {
      await ElMessageBox({
        title: "Direction Choose",
        message: h(DirSelect, {
          dirList: [
            {'value':'GPIO_INPUT_DIRECTION','label':'Input'},
            {'value':'GPIO_OUTPUT_DIRECTION','label':'Output'},
            {'value':'GPIO_UNSPECIFIED_DIRECTION','label':'Not Specified'},
          ],
          onChange: (val) => {
            direction = val;
          },
        }),
        showCancelButton: true,
        confirmButtonText: "OK",
        cancelButtonText: "Cancel",
      })
    }
    /* remove the feature from the table */
    let tableIndex = -1
    const featureTable = pinsCfgArray.value;
    for (let i = 0; i < featureTable.length; i++) {
      if (num == featureTable[i].num) {
        /* remove pins from old feature*/
        const oldFeature = featureTable[i].feature;
        const index = featureMap[oldFeature].indexOf(num);
        if (index > -1) {
          featureMap[oldFeature].splice(index, 1);
        }
        //featureTable.splice(i, 1);
        tableIndex = i
        break;
      }
    }

    if (val != "") {
      /* add the pin to featureMap */
      if (val != "GPIO" && featureMap[val].length > 0 && val != "Disable") {
        err = true;
      } else {
        featureMap[val].push(num);
      }

      if (err == false) {
        if (warnLabelMsg && Object.keys(warnLabelMsg).includes(lable) && warnLabelMsg[lable].test(val)) {
          ElNotification({
            title: warnLabelMsg[lable].title,
            message: warnLabelMsg[lable].msg,
            type: 'warning',
            duration: 0,
          })
        }

        if (tableIndex == -1) {
          let changeAttr = {};
          switch(chipName.value) {
            case "YTM32B1HA0": {
              changeAttr = {
                pinLock: false,
                rateSelect: 'PCTRL_FAST_SLEW_RATE',
                strength: 'PCTRL_LOW_DRIVE_STRENGTH',
                passiveFilter: false,
              }
              break;
            }
            case "YTM32Z1MD0": {
              changeAttr = {}
              break;
            }
            case "YTM32Z1ME0":
            case "YTM32B1LE1": {
              changeAttr = {
                digitalWidth: 0,
                strength: 'PCTRL_LOW_DRIVE_STRENGTH',
                passiveFilter: false,
              }
              break;
            }
            case "YTM32Z1LS0":
            case "YTM32B1LE0": {
              changeAttr = {
                digitalWidth: 0,
                rateSelect: 'PCTRL_FAST_SLEW_RATE',
                strength: 'PCTRL_LOW_DRIVE_STRENGTH',
                passiveFilter: false,
              }
              break;
            }
            case "YTM32B1MC0": {
              changeAttr = {
                strength: 'PCTRL_LOW_DRIVE_STRENGTH',
                passiveFilter: false,
              }
              break;
            }
            default: {
              changeAttr = {
                rateSelect: 'PCTRL_FAST_SLEW_RATE',
                strength: 'PCTRL_LOW_DRIVE_STRENGTH',
                passiveFilter: false,
              }
              break;
            }
          }
          /* add the feature to featureTable*/
          featureTable.push({
            base: 'PCTRL' + getGroup(lable),
            gpioBase: 'GPIO' + getGroup(lable),
            num: num,
            label: lable,
            feature: val,
            alt: alt,
            intCfg: 'PCTRL_DMA_INT_DISABLED',
            direction: direction,
            pullEnable: 0,
            pullSelect: '',
            filter: false,
            intFlag: false,
            init: 0,
            userLabel: '',
            ...changeAttr
          });
        } else {
          featureTable[tableIndex].feature = val
          featureTable[tableIndex].direction = direction
          featureTable[tableIndex].alt = alt
        }
        
        chip.setPinDirection(num, direction);
        
      } else {
        ElMessage.error(
          `Feautre ${val} has been set up in PIN ${featureMap[val].join(",")}`
        );
        chip.deletePinFeature(num);
      }
    } else {
      //delete the feature
      if (tableIndex != -1) {
        featureTable.splice(tableIndex, 1)
      }
      chip.setPinDirection(num, '');
    }
  } catch (e) {
    err = true;
  }
  return !err
}

let initDone = false;

const deleteFeature = (index: number, row: PinCfg) => {
  const findex = featureMap[row.feature].indexOf(row.num);
  if (findex > -1) {
    featureMap[row.feature].splice(findex, 1);
  }
  chip.deletePinFeature(row.num);
  pinsData.value.cfgs[activeInst.value].pins.splice(index, 1);
};

function searchPinChange(val: string) {
  if (val != "") {
    chip.closeHighLight();
    chip.highLightPin(val);
  }
}
function searchPinClear() {
  chip.closeHighLight();
}


function searchChange(val: string) {
  if (val != "") {
    chip.closeHighLight();
    chip.highLightFeature(val);
  }
}

function searchClear() {
  chip.closeHighLight();
}

function resizeHandler() {
  winH.value = window.innerHeight
  winW.value = window.innerWidth
  chip.resize(window.innerWidth, window.innerHeight - tableHeight.value - 50);
  buttons.resize(window.innerWidth);
}


/**
 * @brief This function is used to judge the data is old pin version or not
 */
function isOldVersionCfg(data: any) {
  if (Array.isArray(data)) {
    return true;
  }
  return false;
}

/**
 * @brief Check the init data is valid or not.
 * 
 * @detail The init data is considered as invalid as fellows:
 * - pin's num is invalid.
 * - pin's num and pin's label not match.
 * - pin's num and pin's feature not match.
 * - pin's feature(except GPIO) is repeated.
 */
function checkAllCfg(pinConfigs: PinCfgArrayType[]) {
  enum PinCfgErrorType {
    OK,
    PinAndNumNotMatch,
    PinLabelNotMatch,
    PinFeatureNotMatch,
    FeatureRepeated
  }

  /** This value is comes from .js filter to only contain label */
  const allLabel = chip.getAllLabel();
  let pinCfgIdx = 0;
  const validPinCfgArray: PinCfgArrayType[] = [];
  
  /* feature and pin num check */
  const isPinNumValid = (v)=>{
    return v>=0 && v<allLabel.length;
  }
  
  /* use arrow function to decrease the cyclic complexity */
  const checkOneCfg = (name: string, pins: PinCfg[])=>{
    let checkRes = PinCfgErrorType.OK;
    const featureDict = {};
    let pinIdx = 0;
    for (pinIdx;pinIdx < pins.length; pinIdx++) {
      const pin = pins[pinIdx];
      const num = parseInt(pin.num) - 1;
      if (!isPinNumValid(num)) {
        checkRes = PinCfgErrorType.PinAndNumNotMatch
        break;
      }
      if (pin.label != allLabel[num]) {
        checkRes = PinCfgErrorType.PinLabelNotMatch
        break;
      }
      if (!chip.getFeature(num).includes(pin.feature)) {
        checkRes = PinCfgErrorType.PinFeatureNotMatch
        break;
      }
      if (pin.feature != 'GPIO' && pin.feature != 'Disable' && featureDict[pin.feature]) {
        checkRes = PinCfgErrorType.FeatureRepeated
        break;
      }
      featureDict[pin.feature] = 1;
    }

    let errorMsg = `The pin configuration '${name}' has been removed`;
    switch (checkRes) {
      case PinCfgErrorType.OK:
        validPinCfgArray.push(pinConfigs[pinCfgIdx]);
        break;
      case PinCfgErrorType.PinAndNumNotMatch:
        errorMsg = `The pin configuration '${name}' has been removed because the 'num' value of its ${pinIdx}th item is not valid for the current chip.`
        break;
      case PinCfgErrorType.PinLabelNotMatch:
        errorMsg = `The pin configuration '${name}' has been removed because the 'Label' value of its ${pinIdx}th item is not match.`
        break;
      case PinCfgErrorType.PinFeatureNotMatch:
        errorMsg = `The pin configuration '${name}' has been removed because the 'Feature' value of its ${pinIdx}th item is not match.`
        break;
      case PinCfgErrorType.FeatureRepeated:
        errorMsg = `The pin configuration '${name}' has been removed because the 'Feature' value of its ${pinIdx}th item repeated.`
        break;
    }
    return {checkRes, errorMsg};
  }

  for (pinCfgIdx; pinCfgIdx < pinConfigs.length; pinCfgIdx++) {
    const pins = pinConfigs[pinCfgIdx].pins;
    const name = pinConfigs[pinCfgIdx].name;
    const {checkRes, errorMsg}  = checkOneCfg(name, pins);

    if (checkRes != PinCfgErrorType.OK) {
      ElNotification({
        title: 'Warning',
        message: errorMsg,
        type: 'warning',
        duration:  10000
      })
    }

    if (validPinCfgArray.length == 0) {
      pinsData.value.cfgs = [{
        name: 'InitConfigArr0',
        pins: []
      }]
    } else {
      pinsData.value.cfgs = validPinCfgArray;
    }
  }
}

/**
 * @brief This function will re-init feature table's value.
 * 
 * @detail This function will called in init and pin's activeInst change.
 */
function reInitFeatureTable() {
  const featureTable = pinsCfgArray.value;
  featureMap = JSON.parse(JSON.stringify(featureMapEmpty));
  
  for (const i in featureTable) {
    chip.setPinFeature(
      featureTable[i].num,
      featureTable[i].feature,
      featureTable[i].userLabel || "",
      featureTable[i].userColor || undefined,
      false
    );
    if (featureTable[i].feature == 'GPIO') {
      chip.setPinDirection(featureTable[i].num, featureTable[i].direction);
    }
    featureMap[featureTable[i].feature].push(
      featureTable[i].num
    );
  }
}

/**
 * @brief This function is used to update pin's feature, when pin's userLabel is changed from table,
 *        this function will be called to update canvas pin's label.
 */
function updatePinFeature(num: string, feature: string, userLabel: string, userColor:string|undefined) {
  chip.setPinFeature(
    num,
    feature,
    userLabel,
    userColor,
    false
  );
  /**
   * Note: this function will be called only when userLabel change, So there is no need to update 
   *       featureMap 
   */
}

/**
 * @brief This function is used to add event listener for pin table row.
 * 
 * @details This function will add `onmouseover` and `onmouseout` event listener, when pin table row
 *          is hover, chip canvas will highlight the relative pin.
 * 
 * @note Hovering over the table-right element will have no effect, as no event listener has been
 *       attached to it.
 */
let lastIdx = "";
function addHoverEvent() {
  const elMain = document.getElementsByClassName('el-table-v2__main')[0];
  const el = elMain.getElementsByClassName('el-table-v2__body')[0];
  const tableContainer = el.querySelector('div')?.querySelector('div');
  if (tableContainer == undefined) {
    return;
  }

  tableContainer.onmouseover = e=>{
    let targetDom = (e.target as HTMLElement);
    while (!targetDom.classList.contains('table-row') && targetDom.parentElement) {
      targetDom = targetDom.parentElement;
    }
    if (targetDom !== tableContainer) {     
      const indexString = 
        (targetDom.children[targetDom.children.length-2] as HTMLDivElement)?.textContent || '';
      const targetIndex = parseInt(indexString);
      if (lastIdx != "") {
        chip.hoverClosePin(lastIdx);
      }
      chip.hoverHighLightPin(pinsCfgArray.value[targetIndex].num);
      lastIdx = pinsCfgArray.value[targetIndex].num;
      // setTimeout(()=>{
      //   chip.hoverClosePin(pinsCfgArray.value[targetIndex].num);
      // }, 6000)
    }
  }

  tableContainer.onmouseout = e=>{
    let targetDom = (e.target as HTMLElement);
    while (!targetDom.classList.contains('table-row') && targetDom.parentElement) {
      targetDom = targetDom.parentElement;
    }
    if (targetDom !== tableContainer) {
      const indexString = 
        (targetDom.children[targetDom.children.length-2] as HTMLDivElement)?.textContent || '';
      const targetIndex = parseInt(indexString);
      chip.hoverClosePin(pinsCfgArray.value[targetIndex].num);
    }
  }
}

/**
 * @brief This function is used to add drag event listener for pin table row.
 * 
 * @details The pin table will update based on reactive values, so there is no need to manually
 *          change the DOM elements.
 */
function addDragEvent() {
  let dragIndex = -1;
  const elMain = document.getElementsByClassName('el-table-v2__main')[0];
  const el = elMain.getElementsByClassName('el-table-v2__body')[0];
  dragContainer = el.querySelector('div')?.querySelector('div');
  if (dragContainer == undefined) {
    return;
  }

  let dragDom;
  let timeFunc: NodeJS.Timeout | undefined;
  dragContainer.ondragstart = e=>{
    const target = e.target as HTMLElement;
    /** Only .table-drag-area can be drag */
    const draggableArea = target.querySelector('.table-drag-area');
    const draggableRect = draggableArea!.getBoundingClientRect();
    if (e.x < draggableRect.x || e.x > draggableRect.right) {
      e.preventDefault();
      return;
    }
    dragDom = e.target;
    const indexDom = (dragDom.children[dragDom.children.length-2] as HTMLDivElement).textContent;
    dragIndex = parseInt(indexDom||'');
    chip.hoverClosePin(pinsCfgArray.value[dragIndex].num);
    dragDom.classList.add('table-moving');
  }

  dragContainer.ondragenter = e=>{
    let targetDom = (e.target as HTMLElement);
    while (!targetDom.classList.contains('table-row') && targetDom.parentElement) {
      targetDom = targetDom.parentElement;
    }
    if (e.dataTransfer) {
      e.preventDefault();
      e.dataTransfer.dropEffect = "move"
    }
    if (targetDom !== dragContainer) {
      const indexDom = (targetDom.children[targetDom.children.length-2] as HTMLDivElement).textContent;
      const targetIndex = parseInt(indexDom||'');
      if (targetIndex == dragIndex) {
        /* changed */
        return;
      }
      if (targetIndex != -1) {
        
        if (timeFunc != undefined) {
          clearTimeout(timeFunc);
        }
        timeFunc = setTimeout(()=>{
          const pinsArray = pinsCfgArray.value;
          const pin = pinsArray[dragIndex];
          pinsArray.splice(dragIndex, 1);
          pinsArray.splice(targetIndex, 0, pin);
          dragIndex = targetIndex;
          dragDom.classList.remove('table-moving');
          dragDom = targetDom;
          dragDom.classList.add('table-moving');
          timeFunc = undefined;
        }, 200)
      }
    }
  }
  dragContainer.ondragleave = e=>{
    let targetDom = (e.target as HTMLElement);
    while (!targetDom.classList.contains('table-row') && targetDom.parentElement) {
      targetDom = targetDom.parentElement;
    }
    if (e.dataTransfer) {
      e.preventDefault();
      e.dataTransfer.dropEffect = "move"
    }
    if (targetDom !== dragContainer) {
      const indexDom = (targetDom.children[targetDom.children.length-2] as HTMLDivElement).textContent;
      const targetIndex = parseInt(indexDom||'');
      if (targetIndex != dragIndex) {
        /* changed */
        return;
      }
      if (targetIndex != -1) {
        if (timeFunc != undefined) {
          clearTimeout(timeFunc);
          timeFunc = undefined;
        }
      }
    }
  }


  dragContainer.ondragend = e=>{
    for (const pin of pinsCfgArray.value) {
      chip.hoverClosePin(pin.num);
    }
    if (timeFunc != undefined) {
      clearTimeout(timeFunc);
      timeFunc = undefined;
    }
    dragDom.classList.remove('table-moving');

  }

  dragContainer.ondragover = e=>{
    return false;
  }

}


/**
 * @brief This function is used to get table col header.
 * 
 * @details Different chip contains various pin function, this function will filter the table header
 *          by chipName.
 */
function getTableColum(chipName: string) {
  columns.value = allColumn.filter(item=>{
    let flag = true;
    if (item.when != undefined) {
      flag = flag && item.when.includes(chipName);
      
    }
    if (item.except != undefined) {
      flag = flag && !item.except.includes(chipName);
    }
    return flag;
  });

}

/**
 * This function is used to determine the encapsulation (type) for a part number.
 * @param chipName 
 */
function getPinEncapsulation(chipName: string) {
  if (chipName.length < 17) return 'LQFP';
  switch (chipName[14]) {
    case 'L': return 'LQFP';
    case 'F': return 'QFN';
    case 'T': return 'TSSOP';
  }
  return 'LQFP';
}

onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  addHoverEvent();
  addDragEvent();
  const stage = new Konva.Stage({
    container: "chip-view-container",
    width: winW.value,
    height: winH.value - tableHeight.value - 50,
  });
  const layer: Konva.Layer = new Konva.Layer();
  plugin
    .getInitValue()
    .then((initData) => {
      const pinDefs = readPinDef(initData.projectInfo.partNumber)
      chipName.value = initData.projectInfo.partNumber.substr(0, 10);
      getTableColum(chipName.value);
      const encapsulation = getPinEncapsulation(initData.projectInfo.partNumber);
      // add canvas element
      stage.add(layer);
      chip = new LQFP(layer, chipName.value, pinDefs, featureChange, lock.value, encapsulation);
      
      const allFeature = chip.getAllFeature();

      searchList.value = allFeature;
      pinList.value = chip.pins
      for (const i in allFeature) {
        featureMapEmpty[allFeature[i]] = [];
      }

      featureMap = JSON.parse(JSON.stringify(featureMapEmpty));
      const buttonStage = new Konva.Stage({
        container: "button",
        width: winW.value,
        height: 50,
      });

      const buttonLayer = new Konva.Layer();
      buttonStage.add(buttonLayer);
      buttons = new HelpButtons(buttonLayer, searchRef.value, search1Ref.value, chip);

      if (initData.pluginData != undefined) {
        if(initData.pluginData.lock){
          lock.value=true
          chip.lockChange(true)
        }
        /** convert old version to a valid struct */
        if (isOldVersionCfg(initData.pluginData.data)) {
          pinsData.value.cfgs[0].name = 'InitConfigArr0'
          pinsData.value.cfgs[0].pins = initData.pluginData.data
        } else {
          pinsData.value = initData.pluginData.data
        }
        checkAllCfg(JSON.parse(JSON.stringify(pinsData.value.cfgs)));
        plugin.dataChange(pinsData.value);
        reInitFeatureTable();
      }
      warnLabelMsg = getWarningObj();
      initDone = true;
    }).catch((e: any) => {
      console.error(e)
    })
  setTimeout(()=>{
    reInitFeatureTable();
    getColorPicker();
  }, 100)
});

function handleAddClick() {
  function pinCfgNameValid(name: string) {
    name = name.trimEnd();
    if (name == '') {
      return false
    }
    if (name.includes(' ')) {
      return false
    }
    for (const item of pinsData.value.cfgs) {
      if (item.name == name) {
        return false
      }
    }
    return true
  }

  ElMessageBox.prompt('Please input your pins config name', 'Tip', {
    confirmButtonText: 'OK',
    cancelButtonText: 'Cancel',
    inputValidator: pinCfgNameValid,
    inputPlaceholder: 'Pins Config Name',
    inputErrorMessage: 'Invalid config name',
    inputValue: 'InitConfigArr' + pinsData.value.cfgs.length
  })
    .then(({ value }) => {
      pinsData.value.cfgs.push({
        name: value,
        pins:[]
      })
      pinsData.value.curIdx = (pinsData.value.cfgs.length - 1).toString()
    })
    .catch(() => {
      null
    })
}

function handleDelClick() {
  if (activeInst.value >= 0 && activeInst.value < pinsData.value.cfgs.length) {
    if (activeInst.value != 0) {
      const deleteCfgIdx = JSON.parse(JSON.stringify(activeInst.value));
      pinsData.value.curIdx = '0'
      nextTick(()=>{
        pinsData.value.cfgs.splice(deleteCfgIdx, 1);
      })
    } else {
      /* Delete old feature list in chip */
      const oldPins = pinsData.value.cfgs[0].pins;
      for (const pin of oldPins) {
        chip.deletePinFeature(pin.num);
      }
      pinsData.value.cfgs.splice(0, 1);
      reInitFeatureTable();
    }
  }
}

function handleExport() {
  console.log(pinsData.value)
}

</script>
<style scoped>
.pins {
  overflow: hidden;
  position: relative;
  height: 100vh;
}

.pins>.pins-cfg-add-container {
  position: absolute;
  left: 10px;
  right: 10px;
  top: 10px;
  z-index: 2;
  opacity: 1!important;
}

.search {
  position: absolute;
}

.search1{
  position: absolute;
}

.table {
  height: v-bind(tableHeight + "px");
  width: v-bind(winW - 10 + "px");
}
</style>

<style>
.table-row {
  .table-drag-area {
    color: transparent;
    padding-left: 20px;
  }

  .table-drag-area-disabled {
    color: transparent;
    padding-left: 20px;
  }
}

.table-row:hover {
  .table-drag-area {
    color: var(--el-text-color-primary);
    opacity: 0.7;
    
  }
  .table-drag-area-disabled {
    color: var(--el-text-color-primary);
    opacity: 0.2; 
  }
}

.table-row.table-moving {
  opacity: 0.7;
  background-color: var(--el-color-info-light-3)
}


.color-choose-container {
  /* width: 30px; */
  display: flex;
  align-items: center;
  justify-content: center;
}

.color-choose-show {
  background-color: red;
  width: 10px;
  height: 10px;
  border-radius: 2px;
}

.el-color-picker__trigger {
  border: none;
}
</style>