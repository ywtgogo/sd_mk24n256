#!/bin/bash

export PATH=$PATH:/opt/Freescale/KDS_3.0.0/toolchain/bin:$PWD/../tools



cd bootloaderk24/kds/production
make clean
make -k -j2 all
if [ $? -ne 0 ]
then
	exit 1
fi
arm-none-eabi-objcopy -O binary --gap-fill 0xff hb_bootloader_k24.elf hb_bootloader_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi

mkdir bootloader
mv hb_bootloader_k24.* bootloader
cd -




cd homeboxk24/kds/produce_test

make clean
make -k -j4 all
if [ $? -ne 0 ]
then
	exit 1
fi
arm-none-eabi-objcopy -O binary -R .test -R .hbb_config project_k24.elf project_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi
gen_pac 0x9000 project_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi
arm-none-eabi-objcopy -O binary -j .ver_info project_k24.elf ver_info.bin
if [ $? -ne 0 ]
then
	exit 1
fi
split_pac ver_info.bin fota_pac_00009000
if [ $? -ne 0 ]
then
	exit 1
fi
dd if=/dev/zero ibs=1k count=256 | tr "\000" "\377" > produce_img.bin
dd if=../../../bootloaderk24/kds/production/bootloader/hb_bootloader_k24.bin of=produce_img.bin bs=1 seek=0 conv=notrunc
dd if=project_k24.bin of=produce_img.bin bs=1 seek=36864  conv=notrunc
dd if=fota_node_0 of=produce_img.bin bs=1 seek=258048 conv=notrunc

mkdir produce_test
mv fota_node_* project_k24.* ver_info.txt produce_img.bin produce_test
cd -



cd homeboxk24/kds/production

make clean
make -k -j4 all
if [ $? -ne 0 ]
then
	exit 1
fi
arm-none-eabi-objcopy -O binary -R .test -R .hbb_config project_k24.elf project_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi
gen_pac 0x9000 project_k24.bin
if [ $? -ne 0 ]
then
	exit 1
fi
arm-none-eabi-objcopy -O binary -j .ver_info project_k24.elf ver_info.bin
if [ $? -ne 0 ]
then
	exit 1
fi
split_pac ver_info.bin fota_pac_00009000
if [ $? -ne 0 ]
then
	exit 1
fi
dd if=/dev/zero ibs=1k count=256 | tr "\000" "\377" > homebox_production.bin
dd if=../../../bootloaderk24/kds/production/bootloader/hb_bootloader_k24.bin of=homebox_production.bin bs=1 seek=0 conv=notrunc
dd if=project_k24.bin of=homebox_production.bin bs=1 seek=36864  conv=notrunc
dd if=fota_node_0 of=homebox_production.bin bs=1 seek=258048 conv=notrunc

swstring=$(grep swver ver_info.txt|grep -o '[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*')

v3=$(echo -n $swstring|cut -d. -f1)
v2=$(echo -n $swstring|cut -d. -f2)

mkdir origin
mv fota_node_* project_k24.* ver_info.txt homebox_production.bin origin

# v1=1
v0=0

for((v1=1;v1<100;v1++))
do
        fake_ver fota_pac_00009000 0x428 $v3.$v2.$v1.$v0
        fake_ver ver_info.bin 0x20 $v3.$v2.$v1.$v0
        split_pac ver_info.bin fota_pac_00009000
        mkdir $v3.$v2.$v1.$v0
        mv fota_node_*  $v3.$v2.$v1.$v0
done

mkdir fota_test
mv $v3.* fota_test
