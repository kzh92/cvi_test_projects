rm -f ./z_fumaker_console
rm -f CMakeCache.txt
cmake .
make clean
make
./z_ufmaker_console