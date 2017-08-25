#!/bin/bash
:<<!
******NOTICE******
MUST set SDK_PATH & BIN_PATH firstly!!!
example:
export SDK_PATH=~/esp_iot_sdk_freertos
export BIN_PATH=~/esp8266_bin
!

echo "based on gen_misc.sh version 20150911"
echo ""

if [ ! $SDK_PATH ]; then
    export SDK_PATH=$(dirname $(dirname $(pwd)))
fi
echo "SDK_PATH:"
echo "$SDK_PATH"
echo ""

if [ ! $BIN_PATH ]; then
    export BIN_PATH=$SDK_PATH/bin
fi
echo "BIN_PATH:"
echo "$BIN_PATH"
echo ""

echo "Please check SDK_PATH & BIN_PATH, enter (Y/y) to continue:"
read input

if [[ $input != Y ]] && [[ $input != y ]]; then
    exit
fi

echo ""

echo "Please follow below steps(1-5) to generate specific bin(s):"
echo "STEP 1: use boot_v1.2+ by default"
boot=new

echo "boot mode: $boot"
echo ""

echo "STEP 2: choose bin generate(0=eagle.flash.bin+eagle.irom0text.bin, 1=user1.bin, 2=user2.bin)"
echo "enter (0/1/2, default 0):"
read input

if [ -z "$input" ]; then
    if [ $boot != none ]; then
    	boot=none
	echo "ignore boot"
    fi
    app=0
    echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
elif [ $input == 1 ]; then
    if [ $boot == none ]; then
    	app=0
	echo "choose no boot before"
	echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
    else
	app=1
        echo "generate bin: user1.bin"
    fi
elif [ $input == 2 ]; then
    if [ $boot == none ]; then
    	app=0
	echo "choose no boot before"
	echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
    else
    	app=2
    	echo "generate bin: user2.bin"
    fi
else
    if [ $boot != none ]; then
    	boot=none
	echo "ignore boot"
    fi
    app=0
    echo "generate bin: eagle.flash.bin+eagle.irom0text.bin"
fi

echo ""

echo "STEP 3: choose spi speed(0=20MHz, 1=26.7MHz, 2=40MHz, 3=80MHz)"
echo "enter (0/1/2/3, default 2):"
read input

if [ -z "$input" ]; then
    spi_speed=40
elif [ $input == 0 ]; then
    spi_speed=20
elif [ $input == 1 ]; then
    spi_speed=26.7
elif [ $input == 3 ]; then
    spi_speed=80
else
    spi_speed=40
fi

echo "spi speed: $spi_speed MHz"
echo ""

echo "STEP 4: use DOUT by default"

spi_mode=DOUT

echo "spi mode: $spi_mode"
echo ""

echo "STEP 5: 1024KB( 512KB+ 512KB) by default"

spi_size_map=2
echo "spi size: 1024KB"
echo "spi ota map:  512KB + 512KB"

echo ""
date
echo ""

make clean

make BOOT=$boot APP=$app SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map

echo ""
date
