<template>
  <div>
    <div>
      <el-form
        ref="masterRef"
        :model="data"
        label-width="200px"
        size="small"
        :rules="MasterRules"
        hide-required-asterisk
        :disabled="lock"
      >
        <el-form-item
          label="Name"
          prop="name"
        >
          <el-input v-model="data.name" />
        </el-form-item>
        <el-form-item
          label="Read-only"
          prop="readonly"
        >
          <el-checkbox v-model="data.readonly" />
        </el-form-item>
        <el-form-item
          label="Clock source"
          prop="clockSource"
          disable="!FEATURE_LPTMR_CLKSRC_SELECT_ENABLE"
        >
          <el-select
            v-model="data.clockSource"
            style="width:100%"
          >
            <el-option
              v-for="item in clockSource"
              :key="item.key"
              :value="`lpTMR_CLOCK_SOURCE_${item.label}`"
              :label="item.label"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          label="Bypass prescaler"
          prop="bypassPrescaler"
        >
          <el-checkbox v-model="data.bypassPrescaler" />
        </el-form-item>
        <el-form-item
          label="Prescaler value"
          prop="prescaler"
          v-if="!data.bypassPrescaler"
        >
          <el-select
            v-model="data.prescaler"
            style="width:100%"
          >
            <el-option
              :value="item.value"
              :label="item.label"
              v-for="item in dividerList"
              :key="item.value"
            />
          </el-select>
        </el-form-item>
        <el-form-item label="Timer clock">
          {{ timerClockKHz }}KHz
        </el-form-item>
        <el-form-item
          label="Enable DMA requests"
          prop="dmaRequest"
          v-show="DmaSupport"
        >
          <el-checkbox v-model="data.dmaRequest" />
        </el-form-item>
        <el-form-item
          label="Enable interrupt"
          prop="interruptEnable"
        >
          <el-checkbox v-model="data.interruptEnable" />
        </el-form-item>
        <el-form-item
          label="Counter work mode"
          prop="workMode"
        >
          <el-select
            v-model="data.workMode"
            style="width:100%"
          >
            <el-option
              value="lpTMR_WORKMODE_TIMER"
              label="Timer mode"
            />
            <el-option
              value="lpTMR_WORKMODE_PULSECOUNTER"
              label="Pulse counter mode"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          label="Free run mode"
          prop="freeRun"
        >
          <el-checkbox v-model="data.freeRun" />
        </el-form-item>
        <el-form-item
          label="Compare value"
          prop="compareValue"
        >
          <el-input v-model.number="data.compareValue">
            <template
              #append
              v-if="data.workMode=='lpTMR_WORKMODE_TIMER'"
            >
              <span v-if="data.counterUnits=='lpTMR_COUNTER_UNITS_TICKS'">Ticks</span>
              <span v-else>Us</span>
            </template>
          </el-input>
        </el-form-item>
        <el-form-item
          label="Counter units"
          prop="workMode"
          v-if="data.workMode=='lpTMR_WORKMODE_TIMER'"
        >
          <el-select
            v-model="data.counterUnits"
            style="width:100%"
          >
            <el-option
              value="lpTMR_COUNTER_UNITS_TICKS"
              label="UNIT_TICKS"
            />
            <el-option
              value="lpTMR_COUNTER_UNITS_MICROSECONDS"
              label="UNIT_MICROSECONDS"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          label="Pin selection for Pulse-Counter"
          prop="pinSelect"
          v-if="data.workMode=='lpTMR_WORKMODE_PULSECOUNTER'"
        >
          <el-select
            v-model="data.pinSelect"
            style="width:100%"
          >
            <el-option
              value="lpTMR_PINSELECT_TMU"
              label="PINSELECT_TMU"
            />
            <el-option
              value="lpTMR_PINSELECT_ALT1"
              label="PINSELECT ALT1"
            />
            <el-option
              value="lpTMR_PINSELECT_ALT2"
              label="PINSELECT ALT2"
            />
            <el-option
              value="lpTMR_PINSELECT_ALT3"
              label="PINSELECT ALT3"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          label="Pin Polarity for Pulse-Counter"
          prop="pinPolarity"
          v-if="data.workMode=='lpTMR_WORKMODE_PULSECOUNTER'"
        >
          <el-select
            v-model="data.pinPolarity"
            style="width:100%"
          >
            <el-option
              value="lpTMR_PINPOLARITY_RISING"
              label="PINPOLARITY_RISING"
            />
            <el-option
              value="lpTMR_PINPOLARITY_FALLING"
              label="PINPOLARITY_FALLING"
            />
          </el-select>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick, computed} from "vue";
import { Plugin } from "plugin";
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
import e from "express";
Schema.warning = function () { null };


const sxoscClock = ref(0)
const sircClock = 12000
const ipcClock = ref(0)
const FEATURE_LPTMR_CLKSRC_SELECT_ENABLE = ref(false)
const masterRef = ref<FormInstance>()
const clockSource =ref<{key:string,label:string}[]>([])
const clockFreq = ref<number[]>([])
const DmaSupport = ref(true)

const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = reactive(
  {
    name: 'LPTMR_Config',
    readonly: true,
    dmaRequest: false,
    interruptEnable: false,
    freeRun: false,
    workMode: 'lpTMR_WORKMODE_TIMER',
    prescaler: 'lpTMR_PRESCALE_2',
    bypassPrescaler: false,
    compareValue: 0,
    counterUnits: 'lpTMR_COUNTER_UNITS_TICKS',
    pinSelect: 'lpTMR_PINSELECT_TMU',
    pinPolarity: 'lpTMR_PINPOLARITY_RISING',
    clockSource: 'lpTMR_CLOCK_SOURCE_IPC'
  })

const dividerList=function(){
  const ret:any[]=[]
  for(let i=0;i<16;i++){
    ret.push({
      label:`Prescaler ${2**(i+1)}, glitch filter ${i==0?'null':2**(i)} clocks`,
      value:i==0?'lpTMR_PRESCALE_2':`lpTMR_PRESCALE_${2**(i+1)}_GLITCHFILTER_${2**(i)}`,
      divider:2**(i+1),
    })
  }
  return ret
}()
const timerClockKHz= computed(()=>{
  let divider=1
  if(data.bypassPrescaler==false){
    for(const v of dividerList){
      if(data.prescaler==v.value){
        divider=v.divider
        break
      }
    }

  }
  const indexOfClockSource=clockSource.value.findIndex((v)=>`lpTMR_CLOCK_SOURCE_${v.label}`==data.clockSource)
  return clockFreq.value[indexOfClockSource]/divider/1000
})

watch(timerClockKHz,()=>{
  masterRef.value?.validate().catch(()=>{null})
})

function resizeHandler() {
  null
}

function compareValueValid(rule: any, value: any, callback: any) {
  if (value == 0) {
    plugin.addLog('warn', 'CompareValue', 'The Compare value is zero,the hardware trigger will remain asserted until the lpTMR is disable')
  } else {
    plugin.deleteLog('warn', 'CompareValue')
  }
  if (value < 0 ) {
      callback(new Error(`Value should >=0`))
  }
  if (data.counterUnits == 'lpTMR_COUNTER_UNITS_TICKS') {
    if(value>65535){
      callback(new Error(`Value should from 0-65535`))
    }
  }else{
    let v
    if(timerClockKHz.value==0){
      v=0
    }else{
      v=Math.floor(65535*1000/timerClockKHz.value)
    }
    /* SDK will auto calculate pre and compare value in microsecond mode */
    // if(value>v||value>0xffffffff){
    //   callback(new Error(`Value should from 0-${v>0xffffffff?0xffffffff:v}`))
    // }
  }
  callback()
}




const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
  ],
  compareValue: [
    { type: 'number', required: true, message: 'Please input compare value', trigger: 'change' },
    { type: 'number', validator: compareValueValid, trigger: 'change' },
  ],
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
  masterValidator.validate(data, (errors, fields) => {
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
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      clockSource.value=plugin.getMeta('lptmr_clockSource')
      if(plugin.getMeta('dma_num') == 0){
        DmaSupport.value = false
      }
      clockFreq.value=[]
      const perClkList=plugin.getMeta('perClkList')
      data.clockSource=`lpTMR_CLOCK_SOURCE_${plugin.getMeta('lptmr_clockSource')[0].label}`
      FEATURE_LPTMR_CLKSRC_SELECT_ENABLE.value=plugin.getMeta("FEATURE_LPTMR_CLKSRC_SELECT_ENABLE")
      if (initData.pluginData != undefined) {
        Object.assign(data, initData.pluginData.data);
        lock.value = initData.pluginData.lock
      }
      nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
          null
        })
      })

      function updateClock(activeClock,clockData){
        const freqs=[0]
        for(const i of perClkList){
          freqs.push(clockData.data.clockArray[activeClock].data[i].freq.raw)
        }
        for(const [index,src] of clockSource.value.entries()){
          if(src.label=='IPC'){
            let clockIndex=-1
            for(const i in initData.globalData.clock.data.clockArray[activeClock].data.periClocks){
              if(initData.globalData.clock.data.clockArray[activeClock].data.periClocks[i].name=='lpTMR0_CLK'){
                clockIndex=Number(i)
                break  
              }
            }
            const ipc=freqs[clockData.data.clockArray[activeClock].data.periClocks[clockIndex].periSrc]/(clockData.data.clockArray[activeClock].data.periClocks[clockIndex].div+1)
            clockFreq.value[index]=ipc
          }else{
            clockFreq.value[index]=clockData.data.clockArray[activeClock].data[src.key].freq.raw
          }
        }
      }
      updateClock(initData.globalData.clock.data.activeInst,initData.globalData.clock)

      plugin.watch('clock', (newData) => {
        const activeClock=newData.data.activeInst
        updateClock(activeClock,newData)
        nextTick(()=>{
          dataCheck()
        })
      })


      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(data);
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(data);
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
  color: #409EFF
}
</style>