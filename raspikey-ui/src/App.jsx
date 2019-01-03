import React, { Component } from 'react';
import PropTypes from 'prop-types';
import Switch from '@material-ui/core/Switch';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import CircularProgress from '@material-ui/core/CircularProgress';
import 'typeface-roboto'
import NavBar from './NavBar'
import DeviceList from './DeviceList'
import AlertDialog from './AlertDialog'
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';
import Tooltip from '@material-ui/core/Tooltip';
import { withStyles } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import TextField from '@material-ui/core/TextField';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import DeleteIcon from '@material-ui/icons/Delete';
import SaveIcon from '@material-ui/icons/Save';
import SendIcon from '@material-ui/icons/Send';
import AccountBalanceIcon from '@material-ui/icons/AccountBalance';
import LinearProgress from '@material-ui/core/LinearProgress';
import BlockUi from 'react-block-ui';
import 'react-block-ui/style.css';
import 'typeface-roboto'
import BluetoothIcon from '@material-ui/icons/Bluetooth';
import MemoryIcon from '@material-ui/icons/Memory';

const Globals = require('./Globals');

// Styles
const styles = {
    container: {
        textAlign: 'left',
        padding: 10
    },
    textField: {
        marginLeft: 10,
        marginRight: 10,
        width: 200,
    },
    menu: {
        width: 200,
    },
    button: {
    },
}

class App extends Component {

  constructor (props, context) {
      super(props, context);

      this.state = {
          discovery: false,
          version: null,
          uptime: null,
          gotInfo: false,
          blocking: false
      }
  }

    componentDidMount() {
        this.getInfo();
        this.getDiscovery();

        var src = this;
        setInterval(() => this.updateUi(src), 5000);

        window.addEventListener("beforeunload", this.handleBeforeunload);
    }

    componentWillUnmount() {
        clearInterval(this.state.timer);

        window.removeEventListener("beforeunload", this.handleBeforeunload);
    }

    handleBeforeunload = event => {
    }

    updateUi = (src) => {
        if (this.state.resetting || this.state.blocking || this.refs.deviceList.state.blocking)
            return;

        this.getInfo();
        this.getDiscovery();

        if (this.refs.deviceList != null)
            this.refs.deviceList.updateDevices(); // Update device list
    }

    getInfo() {
        Globals.raspiKeyApiInvoke('info', (result) => {
            this.setState({ version: result.version, uptime: result.uptime, gotInfo: true });
        }, (error, body) => {
            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
            this.setState({ gotInfo: false });
        });
    }

    getDiscovery() {
        Globals.raspiKeyApiInvoke('bt/discovery', (result) => {
            this.setState({ discovery: result.v === "on" });
        }, (error, body) => {
            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
        });
    }

    setDiscovery(value) {
        this.setState({ blocking: true });

        var path = 'bt/discovery?v=' + (value === true ? "on" : "off");

        Globals.raspiKeyApiInvoke(path, (result) => {
            this.setState({ discovery: value, blocking: false });
        }, (error, body) => {
            this.setState({ blocking: false });

            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
        });
    }

    saveData = (event) => {
        this.setState({ blocking: true });

        Globals.raspiKeyApiInvoke('save-data', (result) => {
            this.setState({ blocking: false });

        }, (error, body) => {
            this.setState({ blocking: false });

            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
            this.refs.alertDialog.showDialog(errorMsg, "Error");
        });
    }

    deleteData = (event) => {
        this.setState({ blocking: true });

        Globals.raspiKeyApiInvoke('delete-data', (result) => {

            setTimeout(() => {
                this.setState({ blocking: false });
            }, 30000);

        }, (error, body) => {
            this.setState({ blocking: false });

            var errorMsg = "Error Message: " + error.message + ", Status: " + error.status + " (" + error.statusText + "), Body: " + body;
            console.error(errorMsg);
            this.refs.alertDialog.showDialog(errorMsg, "Error");
        });
    }

    onChangeDiscovery = (event) => {
        this.setDiscovery(event.target.checked);
    };

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

    render() {

        return (
            <div style={styles.container}>
                <AlertDialog ref="alertDialog" />
                <NavBar version={this.state.version} />
                { !this.state.gotInfo && <LinearProgress/> }
                <br />

                <BlockUi tag="div" blocking={this.state.blocking}>
                <div>
                    <Paper style={styles.container}>
                        
                        <Typography variant="headline" component="h3">
                            <MemoryIcon/>Device Control
                        </Typography>
                        <Typography component="p">
                            Bluetooth Module:
                        </Typography>
                        <Tooltip title="Turn on to discover any available bluetooth devices.">
                            <FormControlLabel
                                label="Device Discovery"
                                control={
                                    <Switch
                                        disabled={this.state.version == null}
                                        checked={this.state.discovery}
                                        onChange={this.onChangeDiscovery}
                                        value="discovery" />
                                } />                                    
                        </Tooltip>                                
                        <Typography component="p">
                            RaspiKey State:
                        </Typography>

                        <Tooltip title="Save the current RaspiKey data. For example if you want to keep the currently paired and discovered bluetooth keyboards after RaspiKey is turned off and turned on again.">
                            <Button onClick={this.saveData} disabled={!this.state.gotInfo} color="primary" aria-label="Delete">
                                <SaveIcon /> Save
                            </Button>
                        </Tooltip>
                        <Tooltip title="This will reset the RaspiKey device to the factory state. If you do this, all licensing information will be lost as well and will need to be re-entered.">
                            <Button onClick={this.deleteData} disabled={!this.state.gotInfo} color="secondary" aria-label="Reset">
                                <DeleteIcon /> Reset
                            </Button>
                        </Tooltip>

                        <Typography component="p">
                            Device Details:
                        </Typography>

                        {this.state.gotInfo ? (
                            <div>
                                <Typography variant="caption">
                                    Uptime: {this.formatUptime(this.state.uptime)}
                                </Typography>
                            </div>
                        ) : (
                            <div>
                                <Typography variant="caption" color="error">
                                    Unable to communicate with the RaspiKey device. Please ensure that RaspiKey is plugged in to your computer and that the green status LED is on.
                                </Typography>
                            </div>
                        )}
                        
                    </Paper>

                    <br />

                    <Paper style={styles.container}>
                        <Typography variant="headline" component="h3">
                            <BluetoothIcon/>Bluetooth Devices
                        </Typography>
                        <Typography component="p">
                            List of discovered and configured bluetooth devices:
                        </Typography>
                        <DeviceList ref="deviceList" parent={this} />
                    </Paper>

                </div>
                </BlockUi>

                <br />

                <Typography align="left">
                    <i>RaspiKey v{this.state.version}. Copyright &copy;2019 George Samartzidis, All Rights Reserved.</i>
                </Typography>
            </div>
        );
    }
}

export default App;
