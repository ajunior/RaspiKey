import React from 'react'
import Card from '@material-ui/core/Card'
import CardActions from '@material-ui/core/CardActions'
import CardContent from '@material-ui/core/CardContent'
import CardMedia from '@material-ui/core/CardMedia'
import Button from '@material-ui/core/Button'
import Typography from '@material-ui/core/Typography'
import Tooltip from '@material-ui/core/Tooltip';
import LinearProgress from '@material-ui/core/LinearProgress';
import DeleteIcon from '@material-ui/icons/Delete';
import InfoIcon from '@material-ui/icons/Info';
import BluetoothConnectedIcon from '@material-ui/icons/BluetoothConnected';
import BluetoothIcon from '@material-ui/icons/Bluetooth';
import BluetoothDisabledIcon from '@material-ui/icons/BluetoothDisabled';
import BatteryFullIcon from '@material-ui/icons/BatteryFull';
import Grid from '@material-ui/core/Grid';

const Globals = require('./Globals');


const Device = (props) => {
    
    

    return(        
        <div>
            {props.device ? (

                <Card>
                    
                    {
                        (() => {
                            //var imgPath = Globals.getDeviceImagePath(props.device);
                            if (props.device.name === "Apple Wireless Keyboard")
                                return <CardMedia style={{height: 0, paddingTop: '56.25%'}} image={require('./images/A1314.png')} title={props.device.name}/>
                            else if (props.device.name === "Magic Keyboard")
                                return <CardMedia style={{height: 0, paddingTop: '56.25%'}} image={require('./images/A1644.png')} title={props.device.name}/>
                            else if (props.device.icon === "input-keyboard")
                                return <CardMedia style={{ height: 0, paddingTop: '56.25%' }} image={require('./images/keyboard.png')} title={props.device.name} />
                            else
                                return <CardMedia style={{ height: 0, paddingTop: '56.25%' }} title={props.device.name} />
                        })()
                    }

                    <CardContent>
                        <Typography gutterBottom variant="headline" component="h2">
                            {props.device.name}
                        </Typography>

                        <Typography component="p">
                            {props.device.address}
                        </Typography>

                        <Grid container spacing={24}>
                            <Grid item xs>
                                {
                                    (() => {
                                        if (props.device.paired) {
                                            if (props.device.connected)
                                                return <Tooltip title={props.device.name + " is paired and connected"}><BluetoothConnectedIcon style={{ color: '#5DFC0A' }} /></Tooltip>
                                            else
                                                return <Tooltip title={props.device.name + " is paired but not connected"}><BluetoothIcon style={{ color: '#0066FF' }} /></Tooltip>
                                        }
                                        else
                                            return <Tooltip title={props.device.name + " is unpaired"}><BluetoothDisabledIcon style={{ color: '#000000' }} /></Tooltip>
                                    })()
                                }
                            </Grid>
                            <Grid item xs="6">                       
                                {(props.device.battery != null) &&
                                    <div>
                                    <LinearProgress variant="determinate" style={{ height: 10 }} value={props.device.battery} />
                                        <Typography component="p" align="center">
                                            {props.device.battery}% charged
                                        </Typography>
                                    </div>
                                }
                            </Grid>
                        </Grid>

                    </CardContent>

                    <CardActions>

                        { Globals.isSupportedDevice(props.device) && !props.device.paired ? (
                        <Tooltip title={"Pair and Connect " + props.device.name}>
                            <Button size="small" color="primary" onClick={(e) => props.parent.beginPairDevice(e, props.device.address)}>                            
                                <BluetoothConnectedIcon/>                            
                            </Button> 
                        </Tooltip>
                        ) : null}

                        <Tooltip title={"Remove " + props.device.name}>
                            <Button size="small" color="primary" onClick={(e) => props.parent.removeDevice(e, props.device.address)}>
                                <DeleteIcon/>
                            </Button> 
                        </Tooltip>

                        <Tooltip title={props.device.name + " Info"}>
                            <Button size="small" color="primary" onClick={(e) => props.parent.deviceDetails(e, props.device.address)}>
                                <InfoIcon/>
                            </Button> 
                        </Tooltip>

                    </CardActions>

                </Card>

          ) : null }  
        </div>
    )
}
export default Device