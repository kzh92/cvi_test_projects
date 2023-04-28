cd pack
cmake .
make
./config_app
cd ../
make -j7 usb_cam PROJECT=turnkey_180xb
