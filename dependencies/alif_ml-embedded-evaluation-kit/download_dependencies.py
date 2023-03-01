#!/usr/bin/env python3

# This file was ported to work on Alif Semiconductor Ensemble family of devices.

#  Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
#  Use, distribution and modification of this code is permitted under the
#  terms stated in the Alif Semiconductor Software License Agreement
#
#  You should have received a copy of the Alif Semiconductor Software
#  License Agreement with this file. If not, please write to:
#  contact@alifsemi.com, or visit: https://alifsemi.com/license

#  SPDX-FileCopyrightText: Copyright 2021-2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
#  SPDX-License-Identifier: Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

"""This script does effectively the same as "git submodule update --init" command."""
import logging
import sys
import tarfile
import tempfile
from urllib.request import urlopen
from zipfile import ZipFile
from pathlib import Path

TF = "https://github.com/tensorflow/tflite-micro/archive/28770e4cac1e5dae85b55a0d47d5315d35755d04.zip"
CMSIS = "https://github.com/ARM-software/CMSIS_5/archive/81564cfb339ebf9def167a50693733f8a1e1471e.zip"
CMSIS_DSP = "https://github.com/ARM-software/CMSIS-DSP/archive/b46a2f86b5c9d8247ea5417fc0e0022876b80dcf.zip"
CMSIS_NN = "https://github.com/ARM-software/CMSIS-NN/archive/refs/tags/v4.0.0.zip"
CMSIS_ENSEMBLE = "https://github.com/alifsemi/alif_ensemble-cmsis-dfp/archive/dd07ce1b3f25cf588455167a3583e8579c6dca8b.zip"
ETHOS_U_CORE_DRIVER = "https://git.mlplatform.org/ml/ethos-u/ethos-u-core-driver.git/snapshot/ethos-u-core-driver-22.11.tar.gz"
ETHOS_U_CORE_PLATFORM = "https://git.mlplatform.org/ml/ethos-u/ethos-u-core-platform.git/snapshot/ethos-u-core-platform-22.11.tar.gz"
LVGL = "https://github.com/lvgl/lvgl/archive/dbb15bb3ea0365373bc1ba8b182556f937e61e7d.zip"
ARM2D = "https://github.com/ARM-software/Arm-2D/archive/refs/tags/v1.1.0.zip"


def download(url_file: str, post_process=None):
    with urlopen(url_file) as response, tempfile.NamedTemporaryFile() as temp:
        logging.info(f"Downloading {url_file} ...")
        temp.write(response.read())
        temp.seek(0)
        logging.info(f"Finished downloading {url_file}.")
        if post_process:
            post_process(temp)


def unzip(file, to_path):
    with ZipFile(file) as z:
        for archive_path in z.infolist():
            archive_path.filename = archive_path.filename[archive_path.filename.find("/") + 1:]
            if archive_path.filename:
                z.extract(archive_path, to_path)
                target_path = to_path / archive_path.filename
                attr = archive_path.external_attr >> 16
                if attr != 0:
                    target_path.chmod(attr)


def untar(file, to_path):
    with tarfile.open(file) as z:
        for archive_path in z.getmembers():
            index = archive_path.name.find("/")
            if index < 0:
                continue
            archive_path.name = archive_path.name[index + 1:]
            if archive_path.name:
                z.extract(archive_path, to_path)


def main(dependencies_path: Path):

    download(CMSIS,
             lambda file: unzip(file.name, to_path=dependencies_path / "cmsis"))
    download(CMSIS_DSP,
             lambda file: unzip(file.name, to_path=dependencies_path / "cmsis-dsp"))
    download(CMSIS_NN,
             lambda file: unzip(file.name, to_path=dependencies_path / "cmsis-nn"))
    download(CMSIS_ENSEMBLE,
             lambda file: unzip(file.name, to_path=dependencies_path / "cmsis-ensemble"))
    download(ETHOS_U_CORE_DRIVER,
             lambda file: untar(file.name, to_path=dependencies_path / "core-driver"))
    download(ETHOS_U_CORE_PLATFORM,
             lambda file: untar(file.name, to_path=dependencies_path / "core-platform"))
    download(TF,
             lambda file: unzip(file.name, to_path=dependencies_path / "tensorflow"))
    download(LVGL,
             lambda file: unzip(file.name, to_path=dependencies_path / "lvgl"))
    download(ARM2D,
             lambda file: unzip(file.name, to_path=dependencies_path / "Arm-2D"))


if __name__ == '__main__':
    logging.basicConfig(filename='download_dependencies.log', level=logging.DEBUG, filemode='w')
    logging.getLogger().addHandler(logging.StreamHandler(sys.stdout))

    download_dir = Path(__file__).parent.resolve() / "dependencies"

    if download_dir.is_dir():
        logging.info(f'{download_dir} exists. Skipping download.')
    else:
        main(download_dir)
