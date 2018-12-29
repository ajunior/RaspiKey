import React from 'react';
import Button from '@material-ui/core/Button';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';

const Globals = require('./Globals');

class AlertDialog extends React.Component {
    state = {
        open: false,
        text: null,
        caption: null,
        onClosed: null
    };

    showDialog = (text, caption, rawhtml = false, onClosed = null) => {
        this.setState({
            open: true,
            text: text,
            caption: caption,
            rawhtml: rawhtml,
            onClosed: onClosed
        });
    };

    handleClose = () => {
        this.setState({
            open: false
        });

        if(this.state.onClosed != null && Globals.isFunction(this.state.onClosed))
            this.state.onClosed();
    };

    render() {
        const { rawhtml } = this.state;

        return (
        <div>
            <Dialog
            open={this.state.open}
            onClose={this.handleClose}
            aria-labelledby="alert-dialog-title"
            aria-describedby="alert-dialog-description"
            >
            <DialogTitle id="alert-dialog-title">{this.state.caption}</DialogTitle>
            <DialogContent>
                <DialogContentText id="alert-dialog-description">

                {!rawhtml &&
                        this.state.text
                    }
                    {rawhtml &&
                        <div dangerouslySetInnerHTML={{__html: this.state.text}} />
                    }
            
                </DialogContentText>
            </DialogContent>
            <DialogActions>
                <Button onClick={this.handleClose} color="primary" autoFocus>
                OK
                </Button>
            </DialogActions>
            </Dialog>
        </div>
        );
    }
}

export default AlertDialog;