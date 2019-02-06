

<template>

  <md-card>
    <md-card-header>
      <md-card-header-text>
        <div class="md-title">{{ deviceData.name }}</div>
        <div class="md-subhead">{{ deviceData.alias }} ({{ deviceData.address }})</div>        
      </md-card-header-text>
    </md-card-header>

    <md-card-media style="padding:15px">
      <img v-if="deviceData.name==='Apple Wireless Keyboard'" src="../images/A1314.png">
      <img v-else-if="deviceData.name==='Magic Keyboard'" src="../images/A1644.png">
      <img v-else src="../images/keyboard.png">
    </md-card-media>

    <md-card-content>

      <div class="md-layout">
        <div class="md-layout-item">
          <span class="md-body-1" v-if="deviceData.connected"><md-icon style="color: #14d100">bluetooth_connected</md-icon> Paired (connected)</span>
          <span class="md-body-1" v-else-if="deviceData.paired"><md-icon>bluetooth</md-icon> Paired (disconnected)</span>
          <span class="md-body-1" v-else><md-icon>bluetooth_disabled</md-icon> Unpaired</span>
        </div>
      </div>

      <div class="md-layout">
        <div class="md-layout-item" style="align-items: center;text-align: center;">
          <div style="height: 32px">
            <span v-if="deviceData.battery != null">
            <md-progress-bar md-mode="determinate" :md-value="deviceData.battery"></md-progress-bar>    
            <span class="md-caption" style="vertical-align: text-top"><md-icon>battery_full</md-icon>{{ deviceData.battery }}%</span>      
            </span>
          </div>
        </div>

        <div class="md-layout-item" v-if="deviceData.settings != null">          
          <md-switch class="md-switch-custom" v-model="deviceData.settings.swapFnCtrl" :disabled="disabled" @change="changeSettings()">Swap Fn-Ctrl</md-switch>
          <md-switch class="md-switch-custom" v-model="deviceData.settings.swapAltCmd" :disabled="disabled" @change="changeSettings()">Swap Alt-Cmd</md-switch>
        </div>

      </div>


    </md-card-content>

    <md-card-actions>
      <md-button class="md-primary" v-on:click="pairDevice()" :disabled="disabled||deviceData.connected||deviceData.paired"><md-icon>bluetooth</md-icon> Pair</md-button>
      <md-button class="md-accent" v-on:click="removeDevice()" :disabled="disabled"><md-icon>delete</md-icon> Remove</md-button>
      <md-button class="md-icon-button" v-on:click="deviceDetails()" :disabled="disabled"><md-icon>info</md-icon><md-tooltip md-delay="300">Info</md-tooltip></md-button>

      <md-menu>
        <md-button md-menu-trigger class="md-icon-button" :disabled="disabled||!deviceData.connected">
          <md-icon v-if="!deviceData.hasKeymap">map</md-icon>
          <md-icon v-if="deviceData.hasKeymap" style="color: #14d100">map</md-icon>
          <md-tooltip md-delay="300">Custom Keymap</md-tooltip>
        </md-button>
        <md-menu-content>
          <input type="file" accept=".json" ref="file" style="display: none" @change="onUploadKeymapFileInputChange">
          <md-menu-item class="md-primary" @click="$refs.file.click()">Set Keymap...</md-menu-item>
          <md-menu-item @click="downloadKeymap" :disabled="!deviceData.hasOwnProperty('hasKeymap') || !deviceData.hasKeymap">Get Keymap...</md-menu-item>
          <md-menu-item @click="deleteKeymap" :disabled="!deviceData.hasOwnProperty('hasKeymap') || !deviceData.hasKeymap">Remove Keymap</md-menu-item>
        </md-menu-content>
      </md-menu>

    </md-card-actions>
    
  </md-card>

</template>

<script>
export default {
  name: "DeviceCard",
  props: {
    pairDeviceParent: null,
    removeDeviceParent: null,
    deviceDetailsParent: null,
    uploadKeymapParent: null,
    downloadKeymapParent: null,
    deleteKeymapParent: null,
    setSettingsParent: null,
    deviceData: {},
    disabled: false      
  },
  data: () => ({        
  }),
  mounted: function () {
      console.log("mounted()", this.deviceData);      
  },
  methods: {
    pairDevice: async function() {        
      this.pairDeviceParent(this.deviceData);
    },
    removeDevice: async function() {
      this.removeDeviceParent(this.deviceData);
    },
    deviceDetails: async function() {
      this.deviceDetailsParent(this.deviceData);
    },
    onUploadKeymapFileInputChange(e) {
      var files = e.target.files || e.dataTransfer.files;
      if (!files.length)
        return;         
      var reader = new FileReader();        
      reader.onload = (e) => {
        this.uploadKeymapParent(this.deviceData, e.target.result);
      };
      reader.readAsText(files[0]);
    },
    downloadKeymap: async function() {
      this.downloadKeymapParent(this.deviceData);
    },
    deleteKeymap: async function() {
      this.deleteKeymapParent(this.deviceData);
    },
    changeSettings: async function() {
      this.setSettingsParent(this.deviceData);
    }
  }
};
</script>

<style scoped>
.md-switch-custom {
  margin: 0px !important;  
}
</style>

<style>
.md-card {
  width: 360px;
  margin: 4px;
  display: inline-block;
  vertical-align: top;
}
.md-layout-item {
  padding: 5px;
}
.md-progress-bar {
    height: 10px !important;
}
</style>