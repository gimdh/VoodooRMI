# VoodooRMI

A port for macOS of Synaptic's RMI code from Linux. RMI4 is used for touchscreens, touchpads, and other sensors - though this implementation only has code for trackpads/trackpoints. Many PS2 trackpads and sensors support other buses like I2C or SMBus, though SMBus is advantageous for macOS due to not requiring ACPI edits.

This driver communicates over SMBus or I2C.

## Supported Features
* Force Touch emulation for clickpads (press down clickpad and increase touched area)
* Up to four finger gestures (Though it can track up to 5 fingers)
* Buttons
* Trackpoint
* Power Management
* SMBus Communication
* I2C Communication

## Compatibility

**SMBus**  

Windows:
* Check under Device Manager for a Synaptics SMBus device

![](/docs/images/Windows-SMBus-Device.png)

Linux:
* If you are using intertouch (i.e. psmouse.intertouch=1) for your synaptics trackpad, then it's compatible
* Get `i2c-tools` from your package manager. Run `i2cdetect -l`, and note the number for SMBus (It's usually zero). Run `i2cdetect #` where # is the number you got from running the prior command. Synaptic devices are always at address 0x2c, so check at that address for anything other than `--`. It will usually appear as `UU` in my experiance if it's a Synaptics device.
  * If the trackpad does not show up, there is a chance that it will still work. There have been one or two examples of the trackpad not showing up but still being compatible.
* Likely compatible if you run `dmesg` and find a message along the lines of `"Your touchpad x says it can support a different bus."` and it's a synaptics trackpad.

**I2C**

macOS:
* If you already have VoodooI2CHID installed, check folllowing values in IORegistry
  * `VoodooI2CHIDDevice` has `HIDDescriptor` containing `VendorID` `0x6cb`, or
  * Parent device of `VoodooI2CHIDDevice` has `name` containing `SYN` or `SYNA`

Windows:
* Check for `HID-compliant touch pad` in device manager
  * In properties, verify `General` - `location` is `on I2C HID Device`  and `Details` - `Hardware Ids` contains `VID_06CB` (or `SYN`, `SYNA`)

Linux:
* Check for the presence of `i2c-SYN` in `dmesg`.
* Get `i2c-tools` from your package manager, and use the `i2cdetect` tool to see if there are any devices at address 0x2c for any bus that isn't SMBus. If you see it under SMBus, I'd use SMBus as the trackpad seemingly operates better under SMBus!

## Requirements

**SMBus**
* [VoodooSMBus](https://github.com/VoodooSMBus/VoodooSMBus)
  * Apple's SMBus **PCI** controller cannot load, as it interferes with VoodooSMBus.
* [VoodooPS2](https://github.com/acidanthera/VoodooPS2)
  * Needed for PS2 reset of the trackpad
  * Generally users should only add VoodooPS2Controller and VoodooPS2Keyboard. VoodooPS2Trackpad/VoodooPS2Mouse will cause VoodooRMI to not attach.
  * OpenCore users can just disable VoodooPS2Trackpad/VoodooPS2Mouse in their `config.plist`.
  * Clover users can go inside the VoodooPS2 kext and remove VoodooPS2Trackpad/VoodooPS2Mouse from PlugIns folder.

**I2C**
* [VoodooI2C](https://github.com/VoodooI2C/VoodooI2C)
  * Follow their [Documentation](https://voodooi2c.github.io) to identify if you need GPIO pinning.
  * Polling mode should just work
* If your device's ACPI name is not included below or marked as unknown, you may add it yourself and create a PR/issue

| Name | Main function |
|---|---|
| `SYN1B7F` | F12 |
| `SYNA0000` | F11 |
| `SYNA2393` | unknown |
| `SYNA2B2C` | F12 |
| `SYNA2B31` | F12 |
| `SYNA2B33` | F11 |
| `SYNA2B34` | unknown |
| `SYNA3105` | unknown |
| `SYNA3602` | unknown |
| `SYNA7500` | unknown |
| `SYNA7501` | unknown |

## Installation
1) Add the required kexts to your bootloader
2) Disable VoodooPS2Trackpad, VoodooPS2Mouse, and if applicable, VoodooInput from within the PS2 kext.
3) For OpenCore users, make sure to add VoodooInput, VoodooTrackpoint and RMISMBus/RMII2C to your Config.plist.
    * RMISMBus/RMII2C should be after VoodooRMI
    * All dependencies are found under `VoodooRMI.kext/Contents/PlugIns/`

You generally only want **one** of the two below kexts
* RMII2C is needed if using VoodooI2C for your trackpad
* RMISMBus is needed if using VoodooSMBus for your trackpad

There is no support for this kext being loaded into Library/Extensions or System/Library/Extensions. This likely won't break loading it, but test with injection first before sending in a bug report.

## Configuration

The values below can be edited under Info.plist within the kext itself - these can be changed without recompiling  
**Note** that using non-integer value causes undefined behaviour which may prevent the kext from loading

| Value | Default | Description |
| ----- | ------- | ----------- |
| `ForceTouchEmulation` | True | Allows Force Touch emulation on Clickpads |
| `ForceTouchMinPressure` | 90 | Minimum z value to trigger Force touch when clickpad is clicked |
| `DisableWhileTypingTimeout` | 100 | Milliseconds after typing in which to reject trackpad packets |
| `TrackpointMultiplier` | 20 | Multiplier used on trackpoint inputs (other than scrolling). This is divided by 20, so the default value of 20 will not change the output value at all |
| `TrackpointScrollMultiplierX` | 20 | Multiplier used on the x access when middle button is held down for scrolling. This is divded by 20. |
| `TrackpointScrollMultiplierY` | 20 | Same as the above, except applied to the Y axis |
| `TrackpointDeadzone` | 1 | Minimum value at which trackpoint reports will be accepted. This is subtracted from the input of the trackpoint, so setting this extremely high will reduce trackpoint resolution |
| `MinYDiffThumbDetection` | 200 | Minimum distance between the second lowest and lowest finger in which Minimum Y logic is used to detect the thumb rather than using the z value from the trackpad. Setting this higher means that the thumb must be farther from the other fingers before the y coordinate is used to detect the thumb, rather than using finger area. Keeping this smaller is preferable as finger area logic seems to only be useful when all 4 fingers are grouped together closely, where the thumb is more likely to be pressing down more |

Note that you can use Rehabman's ioio to set properties temporarily (until the next reboot).  
`ioio -s RMIBus ForceTouchEmulation false`

## Building
1) `git submodule update --init --recursive`
2) Build within XCode using the play button in the top left
    * RMISMBus/RMII2C will automatically build when building VoodooRMI


## Loading/Unloading
For loading, you may need to put RMII2C/RMISMBus's dependencies into the kextload command. Note that RMISMBus/RMII2C *depend* on VoodooRMI.

The below examples assume VoodooSMBus/VoodooI2C are in the same folder as VoodooRMI. If they are not, you will need to give the path to those kexts.
When manually loading from within macOS, keep in mind that csrutil needs to be partially disabled to allow unsigned kexts, and the kexts need to be owned by Root.

#### Changing owner of kext
```
// Note that this changes the owner of every kext in the directory your in
sudo chown -R root:wheel *.kext
```
#### Manually loading kext

Example for SMBus:
```
cd path/to/unziped-VoodooRMI_Debug
sudo kextutil -vvvv -d VoodooRMI.kext -d VoodooSMBus.kext VoodooRMI.kext/Contents/PlugIns/RMISMBus.kext
```
Example for I2C:
```
sudo kextutil -vvvv -d VoodooRMI.kext -d VoodooI2C.kext VoodooRMI.kext/Contents/PlugIns/RMII2C.kext
```

For unloading, you can use the bundle ids. This should unload cleanly, though you may need to unload twice in a row to get it to cooperate.
```
sudo kextunload -vvvv -b com.1Revenger1.RMISMBus -b com.1Revenger1.VoodooRMI
```

## Troubleshooting

Couple things to keep in mind:
1) Make sure VoodooSMBus/VoodooI2C is loading and attaching
    * [VoodooSMBus Troubleshooting](https://github.com/VoodooSMBus/VoodooSMBus/tree/dev#voodoosmbus-does-not-load)
    * [VoodooI2C Troubleshooting](https://voodooi2c.github.io/#Troubleshooting/Troubleshooting)
2) Make sure VoodooInput/VoodooTrackpoint are loading
3) IORegistryExplorer is a good way to see which Functions are loading, and what is/isn't loading
4) Getting logs is harder than it should be
    * If loading within macOS, you can use `sudo log show --last 5m | grep VRMI`
    * If injecting, you will want to add the boot arg `msgbuf=1048576` and use `sudo dmesg | grep VRMI`
