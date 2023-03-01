set -e
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

echo "Building standalone classifier"

cd $SCRIPTPATH

rm -rf build
mkdir build
cd build
cmake \
-DTARGET_PLATFORM=ensemble \
-DTARGET_SUBSYSTEM=RTSS-HP \
-DTARGET_BOARD=AppKit_Alpha1 \
-DCONSOLE_UART=2 \
-DCMAKE_TOOLCHAIN_FILE=scripts/cmake/toolchains/bare-metal-gcc.cmake \
-DCMAKE_BUILD_TYPE=Release \
-DLOG_LEVEL=LOG_LEVEL_DEBUG ..

make -j

echo "Building standalone classifier OK"
