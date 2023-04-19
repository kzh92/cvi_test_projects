rm -rf ./generated/upgrade
cp -rf ./generated/images ./generated/upgrade
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/fip_fsbl.bin ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/yoc.bin ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/boot ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
python3 ../../boards/tools/common/raw2cimg.py ./generated/images/boot0 ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/imtb ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/misc.bin ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/partwx.bin ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/pusr1.bin ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
#python3 ../../boards/tools/common/raw2cimg.py ./generated/images/pusr2.bin ./generated/upgrade ./generated/upgrade/partition_alios_spinor.xml
