def download_dependencies() {
    sh "rm -Rf dependencies"
    sh "python3 download_dependencies.py"
}

def setup_resources() {

    sh "python3 set_up_default_resources.py --additional-ethos-u-config-name ethos-u55-256"
}

/**@
* Builds project.
*
* @param build_type Release or Debug
* @param toolchain  Used toolchain, gcc or armclang
*/
def build_hp(String build_type, String toolchain) {

    build_path = "build_${toolchain}_hp_${build_type}".toLowerCase()
    cmake_cmd = "cmake .. -DTARGET_PLATFORM=ensemble -DTARGET_SUBSYSTEM=RTSS-HP -DUSE_CASE_BUILD=alif_img_class\\;alif_object_detection -DCMAKE_TOOLCHAIN_FILE=scripts/cmake/toolchains/bare-metal-${toolchain}.cmake -DCMAKE_BUILD_TYPE=${build_type} -DLOG_LEVEL=LOG_LEVEL_DEBUG"

    sh """#!/bin/bash
        export PATH=$PATH:/opt/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin
        mkdir $build_path
        pushd $build_path
        $cmake_cmd &&
        make -j 4
        exit_code=\$?
        popd
        exit \$exit_code"""
}

def build_he_tcm(String build_type, String toolchain) {

    build_path = "build_${toolchain}_he_tcm_${build_type}".toLowerCase()
    cmake_cmd = "cmake .. -DTARGET_PLATFORM=ensemble -DTARGET_SUBSYSTEM=RTSS-HE -DUSE_CASE_BUILD=alif_kws -DGLCD_UI=NO -DLINKER_SCRIPT_NAME=ensemble-RTSS-HE-TCM -DCMAKE_TOOLCHAIN_FILE=scripts/cmake/toolchains/bare-metal-${toolchain}.cmake -DCMAKE_BUILD_TYPE=${build_type} -DLOG_LEVEL=LOG_LEVEL_DEBUG"

    sh """#!/bin/bash
        export PATH=$PATH:/opt/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin
        mkdir $build_path
        pushd $build_path
        $cmake_cmd &&
        make -j 4
        exit_code=\$?
        popd
        exit \$exit_code"""
}

return [
    download_dependencies: this.&download_dependencies,
    setup_resources: this.&setup_resources,
    build_hp: this.&build_hp,
    build_he_tcm: this.&build_he_tcm,
]
