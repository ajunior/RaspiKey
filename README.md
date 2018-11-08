![raspikey-logo](images/raspikey-logo.png)
###### Universal Bluetooth to USB HID Adapter


RaspiKey allows you to convert a bluetooth keyboard device to a universal wired USB (HID) keyboard device. RaspiKey is software that runs on the cheap [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/) board.

It is particularly useful for the Apple Wireless keyboards because it can translate their exotic keymap to that of a standard USB PC keyboard. This an easy way to get an Apple keyboard to seamlessly and fully work on a Windows PC (or any other device that is not an Apple computer) without the need to install any software or special drivers on it.

![raspikey-diagram.png](images/raspikey-diagram.png)

Features
--------

*   RaspiKey can actually host **any bluetooth keyboard** but it currently offers full keyboard compatibility (e.g. special multimedia keys support) only for the Apple Wireless (A1314) and the Apple Magic (A1644) keyboard models.
*   **Zero PC software footprint**. It does not require any driver installation or specific software installation on your PC that uses a connected keyboard.
*   **Self-consistent**. It does not require external power. It is self-powered through the PC USB port.
*   It is **inherently secure**. As it is a hardware device, there is no software driver running on your PC that can be hacked or compromised compared to a software driver solution.
*   RaspiKey **can be used on anything**; on any device that accepts a standard USB keyboard, not just on Windows PCs. This means that you can connect your Apple keyboard to your smart TV or your game console with full keyboard functionality.
*   It is robust and responsive. It is based on a cut-down, readonly OS runtime version of the official Raspbian Linux. There is no shutdown process, you just unplug the device when you want. The custom software is implemented 100% in C & C++ and runs in the real-time kernel scheduler policy. 

Screenshots
-----------

<a href="images/shot1.png"><img width="100" height="100" src="images/shot1_tn.png"></a>
<a href="images/shot2.png"><img width="100" height="100" src="images/shot2_tn.png"></a>
<a href="images/shot3.png"><img width="100" height="100" src="images/shot3_tn.png"></a>
<a href="images/shot4.jpg"><img width="100" height="100" src="images/shot4_tn.jpg"></a>


Hardware and Software Requirements
----------------------------------

*   You need a [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/) device. The cheapest Zero W model (which is the one you need) currently sells for $10.
*   You then need a cheap 2GB (or more) micro-SD card to burn the RaspiKey software on.
*   You need access to a modern Windows PC with the Chrome browser to tether a keyboard to the Raspberry Pi board for the first time. You can then use the Raspberry Pi board and your keyboard on anything that has a USB keyboard port.

Installation and Use
--------------------

1.  Download [Apple’s Bonjour](https://developer.apple.com/bonjour/) and install it to Windows. The latest 32-bit and 64-bit versions are available [here](https://github.com/samartzidis/RaspiKey/tree/master/utilities).
2.  Download the latest RaspiKey **raspikey.xz** file from [here](https://github.com/samartzidis/RaspiKey/releases).
3.  Download [Etcher](https://etcher.io/).
4.  Use Etcher on Windows to burn raspikey.xz to a 2GB (if bigger doesn’t matter) micro-SD card.
5.  Insert the micro-SD card to the “Raspberry Pi Zero W” and then connect it to the computer using just the middle micro-USB port (labelled “USB” with tiny white letters). The Raspberry Pi will both draw power from this port as well as use it to communicate with the PC.
6.  Wait about 10 seconds. Then open Chrome on Windows at the address: **[http://raspikey.local](http://raspikey.local)**. You should then be able to use the RaspiKey configuration page.

Known Issues
------------

*   The battery level indicator does not yet work for the Apple Magic (A1644) keyboard.
*   The Ctrl-LShift-T combination does not work on Apple keyboards, use the Ctrl-RShift-T instead where applicable.

Apple Keyboards Keymap
----------------------

<table>
    <tr>
      <th>Input Physical Key(s)</th>
      <th>Output Logical Key</th>
    </tr>
    <tr>
      <td>Ctrl</td><td>Fn</td>
    </tr>
    <tr>
      <td>Fn</td><td>Left Ctrl</td>
    </tr>
    <tr>
      <td>Eject</td><td>Delete</td>
    </tr>
  </table>

<table>
    <tr>
      <th>Input Logical Key(s)</th>
      <th>Output Logical Key</th>
    </tr>
    <tr>
      <td>Fn + [F1-F6]</td><td>[F13-F18]</td>
    </tr>
    <tr>
      <td>Fn + Left Ctrl</td><td>Right Ctrl</td>
    </tr>
    <tr>
      <td>Fn + Enter</td><td>Insert</td>
    </tr>
    <tr>
      <td>Fn + P</td><td>Print Screen</td>
    </tr>
    <tr>
      <td>Fn + S</td><td>Scroll Lock</td>
    </tr>
    <tr>
      <td>Fn + B</td><td>Pause/Break</td>
    </tr>
    <tr>
      <td>Fn + Up</td><td>Page Up</td>
    </tr>
    <tr>
      <td>Fn + Down</td><td>Page Down</td>
    </tr>
    <tr>
      <td>Fn + Left</td><td>Home</td>
    </tr>
    <tr>
      <td>Fn + Right</td><td>End</td>
    </tr>
  </table>

Planned Futures
---------------

*   Support for the latest full-size Apple Magic keyboard.
*   Support for user customizable keymap and programmability.
*   Support for multimedia or special key functions for more Bluetooth keyboards.
*   Support for multiple Bluetooth keyboards connected in parallel.