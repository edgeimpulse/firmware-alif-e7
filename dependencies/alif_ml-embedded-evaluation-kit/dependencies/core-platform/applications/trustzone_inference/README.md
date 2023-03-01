# Arm Ethos-U TrustZone demonstration

This application is provided as an example of how to use TrustZone to separate
the inference from an application layer in secure and nonsecure world.
This software is primarily intended for guidance, to demonstrate how the
firmware can be partitioned.

### Building

Building with default settings requires CMake for the configuration and make for
building. This will produce an elf file which can be run on the FVP.

```
$ cmake -B build targets/corstone-300
$ make -C build trustzone
```

### ML Model and data
The model in this example has been generated from
```
 image_classification/mobilenet_v2_1.0_224/tflite_uint8
```
Found in the ARM ML-zoo open source github repository:
```
https://github.com/ARM-software/ML-zoo.git
```

The model is converted using vela (v2.0.1)
```
vela --cascading=True --config Arm/vela.ini --system-config=Ethos_U55_High_End_Embedded --memory-mode=Shared_Sram --accelerator-config=ethos-u55-128 --block-config-limit=0 ML-zoo/models/image_classification/mobilenet_v2_1.0_224/tflite_uint8/mobilenet_v2_1.0_224_quantized_1_default_1.tflite
```
Where the vela.ini is fetched from the vela git repository:

```
wget https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ethos-u-vela/+/refs/heads/master/ethosu/config_files/Arm/vela.ini
```

Then the compiled model representation has been converted to a header file using xxd.

```
xxd -i output/mobilenet_v2_1.0_224_quantized_1_default_1_vela.tflite > model.h
```

The input and expected output data has been taken from the ML-zoo and generated
from the respective .npy files to header files with:
```
python3 -c 'import numpy;i=numpy.load("ML-zoo/models/image_classification/mobilenet_v2_1.0_224/tflite_uint8/testing_input/input/0.npy");i.tofile("input.bin");' && xxd -i input.bin > input.h
```

```
python3 -c 'import numpy;i=numpy.load("ML-zoo/models/image_classification/mobilenet_v2_1.0_224/tflite_uint8/testing_output/output/0.npy");i.tofile("output.bin");' && xxd -i output.bin > output.h
```

The generated header files are then modified to adapt name and link attributes.

### Testing

Assuming that the Corstone-300 FVP has been downloaded, installed and placed in
the PATH variable. Then the software binary can be tested like this.

```
$ ctest trustzone
```

Note!
The expected output using the input data is not bitexact.

# License

The Arm Ethos-U Core Platform is provided under an Apache-2.0 license. Please
see [LICENSE.txt](LICENSE.txt) for more information.

# Contributions

Please see [README](../../README.md)

# Security

Please see [Security](../../SECURITY.md).
