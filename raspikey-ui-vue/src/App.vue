<template>

  <div id="app">

    <ModalDialog ref="modalDialog1"/>

    <md-toolbar class="md-primary">
        <h1 class="md-title">RaspiKey</h1>
    </md-toolbar>
  
    <div style="height: 10px">
      <md-progress-bar md-mode="indeterminate" v-if="busy||!connected"></md-progress-bar>
    </div>

    <md-content class="md-elevation-1">
      <div class="main-control">
        <div class="md-headline"><md-icon>settings</md-icon> RaspiKey Control</div>

        <md-switch v-model="discovery" :disabled="busy" @change="changeDiscovery()"><md-icon>bluetooth_searching</md-icon> Bluetooth Discovery Mode</md-switch>

        <div class="md-title"> State</div>     
        <div class="md-caption">Save or factory reset the current RaspiKey system state. <br/>You will need to save state when you pair a new keyboard and you'd want to keep it paired them after RaspiKey restarts.</div>         
        <md-button class="md-raised md-primary" v-on:click="saveState()" :disabled="!connected"><md-icon>save</md-icon> Save</md-button>
        <md-button class="md-raised md-accent" v-on:click="resetState()" :disabled="!connected"><md-icon>delete</md-icon> Reset</md-button>

        <div class="md-title">RaspiKey Status</div>

        <div class="md-body-2">Connection:
          <span class="md-body-1" v-if="connected"><md-icon style="color: #14d100">usb</md-icon> Connected.</span>
          <span class="md-body-1" v-if="!connected"><md-icon>usb</md-icon> Connecting...</span>
        </div>

        <div class="md-body-2">Version: <span class="md-body-1">{{ version }}</span></div>
        <div class="md-body-2">Uptime: <span class="md-body-1">{{ uptime }}</span></div>
      </div>
    </md-content>

    <md-content class="md-elevation-1">
      
      <div class="md-headline"><md-icon>keyboard</md-icon> Keyboards</div>      
      <div class="md-caption">View and manage discovered Bluetooth keyboards.</div>

      <div class="device-cards" v-if="connected">
        <DeviceCard v-for="device in devices" :key="device.address" :deviceData="device" 
          :pairDeviceParent="pairDevice" 
          :removeDeviceParent="removeDevice" 
          :deviceDetailsParent="deviceDetails" 
          :uploadKeymapParent="uploadKeymap"    
          :downloadKeymapParent="downloadKeymap"
          :deleteKeymapParent="deleteKeymap"    
          :setSettingsParent="setSettings" 
          :disabled="busy"/>
      </div>

    </md-content>

    <div class="md-caption">RaspiKey, &copy;2019 George Samartzidis.</div> 
    <a href="https://github.com/samartzidis/RaspiKey" target="_blank">GitHub</a>
  </div>

  

</template>

<script>
  import DeviceCard from './components/DeviceCard.vue'
  import ModalDialog from './components/ModalDialog.vue'
  import ApiService from './api-service'
  import Util from './util'
  import FileSaver from 'file-saver';
  

  export default {
    name: 'app',
    components: {
      DeviceCard, ModalDialog
    },
    mounted: function () {

      console.log("mounted()");

      this.intervalid = setInterval(() => this.updateUi(this), 5000);

      this.updateUi();
    },
    beforeDestroy: function () {
      console.log("beforeDestroy()");

      clearInterval(this.intervalid);
    },
    data: function () {
      return {
        devices: [],
        connected: false,
        busy: false,
        discovery: false,
        uptime: null,
        version: null,
        intervalid: null        
      }
    },
    computed: {
    },
    methods: {
      saveState: async function() {
        try {      
          await ApiService.saveData();
        }
        catch (e){          
          console.error(e);
          await this.$refs.modalDialog1.showModal(e.message, "Error");
        }
      },
      resetState: async function() {
        let dialogResult = await this.$refs.modalDialog1.showModal("This will reset RaspiKey to the factory default settings. Are you sure?", "Warning", ModalDialog.ButtonStyleEnum.Ok | ModalDialog.ButtonStyleEnum.Cancel);
        if(dialogResult == ModalDialog.ButtonStyleEnum.Cancel)
          return;
        
        try {      
          await ApiService.deleteData();
        }
        catch (e){          
          console.error(e);
          await this.$refs.modalDialog1.showModal(e.message, "Error");
        }
      },
      changeDiscovery: async function() {

        try {        
          this.busy = true;
          await ApiService.setDiscovery(this.discovery);        
        }
        catch(e) {
          this.discovery = !this.discovery;
        }
        finally {
          this.busy = false;
        }

      },
      updateUi: async function(src) {
        
        if(this.busy)
          return;

        try {      
          let res = await ApiService.getInfo();
          this.uptime = Util.formatUptime(res.uptime);
          this.version = res.version;

          res = await ApiService.getDiscovery();
          this.discovery = (res.v === "on");

          res = await ApiService.getDevices();          
          this.devices = res.filter(t => t.icon === "input-keyboard");

          this.connected = true;
        }
        catch (e){          
          console.error(e);

          this.connected = false;
        }
        finally {          
        }
      },
      pairDevice: async function(deviceData) {
        
        console.log("pairDevice()", deviceData);

        try {
          this.busy = true;

          let result = await ApiService.beginPairDevice(deviceData.address); 
          
          if (result.hasOwnProperty("pinCode")) {
                let msg = `Type the following pin code on your keyboard ${result.pinCode} then press ENTER on the keyboard and finally click OK here.`;
                let dialogResult = await this.$refs.modalDialog1.showModal(msg, "Pairing...");                                                 
          }

          await ApiService.endPairDevice(deviceData.address); 
          
          await Util.waitForMsec(1000);

          await ApiService.connectDevice(deviceData.address);
          await ApiService.trustDevice(deviceData.address);
          
          await this.$refs.modalDialog1.showModal(`${deviceData.name} paired successfully!`, "Info");          
        }
        catch (e){
          console.error(e);
          await this.$refs.modalDialog1.showModal(e.message, "Error");
        }
        finally {
          this.busy = false;
        }

        await this.updateUi();
        
      },
      removeDevice: async function(deviceData) {
        
        console.log("removeDevice()", deviceData);

        await ApiService.removeDevice(deviceData.address);

        await this.updateUi();
      },
      deviceDetails: async function(deviceData) {
        
        console.log("deviceDetails()", deviceData);

        let result = await ApiService.getDeviceInfo(deviceData.address);

        var innerHtml = "";
        Object.keys(result).forEach(function (k) {
            innerHtml += "<strong>" + k + "</strong>: " + result[k] + "<br/>";
        });

        await this.$refs.modalDialog1.showModal(innerHtml, deviceData.name, ModalDialog.Ok, true);        
      },
      uploadKeymap: async function(deviceData, data) {
        try
        {
          await ApiService.setKeymap(deviceData.address, data);
        }
        catch (e){
          console.error(e);
          await this.$refs.modalDialog1.showModal(`Failed to upload keymap: ${e.message}`, "Error");
        }
      },
      downloadKeymap: async function(deviceData) {     
        try
        {
          let data = await ApiService.getKeymap(deviceData.address);

          var blob = new Blob([data], {type: "text/plain;charset=utf-8"});
          FileSaver.saveAs(blob, `${deviceData.alias} Keymap.json`); 
        }
        catch (e){
          console.error(e);
          await this.$refs.modalDialog1.showModal(`Failed to get keymap: ${e.message}`, "Error");
        }
      },
      deleteKeymap: async function(deviceData) {
        try
        {
          await ApiService.deleteKeymap(deviceData.address);
        }
        catch (e){
          console.error(e);
          await this.$refs.modalDialog1.showModal(`Failed to delete keymap: ${e.message}`, "Error");
        }
      },
      setSettings: async function(deviceData) {
        try
        {
          let data = JSON.stringify(deviceData.settings);
          await ApiService.setSettings(deviceData.address, data);
        }
        catch (e){
          console.error(e);
          await this.$refs.modalDialog1.showModal(`Failed to set device settings: ${e.message}`, "Error");
        }
      }
    }
  }

</script>

<style> 
  #app {
    font-family: 'Roboto', 'Avenir', Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;
  }
  .md-content {
    padding: 16px;
    margin: 5px;  
    text-align: left;  
  }
</style>

<style scoped>
  .device-cards {
    /* display: flex; */
    align-items: left;
    justify-content: left;
  }
  .main-control {
    display: block;
    text-align: left;
  } 
</style>
