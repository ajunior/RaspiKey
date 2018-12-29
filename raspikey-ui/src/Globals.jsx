

export const RASPIKEY_BASE_URI = 'http://raspikey.local/';
//export const RASPIKEY_BASE_URI = 'http://zea.local/';

export function raspiKeyApiInvoke(path, onSuccess, onError) {
    return this.raspiKeyApiInvokeInternal("GET", path, null, onSuccess, onError);
}

export function raspiKeyApiInvokePost(path, body, onSuccess, onError) {
    return this.raspiKeyApiInvokeInternal("POST", path, body, onSuccess, onError);
}

export function raspiKeyApiInvokeInternal(method, path, body, onSuccess, onError) {
    var uri = this.RASPIKEY_BASE_URI + path;
    fetch(uri, {method: method, body: body})
        .then(result => { 
            if(result.ok) 
                return result.json(); 
            else
                throw result; 
        })
        .then(result => { 
            onSuccess(result);
        })
        .catch((error) => {        
            if(onError == null || !isFunction(onError))
                return;

            if (isFunction(error.text)) {
                error.text().then(body => {
                    onError(error, body);
                });
            } else
                onError(error, null);
        })
}

export function isFunction(functionToCheck) {
    return functionToCheck && {}.toString.call(functionToCheck) === '[object Function]';
}

export function isSupportedDevice(device) {
    return device.icon === "input-keyboard";
}

export function isValidJson(json) {
    try {
        JSON.parse(json);
        return true;
    } catch (e) {
        return false;
    }
}
