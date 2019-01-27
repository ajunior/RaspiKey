<template>
  <div>
    <md-dialog :md-active.sync="showDialog">
      <md-dialog-title>{{caption}}</md-dialog-title>
      
        <md-dialog-content>
            <div v-if="rawHtmlMode" v-html="text"/>
            <div v-if="!rawHtmlMode">{{text}}</div>
        </md-dialog-content>

      <md-dialog-actions>
        <md-button class="md-primary" @click="onClick($options.ButtonStyleEnum.Ok)" v-if="buttonsStyle & $options.ButtonStyleEnum.Ok">OK</md-button>
        <md-button class="md-primary" @click="onClick($options.ButtonStyleEnum.Cancel)" v-if="buttonsStyle & $options.ButtonStyleEnum.Cancel">Cancel</md-button>
      </md-dialog-actions>
    </md-dialog>
  </div>
</template>

<script>
export default {
    name: 'ModalDialog',
    ButtonStyleEnum: Object.freeze({ "Ok": 1, "Cancel": 2 }),
    props: {
        
    },
    data: () => ({
        showDialog: false,
        text: null,
        caption: null,
        buttonsStyle: null,
        rawHtmlMode: false,
        promiseResolve: null
    }), 
    methods: {        
        onClick: async function(result) {
            this.dialogResult = result;
            this.showDialog = false;

            this.promiseResolve(result);
        },      
        showModal: async function(text, caption, buttonsStyle, rawHtmlMode) {    
            this.text = text;
            this.caption = caption;
            if(buttonsStyle == null)
                this.buttonsStyle = this.$options.ButtonStyleEnum.Ok;
            else
                this.buttonsStyle = buttonsStyle;
            this.rawHtmlMode = rawHtmlMode;
            this.showDialog = true;
            
            var promise = new Promise(resolve => { this.promiseResolve = resolve; });
            var dialogResult = await promise;

            return dialogResult;
      }
    }
};
</script>

<style scoped>
.md-dialog {
    max-width: 768px;
}
</style>