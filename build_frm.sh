if [ ! -f ".yoc" ]; then
    echo "Initialisng yoc..."
    yoc init
fi
cd pack
cmake .
rm -f ./config_app
make
if [ ! -f "./config_app" ]; then
    echo "Failed to compile pack app."
    cd ../
else
    ./config_app
    cd ../
	rm -f ./solutions/usb_cam/generated/images.zip
    make -j4 usb_cam PROJECT=turnkey_180xb
    cd pack
    ./config_app pack
    cd ../
fi
