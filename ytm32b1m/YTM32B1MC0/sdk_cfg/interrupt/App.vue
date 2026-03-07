<template>
  <div class="table">
    <el-auto-resizer>
      <template #default="{ height, width }">
        <el-table-v2
          :columns="columns"
          :data="data"
          :estimated-row-height="30"
          :width="width"
          :header-height="30"
          :height="height"
          fixed
        />
      </template>
    </el-auto-resizer>
  </div>
</template>
<script setup lang="ts">
import { onMounted, onBeforeUnmount, ref, h, watch } from "vue";
import { Plugin } from "plugin";
import type { Column } from '@element-plus/components/table-v2'


const data = ref<{
  name:string,
  num:number,
  enable:boolean,
  priority:number,
  callback:string,
  newCallback:string
}[]>([])

const columns: Column<any>[] = [
  {
    key: 'name',
    title: 'Name',
    dataKey: 'name',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {
        style: {
          'font-size': '12px'
        }
      }, rowData.name)
  },
  {
    key: 'num',
    title: 'Number',
    dataKey: 'num',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('span', {

        style: {
          'font-size': '12px',
          'color': '#409EFF',
          'font-weight': 'bolder'
        }
      }, rowData.num)
  },
  {
    key: 'enable',
    title: 'Enable',
    dataKey: 'enable',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) =>
      h('input', {
        type: 'checkbox',
        disabled:lock.value||rowData.name=='NMI_IRQn',
        checked: rowData.enable,
        'onInput': (event) => {
          rowData.enable = event.target.checked
        }
      })
  },
  {
    key: 'priority',
    title: 'Priority',
    dataKey: 'priority',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData,rowIndex}) =>
      h('input', {
        type: 'number',
        value: rowData.priority,
        min: (rowIndex==0||rowIndex==1)?-3:0,
        max: 7,
        disabled:rowIndex==0||rowIndex==1||lock.value,
        'onInput': (event) => rowData.priority = Number(event.target.value)
      })
  },
  {
    key: 'callback',
    title: 'SDK Interrupt Handle',
    dataKey: 'callback',
    width: 300,
    align: 'center',
    cellRenderer: ({ rowData, rowIndex }) =>
      h('div', [
        h('input', {
          type: "text",
          disabled: true,
          style: {
            'width': '300px',
            'border': rowData.callback == '' ? '1px solid #F56C6C' : '',
          },
          value: rowData.callback,
          'onInput': (event) => {
            rowData.callback = event.target.value
            if (rowData.callback == '') {
              plugin.addLog('error', `Row${rowIndex}`, "Interrupt handle can't be null")
            } else {
              plugin.deleteLog('error', `Row${rowIndex}`)
            }
          }
        }),
        rowData.callback == '' ?
          h('span', {
            style: {
              'font-size': '12px',
              'color': '#F56C6C',
              'position': 'absolute',
              'top': '6px',
              'left': '505px'
            }
          }, 'Please input interrupt handle') : h('span')
      ])
  },
  {
    key: 'newCallback',
    title: 'New Interrupt Handle',
    dataKey: 'newCallback',
    width: 300,
    align: 'center',
    cellRenderer: ({ rowData, rowIndex }) =>
      h('div', [
        h('input', {
          type: "text",
          disabled:lock.value,
          value: rowData.newCallback,
          'onInput': (event) => {
            rowData.newCallback = event.target.value
          }
        })
      ])
  }
]


const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const height = ref(window.innerHeight)
const width = ref(window.innerWidth)

function resizeHandler() {
  height.value = window.innerHeight
  width.value = window.innerWidth
}

function dataCheck() {
  for (const i in data.value) {
    if (data.value[i].callback == ''||!/^[-_a-zA-Z0-9]+$/gi.test(data.value[i].callback)) {
      plugin.addLog('error', `Row${i}`, "Interrupt handle can't be null")
    }
  }
}

onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});


onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      data.value = plugin.getMeta('interrupt')
      if (initData.pluginData != undefined) {
       
        for(const i in data.value){
          const target=initData.pluginData.data.find((item)=>item.num==data.value[i].num)
          if(target!=undefined){
            data.value[i].enable=target.enable
            data.value[i].priority=target.priority
            data.value[i].callback=target.callback
            data.value[i].newCallback=target.newCallback
          }
        }
        lock.value = initData.pluginData.lock
      }
      /*NMI*/
      data.value[0].priority=-2
      /*Hardfault*/
      data.value[1].priority=-1
      dataCheck()
      /*always push a data when init done*/
      plugin.dataChange(data.value);
      watch(data, (raw) => {
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
<style>
.el-table .active-row {
  color: #409EFF,
}

.table {

  margin: 10px;
  width: v-bind(width -20 + "px");
  height: v-bind(height-20 + "px");
  overflow: auto;
  text-align: center;
}
</style>