set -e
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

echo "Building Alif E7 firmware"

cd $SCRIPTPATH

rm -rf build
mkdir build
cd build
cmake .. -DTARGET_SUBSYSTEM=HP -DCMAKE_TOOLCHAIN_FILE=../scripts/cmake/toolchains/bare-metal-gcc.cmake
make -j8

echo "Building firmware OK"