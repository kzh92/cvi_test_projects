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
	status=$?
	cd ../
	rm -f ./solutions/usb_cam/generated/images.zip
	if [ $status -eq 1 ] ; then
		echo "========================================"
		echo "going to build D10/D10A firmware..."
		echo "========================================"
		make -j4 usb_cam PROJECT=turnkey_180xb
	else
		echo "========================================"
		echo "going to build D20/D20A firmware..."
		echo "========================================"
		make -j4 usb_cam PROJECT=turnkey
	fi
    cd pack
    ./config_app pack
    cd ../
fi
