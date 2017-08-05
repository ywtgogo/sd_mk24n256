#!/bin/bash
mkdir -p $1
cd homeboxk24/kds/wifi_produce_test
cp -rf wifi_produce_test $1
cd -
cd homeboxk24/kds/wifi_production
cp -rf origin fota_test $1
cd -
cd bootloaderk24/kds/wifi_production
cp -rf bootloader $1
cd -

mkdir -p $1/flash_test/wifi/module
mkdir -p $1/flash_test/wifi/img/audio_img
mkdir -p $1/flash_test/wifi/img/homebox_img
mkdir -p $1/flash_test/wifi/img/produce_img
cp esp8266/* $1/flash_test/wifi/module
cp protocol/wave.bin $1/flash_test/wifi/img/audio_img
cp $1/origin/fota_node_* $1/origin/ver_info.txt $1/flash_test/wifi/img/homebox_img
cp $1/wifi_produce_test/produce_img.bin $1/wifi_produce_test/ver_info.txt $1/flash_test/wifi/img/produce_img
