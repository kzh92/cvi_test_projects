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
	make -j4 usb_cam PROJECT=turnkey
	echo "Compressing prim..."
	rm -f lz4 ./solutions/usb_cam/generated/images/prim.lz4
	lz4 ./solutions/usb_cam/generated/images/prim
	mv ./solutions/usb_cam/generated/images/prim.lz4 ./solutions/usb_cam/generated/images/prim
fi

