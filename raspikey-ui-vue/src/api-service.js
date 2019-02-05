
const BASE_URL = process.env.VUE_APP_BASE_URL;

class ApiService {

    async getInfo() {
        var url = `${BASE_URL}/info`;   
        let data = await this.sendRequest("GET", url);
        return JSON.parse(data);   
    }

    async getDiscovery() {
        var url = `${BASE_URL}/bt/discovery`;    
        let data =  await this.sendRequest("GET", url);
        return JSON.parse(data);  
    }

    async setDiscovery(value) {
        var url = `${BASE_URL}/bt/discovery?v=${(value === true) ? "on" : "off"}`;
        await this.sendRequest("GET", url);
    }

    async getDevices() {
        var url = `${BASE_URL}/bt/devices`;      
        let data = await this.sendRequest("GET", url);
        return JSON.parse(data); 
    }    

    async beginPairDevice(address) {
        var url = `${BASE_URL}/bt/begin-pair?v=${address}`;  
        let data = await this.sendRequest("GET", url);
        return JSON.parse(data); 
    }

    async endPairDevice(address) {
        var url = `${BASE_URL}/bt/end-pair?v=${address}`; 
        await this.sendRequest("GET", url);
    }

    async connectDevice(address) {
        var url = `${BASE_URL}/bt/connect?v=${address}`;
        await this.sendRequest("GET", url);
    }

    async trustDevice(address) {
        var url = `${BASE_URL}/bt/trust?v=${address}`;
        await this.sendRequest("GET", url);
    }

    async removeDevice(address) {
        var url = `${BASE_URL}/bt/remove?v=${address}`;      
        await this.sendRequest("GET", url);
    }

    async getDeviceInfo(address) {
        var url = `${BASE_URL}/bt/info?v=${address}`;   
        let data = await this.sendRequest("GET", url);
        return JSON.parse(data); 
    }

    async saveData() {
        var url = `${BASE_URL}/save-data`;      
        await this.sendRequest("GET", url);
    }

    async deleteData() {
        var url = `${BASE_URL}/delete-data`;    
        await this.sendRequest("GET", url);
    }

    async setKeymap(address, data) {
        var url = `${BASE_URL}/keymap?v=${address}`;  
        await this.sendRequest("POST", url, data);
    }

    async deleteKeymap(address) {
        var url = `${BASE_URL}/keymap?v=${address}`;  
        await this.sendRequest("DELETE", url);
    }

    async getKeymap(address) {
        var url = `${BASE_URL}/keymap?v=${address}`;  
        return await this.sendRequest("GET", url);
    }

    async setSettings(address, data) {
        var url = `${BASE_URL}/settings?v=${address}`;  
        await this.sendRequest("POST", url, data);
    }

    async sendRequest(method, url, body) {

        console.debug(method, url);

        let res = null;
        try {
            res = await fetch(url, { method: method, body: body });
        }
        catch(e) {
            throw { status: 0, statusText: null, message: e };
        }

        let data = await res.text();
        if(res.ok)
            return data;

        throw { status: res.status, statusText: res.statusText, message: data };        
    }
    
}

export default new ApiService();
