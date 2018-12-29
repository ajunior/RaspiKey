import React, {Component} from 'react'
import Grid from '@material-ui/core/Grid'
import TextField from '@material-ui/core/TextField'
import * as contentful from 'contentful'
import Device from './Device'
import Button from '@material-ui/core/Button';
import Dialog from '@material-ui/core/Dialog';
import AlertDialog from './AlertDialog'
import BlockUi from 'react-block-ui';
import 'react-block-ui/style.css';

const Globals = require('./Globals');

class DeviceList extends Component {

    state = {
        app: null,
        devices: [ ], 
        blocking: false,    
        pairing: false
    }

    constructor() {
        super();        
    }

    componentDidMount() {         
        this.updateDevices();          
    }    

    componentWillUnmount() {
    }

    updateDevices = () => {
        if(this.state.blocking)
            return;

        Globals.raspiKeyApiInvoke('bt/devices', (result) => {
            this.setState({ devices: result });
        }, (error, body) => {
            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
            });
    }

    removeDevice = (event, address) => {        
        this.setState({blocking: true});

        var route = 'bt/remove?v=' + address;
        Globals.raspiKeyApiInvoke(route, (result) => {
            var filtered = this.state.devices.filter(item => item.address !== address);

            this.setState({ blocking: false, devices: filtered });
            this.updateDevices();

        }, (error, body) => {

            this.setState({ blocking: false });
            this.updateDevices();

            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
            this.setState({ blocking: false });
        });
    }

    beginPairDevice = (event, address) => {   

        // We currently don't allow multi-device pairing
        for (var i = 0; i < this.state.devices.length; i++) {
            var dev = this.state.devices[i];
            if (dev.paired === true) {
                this.refs.alertDialog.showDialog("Sorry, pairing with more than one devices is not supported in the current version.", "Error");
                return;
            }
        }

        this.setState({blocking: true});

        var route = 'bt/begin-pair?v=' + address;
        Globals.raspiKeyApiInvoke(route, (result) => {
            if (result.hasOwnProperty("pinCode")) {
                var msg = "Type the following pin code on your keyboard " + result.pinCode + " then press ENTER on the keyboard and finally click OK here.";
                this.refs.alertDialog.showDialog(msg, "Pairing...", false, () => {
                    this.endPairDevice(address);
                });
            }
            else
                this.endPairDevice(address);
        }, (error, body) => {

            this.setState({ blocking: false });
            this.updateDevices();

            this.refs.alertDialog.showDialog(body, "Error");
        });
    }

    endPairDevice = (address) => {
        this.setState({ blocking: true });

        var route = 'bt/end-pair?v=' + address;
        Globals.raspiKeyApiInvoke(route, (result) => {
            setTimeout(() => {
                this.connectDevice(address);
            }, 1000);
        }, (error, body) => {

            this.setState({ blocking: false });
            this.updateDevices();

            this.refs.alertDialog.showDialog(body, "Error");
        });
    }

    connectDevice = (address) => { 
        this.setState({ blocking: true });

        var route = 'bt/connect?v=' + address;
        Globals.raspiKeyApiInvoke(route, (result) => {
            this.trustDevice(address);
        }, (error, body) => {

            this.setState({ blocking: false });
            this.updateDevices();

            this.refs.alertDialog.showDialog(body, "Error");
        });
    }

    trustDevice = (address) => { 
        this.setState({ blocking: true });

        var route = 'bt/trust?v=' + address;
        Globals.raspiKeyApiInvoke(route, (result) => {

            this.setState({ blocking: false });
            this.updateDevices();

        }, (error, body) => {

            this.setState({ blocking: false });
            this.updateDevices();

            this.refs.alertDialog.showDialog(body, "Error");
        });
    }


    deviceDetails = (event, address) => {
        this.setState({blocking: true});

        var route = 'bt/info?v=' + address;
        Globals.raspiKeyApiInvoke(route, (result) => {
            this.setState({ blocking: false });

            var innerHtml = "";
            Object.keys(result).forEach(function (k) {
                innerHtml += "<strong>" + k + "</strong>: " + result[k] + "<br/>";
            });
            this.refs.alertDialog.showDialog(innerHtml, result.name, true);
        }, (error, body) => {
            this.setState({ blocking: false });

            this.refs.alertDialog.showDialog(body, "Error");
        });
    }

    render() {

        return (
            <div>
                
                <AlertDialog ref="alertDialog"/>    

                <BlockUi tag="div" blocking={this.state.blocking}>

                {this.state.devices ? (
                    <div>
                        <Grid container spacing={24} style={{padding: 24}}>
                                {this.state.devices
                                    .filter(currentDevice => Globals.isSupportedDevice(currentDevice))
                                    .map(currentDevice => (
                                    <Grid item xs={12} sm={6} lg={4} xl={3}>
                                        <Device device={currentDevice} parent={this} />
                                    </Grid>
                                ))}
                        </Grid>

                    </div>
                ) : "No devices found" }

                </BlockUi>
 
            </div>
        );
    }
}
export default DeviceList;