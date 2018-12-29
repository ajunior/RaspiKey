import React from 'react'
import AppBar from '@material-ui/core/AppBar'
import Toolbar from '@material-ui/core/Toolbar'
import Typography from '@material-ui/core/Typography'
import IconButton from '@material-ui/core/IconButton';
import MenuIcon from '@material-ui/icons/Menu';
import NavigationClose from 'material-ui/svg-icons/navigation/close';

const NavBar = (props) => {

    return(
 
        <div>

        <AppBar position="static">

            <Toolbar>
                <Typography variant="title" color="inherit">
                    RaspiKey
                </Typography>
                &nbsp;
                <Typography variant="caption" color="inherit">
                    v{props.version}
                </Typography>                                
            </Toolbar>
            

        </AppBar>

        </div>

    )
}
export default NavBar;