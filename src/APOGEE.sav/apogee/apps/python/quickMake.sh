autoreconf -if
rm -r build
mkdir build
cd build
../configure CPPFLAGS=-I/usr/local/include/libapogee-2.1/apogee
make