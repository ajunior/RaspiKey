

class Util {
    formatUptime = (uptime) => {
        var date = new Date(uptime * 1000);
        var hh = this.zeroPad(date.getUTCHours(), 2);
        var mm = this.zeroPad(date.getUTCMinutes(), 2);
        //var ss = this.zeroPad(date.getSeconds(), 2);

        return hh + " hrs, " + mm + " min.";// + ":" + ss;
    }

    zeroPad = (num, places) => {
        var zero = places - num.toString().length + 1;
        return Array(+(zero > 0 && zero)).join("0") + num;
    }
    waitForMsec = (msec) => { 
        return new Promise(resolve => { setTimeout(() => resolve(), msec)}) 
    };
}

export default new Util();
