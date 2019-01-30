

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
        <div class="md-layout-item">          
        </div>
      </div>
    </md-card-content>

    <md-card-actions>
      <md-button class="md-primary" v-on:click="pairDevice()" :disabled="disabled||deviceData.connected||deviceData.paired"><md-icon>bluetooth</md-icon> Pair</md-button>
      <md-button class="md-accent" v-on:click="removeDevice()" :disabled="disabled"><md-icon>delete</md-icon> Remove</md-button>
      <md-button class="md-icon-button" v-on:click="deviceDetails()" :disabled="disabled"><md-icon>info</md-icon><md-tooltip md-delay="300">Info</md-tooltip></md-button>
      <md-button class="md-icon-button" :disabled="disabled"><md-icon>map</md-icon><md-tooltip md-delay="300">Custom Keymap</md-tooltip></md-button>
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
      }
  }
};
</script>

<style>
.md-card {
  width: 320px;
  margin: 4px;
  display: inline-block;
  vertical-align: top;
}
.md-progress-bar {
    height: 10px !important;
}
</style>