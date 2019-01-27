
const BASE_URL = process.env.VUE_APP_BASE_URL;

class ApiService {

    async getInfo() {
        var url = `${BASE_URL}/info`;      
        return await this.sendRequest("GET", url);
    }

    async getDiscovery() {
        var url = `${BASE_URL}/bt/discovery`;      
        return await this.sendRequest("GET", url);
    }

    async setDiscovery(value) {
        var url = `${BASE_URL}/bt/discovery?v=${(value === true) ? "on" : "off"}`;
        return await this.sendRequest("GET", url);
    }

    async getDevices() {
        var url = `${BASE_URL}/bt/devices`;      
        return await this.sendRequest("GET", url);
    }

    async removeDevice(address) {
        var url = `${BASE_URL}/bt/remove?v=${address}`;      
        return await this.sendRequest("GET", url);
    }

    async beginPairDevice(address) {
        var url = `${BASE_URL}/bt/begin-pair?v=${address}`;      
        return await this.sendRequest("GET", url);
    }

    async endPairDevice(address) {
        var url = `${BASE_URL}/bt/end-pair?v=${address}`;      
        return await this.sendRequest("GET", url);
    }

    async connectDevice(address) {
        var url = `${BASE_URL}/bt/connect?v=${address}`;      
        return await this.sendRequest("GET", url);
    }

    async trustDevice(address) {
        var url = `${BASE_URL}/bt/trust?v=${address}`;      
        return await this.sendRequest("GET", url);
    }

    async getDeviceInfo(address) {
        var url = `${BASE_URL}/bt/info?v=${address}`;      
        return await this.sendRequest("GET", url);
    }

    async saveData() {
        var url = `${BASE_URL}/save-data`;      
        return await this.sendRequest("GET", url);
    }

    async deleteData() {
        var url = `${BASE_URL}/delete-data`;      
        return await this.sendRequest("GET", url);
    }

    async sendRequest(method, url, body) {

        console.debug(method, url);

        let res = null;
        try {
            res = await fetch(url, {method: method, body: body});
        }
        catch(e) {
            throw { status: 0, statusText: null, message: e };
        }

        if(res.ok) 
            return await res.json();
        
        let msg = await res.text();
        throw { status: res.status, statusText: res.statusText, message: msg };        
    }
}

export default new ApiService();
