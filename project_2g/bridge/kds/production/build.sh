#!/bin/bash

cd ../../..
export PATH=$PATH:/opt/Freescale/KDS_3.0.0/toolchain/bin:$PWD/tools

cd bootloaderk24/kds/production
make clean
make -k -j2 all
cd -

cd bridge/kds/production
make clean
make -k -j4 all
if [ $? -ne 0 ]
then
	exit 1
fi

arm-none-eabi-objcopy -O binary -R .test -R .hbb_config bridge_k24.elf bridge_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi

gen_pac 0x9000 bridge_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi
arm-none-eabi-objcopy -O binary -j .ver_info bridge_k24.elf ver_info.bin
if [ $? -ne 0 ]
then
	exit 1
fi
split_pac ver_info.bin fota_pac_00009000
if [ $? -ne 0 ]
then
	exit 1
fi
mkdir origin

mv fota_node_* bridge_k24.* ver_info.txt origin

v3=0
v2=5
# v1=1
v0=9

for((v1=1;v1<100;v1++))
do
        fake_ver fota_pac_00009000 0x428 $v3.$v2.$v1.$v0
        fake_ver ver_info.bin 0x20 $v3.$v2.$v1.$v0
        split_pac ver_info.bin fota_pac_00009000
        mkdir $v3.$v2.$v1.$v0
        mv fota_node_* ver_info.txt $v3.$v2.$v1.$v0
done

mkdir fota_test
mv 0.* fota_test
