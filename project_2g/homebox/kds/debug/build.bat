set PATH=%PATH%;%~dp0\..\..\..\tools;%~dp0\..\..\..\toolchain\bin

make clean

make all -j4

arm-none-eabi-objcopy -O binary -R .test -R .rf_config homebox_bronze_frdmk64f.elf homebox_bronze_frdmk64f.bin

findstr m_text homebox_bronze_frdmk64f.map > homebox_bronze_frdmk64f.layout

gen_fota_pac homebox_bronze_frdmk64f.layout homebox_bronze_frdmk64f.bin

fota_split_img  ../../version.h  fota_pac_0000C000
