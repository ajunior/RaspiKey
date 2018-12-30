![raspikey-logo](images/raspikey-logo.png)
###### Apple and Universal Bluetooth to USB Keyboard Adapter

RaspiKey is an adaptor (bridge) that converts a bluetooth keyboard to a wired USB keyboard. RaspiKey runs on the [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/) board.

RaspiKey is particularly useful with the **Apple Wireless** keyboards family because it can translate their exotic keymap to that of a **standard PC keyboard** and make them fully usable on a PC or other devices (TVs, game consoles, etc.).

![raspikey-diagram](images/raspikey-diagram.png)

Features
--------

* RaspiKey can host **any bluetooth keyboard** but it currently offers full keyboard compatibility (e.g. special multimedia keys support) only for the Apple Wireless (A1314) and the Apple Magic (A1644) keyboard models.
* **Zero PC software footprint**. It does not require any driver installation or specific software installation on your PC that uses a connected keyboard.
* **Self-consistent**. It does not require external power. It is self-powered through the PC USB port.
* It is **inherently secure**. As it is a hardware device, there is no software driver running on your PC that can be hacked, intercepted or compromised compared to a software solution. You even have **full access to the source code** to examine what it does or even build it yourself.
* RaspiKey **can be used on anything**; on any device that accepts a standard USB keyboard, not just on Windows PCs. This means that you can connect your Apple keyboard to your smart TV or your game console with full keyboard functionality.
* It is **robust and responsive**, based on a cut-down, readonly version of the official Raspbian Linux. Even though it runs an OS internally, there is no need for a shutdown process, you just unplug it when you want.

Screenshots
-----------

<a href="images/shot1.png"><img width="100" height="100" src="images/shot1_tn.png"></a>
<a href="images/shot2.png"><img width="100" height="100" src="images/shot2_tn.png"></a>
<a href="images/shot3.png"><img width="100" height="100" src="images/shot3_tn.png"></a>
<a href="images/shot4.jpg"><img width="100" height="100" src="images/shot4_tn.jpg"></a>


Hardware and Software Requirements
----------------------------------

1. A plain [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/) board.
2. A 2GB micro SD card.
3. Access to a modern Windows PC with the Chrome browser. You need this to build your new RaspiKey device and tether a Bluetooth keyboard to it for the first time.

Building your new RaspiKey device
---------------------------------

1. Provided you have your new RaspiKey board mentioned in the previous step.
2. Download [Apple Bonjour](https://developer.apple.com/bonjour/) and install it on Windows. The latest 32-bit and 64-bit versions are available [here](https://github.com/samartzidis/RaspiKey/tree/master/utilities).
3. Download and install [Etcher](https://etcher.io/).
4. Download the latest RaspiKey release **raspikey.zip** and the latest Raspbian headless release **...-raspbian-stretch-lite-headless.xz** from [here](https://github.com/samartzidis/RaspiKey/releases).
5. Run Etcher to write the **...-raspbian-stretch-lite-headless.xz** image to the SD card.
6. Open the downloaded **raspikey.zip** file and copy all of its contents (setup, start.sh) directly to the the SD card. The SD card should appear on Windows as a drive named **boot**.
7. Now insert the SD card to the “Raspberry Pi Zero W” device and connect it to the PC using just the middle micro-USB port (labelled “USB” with tiny white letters). The Raspberry Pi will both draw power from this port as well as use it to communicate with the PC.
8. The Pi will boot and continue the setup. This process should last about 3-4 minutes. You will be able to tell when it's finished after the Raspberry Pi's green led permanently stops flashing. Once that happens, unplug it from the PC's USB port.
9. Your new RaspiKey device is now fully configured and ready to use.

RaspiKey Usage
--------------
1. Plug RaspiKey to a PC.
2. Wait for about 10 seconds for it to stat and open Chrome at **[http://raspikey.local](http://raspikey.local)**. 
3. You should now be able to use the configuration page to discover and tether your Bluetooth keyboard to it. Once the keyboard is discovered and tethered, it will be usable on this PC directly or on anywhere else you plug RaspiKey to. You do not need to reconfigure RaspiKey to plug it in and use it elsewhere.


Known Issues
------------

*   The battery level indicator does not yet work for the Apple Magic (A1644) keyboard.
*   The Ctrl-LShift-T combination does not work on Apple keyboards, use the Ctrl-RShift-T instead where applicable.

Apple Keyboards Keymap
----------------------

<table>
    <tr>
      <th>Input Key(s)</th>
      <th>Output Key</th>
    </tr>
    <tr>
      <td><kbd>LCtrl</kbd></td><td><kbd>Fn</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd></td><td><kbd>LCtrl</kbd></td>
    </tr>
    <tr>
      <td><kbd>⏏︎ Eject</kbd></td><td><kbd>Del</kbd></td>
    </tr>
    <tr>
      <td><kbd>⌘ Cmd</kbd></td><td><kbd>Alt</kbd></td>
    </tr>    
    <tr>
      <td><kbd>⌥ Alt</kbd></td><td><kbd>Cmd</kbd></td>
    </tr>    
    <tr>
      <td><kbd>Fn</kbd>+<kbd>[F1]</kbd>...<kbd>[F6]</kbd></td><td><kbd>[F13]</kbd>...<kbd>[F18]</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>LCtrl</kbd></td><td><kbd>RCtrl</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>Return</kbd></td><td><kbd>Insert</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>P</kbd></td><td><kbd>Prt Scr</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>S</kbd></td><td><kbd>Scr Lck</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>B</kbd></td><td><kbd>Pause/Break</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>&uarr;</kbd></td><td><kbd>Pg Up</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>&darr;</kbd></td><td><kbd>Pg Down</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>&larr;</kbd></td><td><kbd>Home</kbd></td>
    </tr>
    <tr>
      <td><kbd>Fn</kbd>+<kbd>&rarr;</kbd></td><td><kbd>End</kbd></td>
    </tr>
  </table>

Planned Futures
---------------

*   Support for the latest full-size Apple Magic keyboard.
*   Support for user customizable keymap and programmability.
*   Support for multimedia or special key functions for more Bluetooth keyboards.
*   Support for multiple Bluetooth keyboards connected in parallel.

---
RaspiKey is a free and open-source software effort. If you like it and find it useful, please consider donating

[![donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=TBM5P9X6GZRCL)

---

<a href="https://github.com/samartzidis/RaspiKey"><img src="images/github-small.png"/></a>