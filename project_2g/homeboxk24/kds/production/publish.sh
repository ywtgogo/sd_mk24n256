#!/bin/bash
mkdir -p $1
cd homeboxk24/kds/produce_test
cp -rf produce_test $1
cd -
cd homeboxk24/kds/production
cp -rf origin fota_test $1
cd -
cd bootloaderk24/kds/production
cp -rf bootloader $1
cd -

mkdir -p $1/flash_test/2g/module
mkdir -p $1/flash_test/2g/img/audio_img
mkdir -p $1/flash_test/2g/img/homebox_img
mkdir -p $1/flash_test/2g/img/produce_img
cp stc/Objects/power.hex $1/flash_test/2g/module
cp protocol/wave.bin $1/flash_test/2g/img/audio_img
cp $1/origin/fota_node_* $1/origin/ver_info.txt $1/flash_test/2g/img/homebox_img
cp $1/produce_test/produce_img.bin $1/produce_test/ver_info.txt $1/flash_test/2g/img/produce_img
