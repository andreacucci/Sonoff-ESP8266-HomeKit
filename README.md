# Sonoff-HomeKit implementation

Firmware to control your Sonoff natively and directly through HomeKit.

Only [iTead Sonoff Basic](http://sonoff.itead.cc/en/products/sonoff/sonoff-basic) is supported.

# Build instructions for Sonoff-Homekit

## Prerequisites
- [ESP8266-HomeKit](https://github.com/HomeACcessoryKid/ESP8266-HomeKit)  

## Build Instructions

### Setting your WiFi
If you did not set up your WiFi, do so by uncommenting the first block of code in the user_init routine and filling the SSID and password. After, do not forget to to comment it out again and remove your password. Its bad for flash to write it each time and you would not want to upload your password to GitHub by accident.

### Compiling

```bash
~$ export SDK_PATH=<your SDK directory>
~$ export BIN_PATH=<where your binaries will be>
~$ cd Sonoff-ESP8266-HomeKit
~/Sonoff-ESP8266-HomeKit$ ./gen_misc.sh
```
Use all the defaults or experiment with alternatives...  
This will create (after several minutes) the files:
* $BIN_PATH/eagle.flash.bin  
* $BIN_PATH/eagle.irom0text.bin

If you answer all the questions by default, ESP8266-HomeKit will need more space than originally foreseen in ESP8266_RTOS_SDK 1.5.0 which was to start irom at 0x20000. To address this it is needed to change the `ESP8266_RTOS_SDK/ld/eagle.app.v6.ld` file:

```diff
diff ld/eagle.app.v6.ld.0 ld/eagle.app.v6.ld
29c29,30
<   irom0_0_seg :                       	org = 0x40220000, len = 0x5C000
---
> /*irom0_0_seg :                       	org = 0x40220000, len = 0x5C000 */
>   irom0_0_seg :                       	org = 0x40214000, len = 0x67000
```

for convenience also change the master `ESP8266_RTOS_SDK/Makefile`:
```diff
diff ESP8266/source/ESP8266_RTOS_SDK-master-v1.5.0/Makefile Makefile 
271c271
< 	@echo "eagle.irom0text.bin---->0x20000"
---
> 	@echo "eagle.irom0text.bin---->0x14000"
```
### Flashing:

```bash
esptool.py --baud 230400 -p /dev/yourUSBid write_flash 0x00000 $BIN_PATH/eagle.flash.bin 0x14000 $BIN_PATH/eagle.irom0text.bin
```

### Usage:

- After boot, if the device is not yet paired, an srp-key is calculated in about 25 seconds
- After that the server starts and mulicastdns starts to advertize
- The default pincode is `031-45-154`
- The code writes clients keys to sector 0x13000
- After you unpair from the original device, the keys will be destroyed and you can pair again.
- if something went wrong, go to the foundation, and change the `signature` string to something else and you can start from scratch
