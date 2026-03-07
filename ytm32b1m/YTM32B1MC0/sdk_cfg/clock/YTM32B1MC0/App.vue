<template>
  <div id="button" />
  <div class="clock">
    <div style="padding-top: 10px;margin-left:10px">
      <el-select v-model="data.activeInst">
        <el-option
          v-for="(item, index) in data.clockArray"
          :key="index"
          :label="item.name"
          :value="Number(index)"
        />
      </el-select>
      <el-button
        style="margin-left:10px"
        @click="addClock"
        :disabled="lock"
      >
        Add
      </el-button>
      <el-button
        type="danger"
        :disabled="data.clockArray.length == 1 || lock"
        plain
        @click="deleteClock"
      >
        Delete
      </el-button>
    </div>
    <div class="clockLeft">
      <div id="container" />
    </div>
    <div id="clockRight">
      <el-table-v2
        :columns="columns"
        :data="data.clockArray[data.activeInst].data.periClocks"
        :estimated-row-height="30"
        :width="leftRight"
        :header-height="30"
        :height="winH"
        fixed
        class="table"
      />
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, onBeforeUnmount, watch, toRaw, h, reactive, toRef } from "vue";
import { Clock , defaultClockData} from './clock.ts'
import Konva from "konva";
import { Plugin } from "plugin";
import type { Column } from '@element-plus/components/table-v2'
import { ElMessageBox } from "element-plus";
import {makeResizable} from './resize.ts'
import { getVarColor } from "./base.ts";
// import interact from "interactjs";
const plusIcon =
  '<svg t="1661419115069" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2335" width="200" height="200"><path d="M1005.312 914.752l-198.528-198.464A448 448 0 1 0 0 448a448 448 0 0 0 716.288 358.784l198.4 198.4a64 64 0 1 0 90.624-90.432zM384 384H256v128h128v128h128V512h128V384H512V256H384v128z m64 384A320 320 0 1 1 448 128a320 320 0 0 1 0 640z" fill="#409EFF" p-id="2336"></path></svg>';
const minIcon =
  '<svg t="1661420224594" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="3238" width="200" height="200"><path d="M1005.312 914.752l-198.528-198.464A448 448 0 1 0 0 448a448 448 0 0 0 716.288 358.784l198.4 198.4a64 64 0 1 0 90.624-90.432zM448 767.936A320 320 0 1 1 448 128a320 320 0 0 1 0 640zM256 384v128h384V384H256z" fill="#409EFF" p-id="3239"></path></svg>';
const fullScreenIcon =
  '<svg t="1661420254009" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="4177" width="200" height="200"><path d="M213.333333 213.333333h213.333334V128H170.666667a42.666667 42.666667 0 0 0-42.666667 42.666667v256h85.333333V213.333333zM170.666667 896h256v-85.333333H213.333333v-213.333334H128v256a42.666667 42.666667 0 0 0 42.666667 42.666667z m725.333333-42.666667v-256h-85.333333v213.333334h-213.333334v85.333333h256a42.666667 42.666667 0 0 0 42.666667-42.666667zM597.333333 213.333333h213.333334v213.333334h85.333333V170.666667a42.666667 42.666667 0 0 0-42.666667-42.666667h-256v85.333333z" fill="#409EFF" p-id="4178"></path></svg>';


class HelpButtons {
  parent: Konva.Layer;
  buttonsGroup: Konva.Group;
  clock: any
  constructor(parent: Konva.Layer, clock: ClockMC0) {
    this.parent = parent
    this.clock = clock
    const stage = this.parent.getStage()
    this.buttonsGroup = new Konva.Group({
      x: stage.width() / 2,
      y: 10,
      draggable: false,
      rotation: 0,
    });
    this.parent.add(this.buttonsGroup);
    this.createButton(- 100, plusIcon, () => {
      this.clock.changeScale(0.05);
    });
    this.createButton(0, fullScreenIcon, () => {
      this.clock.fullScreen();
    });
    this.createButton(100, minIcon, () => {
      this.clock.changeScale(-0.05);
    });

  }
  changeClock(clock) {
    this.clock = clock
  }
  createButton(x: number, svg: string, action: any, disable = false, id: string | undefined = undefined) {
    const url = 'data:image/svg+xml;base64,' + window.btoa(svg);
    Konva.Image.fromURL(url, (image: any) => {
      image.draggable(false);

      image.setPosition({ x: x, y: 0 })
      image.width(30)
      image.height(30)
      if (id != undefined) {
        image.setAttr('id', id)
      }
      if (disable == false) {
        const rect = new Konva.Rect({
          x: x - 5,
          y: -5,
          width: 40,
          height: 40,
          draggable: false,
          fill: getVarColor("--bg-color"),
          strokeWidth: 0,
          stroke: getVarColor('--el-color-primary'),
          opacity: 1
        })
        this.buttonsGroup.add(rect)
        image.on('mouseenter', () => {
          rect.setAttr('strokeWidth', 2)
        })
        image.on('mouseleave', () => {
          rect.setAttr('strokeWidth', 0)
        })
        image.on('click', () => {
          if (action != null) {
            action()
          }

        })
      }
      this.buttonsGroup.add(image);
    });
  }
  resize(x: number) {
    const stage = this.parent.getStage()
    stage.width(x)

    this.buttonsGroup.setPosition({
      x: x / 2,
      y: 10
    })
    const stageBox = stage.container().getBoundingClientRect()
    const areaPosition = {
      x: stageBox.left + x / 2 + 150,
      y: stageBox.top + 10,
    };
  }
}

function CheckIPCItem(rowData){
  if(rowData.busClock[Number(rowData.busSrc)]=='SLOW_BUS_CLK'){
    if(rowData.periClock){
      if(rowData.periClock[Number(rowData.periSrc)] == 'CLK_SRC_FIRC'){
        if(rowData.div == 0){
          plugin.addLog('error', `IPC-${rowData.name}`,
          `max clock frequency is 40MHz, but you select ${rowData.periClock[Number(rowData.periSrc)]} which is ${80/(Number(rowData.div)+1)}Mhz`
        )
        }else{
          plugin.deleteLog('error', `IPC-${rowData.name}`)
        }
      }else{
        plugin.deleteLog('error', `IPC-${rowData.name}`)
      }
    }
  }else{
    plugin.deleteLog('error', `IPC-${rowData.name}`)
  }
}

function CheckIPC(periData){
  for(const rowData of periData){
    CheckIPCItem(rowData)
  }
}

const columns: Column<any>[] = [
  {
    key: 'name',
    title: 'Name',
    dataKey: 'name',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {

        style: {
          'font-size': '12px'
        }
      }, rowData.name)
  },
  {
    key: 'userCtrl',
    title: 'UserCtrl',
    dataKey: 'userCtrl',
    width: 70,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'checkbox',
        disabled: lock.value,
        checked: rowData.userCtrl,
        'onInput': (event) => {
          rowData.userCtrl = event.target.checked
        }
      })
  },
  {
    key: 'gate',
    title: 'Gate',
    dataKey: 'gate',
    width: 60,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'checkbox',
        disabled: lock.value,
        checked: rowData.gate,
        'onInput': (event) => {
          rowData.gate = event.target.checked
        }
      })
  },
  {
    key: 'div',
    title: 'Divider',
    dataKey: 'div',
    width: 70,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      if (rowData.periClock) {
        return h('select', {
          size: 'small',
          disabled: lock.value,
          value: rowData.div,
          'onChange': (event) => {
            rowData.div = Number(event.target.value)
            CheckIPCItem(rowData)
          },
          style: { width: '100%' }
        }, Array.from(Array(16).keys()).map((i) => {
          return h('option', {
            label: i + 1,
            value: Number(i)
          })
        }))
      }

    }
  },
  // {
  //   key: 'busSrc',
  //   title: 'Bus Clock',
  //   dataKey: 'busSrc',
  //   width: 150,
  //   align: 'center',
  //   cellRenderer: ({ rowData }) => {
  //     if (rowData.busClock.length > 1) {
  //       return h(ElSelect, {
  //         size: 'small',
  //         modelValue: rowData.busSrc,
  //         'onChange': (value) => rowData.busSrc = value,
  //         style: { width: '100%' }
  //       }, () => {
  //         const o: any = []
  //         for (const i in rowData.busClock) {
  //           const vn = h(ElOption, {
  //             label: rowData.busClock[i],
  //             value: Number(i)
  //           })
  //           o.push(vn)
  //         }
  //         return o
  //       })
  //     } else {
  //       return h('span', { style: { 'font-size': '12px' } }, rowData.busClock.join(','))
  //     }
  //   }
  // },
  {
    key: 'periSrc',
    title: 'Peripheral Functional Clock',
    dataKey: 'periSrc',
    width: 250,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      if (rowData.periClock) {
        return h('select', {
          size: 'small',
          value: rowData.periSrc,
          disabled: lock.value,
          'onChange': (event) => {
            rowData.periSrc = Number(event.target.value)
            CheckIPCItem(rowData)
          },
          style: { width: '100%' }
        }, rowData.periClock.map((val, index) => {
          return h('option', {
            label: val,
            value: Number(index)
          })
        }))
      }

    }
  },
]


const lock = ref(false)
const plugin = new Plugin({
  lockChange: (val) => {
    lock.value = val;
    if (clock != undefined) {
      clock.lockChange(val)
    }
  }
});


/*Stage and Table height define*/
const winW = ref(window.innerWidth != 0 ? window.innerWidth - 10 : 1000);
const winH = ref(window.innerHeight != 0 ? window.innerHeight : 1000);
let clock: any = undefined;
let layer: Konva.Layer;
let buttons: HelpButtons;
const leftRight = ref(500)
const data = reactive({
  activeInst: 0,
  version: "1",
  clockArray: [{
    name: 'clock_config0',
    data: defaultClockData
  }]
})

const activeInst = toRef(data, 'activeInst')



function clockNameValid(name: string) {
  if (name == '') {
    return false
  }
  for (const item of data.clockArray) {
    if (item.name == name) {
      return false
    }
  }
  return true
}

function addClock() {
  ElMessageBox.prompt('Please input your clock config name', 'Tip', {
    confirmButtonText: 'OK',
    cancelButtonText: 'Cancel',
    inputValidator: clockNameValid,
    inputPlaceholder: 'Clock Config Name',
    inputErrorMessage: 'Invalid config name',
    inputValue: ''
  })
    .then(({ value }) => {
      data.clockArray.push(JSON.parse(JSON.stringify({
        name: value,
        data: defaultClockData
      })))
      data.activeInst = data.clockArray.length - 1
    })
    .catch(() => {
      null
    })
}

function deleteClock() {
  clock.clearLogs()
  data.clockArray.splice(data.activeInst, 1)
  data.activeInst = data.clockArray.length - 1
}


function resizeHandler() {

  winH.value = window.innerHeight;
  winW.value = window.innerWidth - 10;
  if (clock != undefined) {
    clock.resize(winW.value - leftRight.value, winH.value);
  }
  if (buttons != undefined) {
    buttons.resize(winW.value - leftRight.value);
  }

}

let allWidth = 0
for (const c of columns) {
  allWidth += c.width
}

function createClock(index) {
  layer.destroyChildren()
  clock = new Clock(layer, data.clockArray[index].data, data.clockArray[index].name, lock.value);
  clock.resize(winW.value - leftRight.value, winH.value);
  if (buttons != undefined) {
    buttons.changeClock(clock)
  }

}
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});
onMounted(() => {
  makeResizable(document.getElementById("clockRight") as HTMLElement, 'left',(w:number,x?:number)=>{
    leftRight.value=w
          resizeHandler()
          clock.resize(winW.value - leftRight.value, winH.value)
        },0,400)
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {

      if (initData.pluginData != undefined) {
        if (initData.pluginData.lock) {
          lock.value = true
        }
        
          /* there is new version */
          //Object.assign(data, initData.pluginData.data);
          data.activeInst = initData.pluginData.data.activeInst
          data.version = initData.pluginData.data.version
          data.clockArray = []
          for (const val of initData.pluginData.data.clockArray) {
            const defVal = JSON.parse(JSON.stringify(defaultClockData))
            const finVal:any = {}
            const oldVal = val.data
            const name = val.name
            for (const key of Object.keys(defVal)) {
              if(oldVal[key]){
                finVal[key] = Object.assign(defVal[key],oldVal[key])
              }else{
                finVal[key] = JSON.parse(JSON.stringify(defVal[key])) 
              }
            }
            data.clockArray.push({
              name: name,
              data: finVal
            })
          }
        
      }
      const buttonStage = new Konva.Stage({
        container: "button",
        width: winW.value,
        height: 50,
      });

      const buttonLayer = new Konva.Layer();
      buttonStage.add(buttonLayer);

      for (let i = data.clockArray.length - 1; i >= 0; i--) {
        createClock(i)
      }
      buttons = new HelpButtons(buttonLayer, clock);
      buttons.resize(winW.value - leftRight.value);

      /*always push a data when init done*/
      plugin.dataChange(toRaw(data));
      watch(data, (raw) => {
        plugin.dataChange(toRaw(raw));
      },
        {
          deep: true
        })
      watch(activeInst, (val) => {
        createClock(val)
      })
      /* Check clock at first */
      CheckIPC(data.clockArray[data.activeInst].data.periClocks)
      // show.value = true
    })
    .catch((e: any) => {
      console.error(e);
    });
    const stage = new Konva.Stage({
    container: "container",
    width: winW.value - leftRight.value,
    height: winH.value,
  });
  // add canvas element
  layer = new Konva.Layer();
  stage.add(layer);
});





</script>
<style scoped>
.clock {
  position: relative;
}

#container {
  overflow: auto;
  overflow-x: hidden;
  height: v-bind(winH - 60+'px');
  width: 100%;
}

#button {
  position: absolute;
  bottom: 10px;
  left: 0px;
  z-index: 999;
}


.clockLeft {
  width: v-bind(winW-leftRight + "px");
  height: v-bind(winH - 60 + "px");
  overflow: auto;
}

#clockRight {
  top: 0px;
  position: absolute;
  left: v-bind(winW-leftRight + "px");
  width: v-bind(leftRight + "px");
  height: v-bind(winH + "px");
  background-color: #d9ecff;
  overflow: auto;
}
</style>