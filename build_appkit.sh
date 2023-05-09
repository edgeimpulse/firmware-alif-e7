set -e
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

echo "Building standalone classifier"

cd $SCRIPTPATH

rm -rf build_app
mkdir build_app
cd build_app
cmake \
-DTARGET_PLATFORM=ensemble \
-DTARGET_SUBSYSTEM=RTSS-HP \
-DTARGET_BOARD=AppKit_Alpha2 \
-DCONSOLE_UART=2 \
-DCMAKE_TOOLCHAIN_FILE=scripts/cmake/toolchains/bare-metal-gcc.cmake \
-DCMAKE_BUILD_TYPE=Release \
-DLOG_LEVEL=LOG_LEVEL_DEBUG ..

make -j

echo "Building standalone classifier OK"
