# EMA - Energy Measurement for Applications

The EMA framework is a tool for measuring energy consumption of applications.
It allows the integration of various energy measurement devices with the
built-in plugin system. It also allows users to define measurement regions
directly in the source code.

The development was started under [GreenHPC](https://greenhpc.eu)
initiative.

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [Usage](#usage)
4. [Authors](#authors)
5. [External Contributors](#external-contributors)
6. [License](#license)
7. [Funding](#funding)

## Overview

EMA is designed to provide user-friendly API's for measuring the energy
consumption and/or individual energy values in arbitrary applications. It
provides sets of predefined *High Level* macros and more *Low Level* functions
allowing users to do following actions:

   - define measurement regions in the source code of users applications
   - filter measured devices
   - filter plugins
   - control the outputs of measurements
   - get individual energy and/or time values in arbitrary locations of
     applications source code

*You will find more details in [Usage](#usage) section.*

### Plugins

EMA `Plugins` encapsulate realization details of energy measurements. They have
a well-defined interface which is the part of EMA's core. EMA's flexibility is
based on `Plugins` as well: users can write their own plugins to add support
for new hardware devices or extend/fix/modify measurements realization for
existing ones.

Current version comes with the following pre-developed plugins:

   - [RAPL](https://www.intel.de/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.pdf)
     plugin (CPUs)
   - [NVML](https://developer.nvidia.com/management-library-nvml) plugin
     (Nvidia GPUs)
   - [MQTT](https://mqtt.org) plugin (Custom hardware setups
     over network)

## Installation

### Base Prerequisites

   - [CMake](https://cmake.org)

### General Steps

1. [Clone the repository](https://git-scm.com/docs/git-clone) and navigate to its root directory.

2. Create `build` directory and navigate to it:

   ```bash
   mkdir build && cd build
   ```

3. Configure and generate build files.

   There are plenty of ways how you can do this, if you're not familiar with
   [CMake](https://cmake.org) check out [this guide](https://preshing.com/20170511/how-to-build-a-cmake-based-project/),
   or you can also go through related sections of [official documentation](https://cmake.org/documentation/).

4. Build EMA:

   ```bash
   make
   ```

   *NOTE: This command will also build test executables from `tests` directory.*

5. Install:

   ```bash
   make install
   ```

6. Update `LD_LIBRARY_PATH`:

   ```bash
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<EMA_install_dir>/lib
   ```

### Documentation

This project uses [Sphinx](https://www.sphinx-doc.org/en/master/) for building
documentation.

#### Prerequisites

- [Python 3](https://python.org)
- [Clang](https://clang.llvm.org/)
- [Sphinx](https://www.sphinx-doc.org/en/master/)
- [Sphinx C Autodoc](https://sphinx-c-autodoc.readthedocs.io/en/latest/)

#### Steps

1. Create Python environment, install and set up all dependencies.
2. Navigate to the `docs` directory.
3. Build desired type of documentation with `make`.

## Usage

### Example

```C
#include <EMA.h>

int main(int argc, char **argv)
{
    /* Initialize EMA.
     *
     * EMAs data structures will be initialized and registered plugins will be
     * loaded.
     */
    int err = EMA_init(NULL);

    /* Check for errors.
     *
     * If `err` is 0 then no error occurred.
     */
    if( err )
        return 1;

    /* Create a filter to disable the NVML plugin.
     *
     * `EMA_filter_exclude_plugin` is a predefined function to filter out
     * a plugin by name.
     *
     * It is also possible to create a custom filter with function
     * `EMA_filter_create`. This function takes a user-defined callback for
     * individual filtering.
     *
     * A filter is not required if `EMA_REGION_DEFINE` is used.
     */
    Filter *filter = EMA_filter_exclude_plugin("NVML");

    /* Initialize a region.
     *
     * `EMA_REGION_DECLARE` declares a region handle that holds the measurement
     * data.
     *
     * `EMA_REGION_DEFINE` defines the region by setting a name.
     *
     * `EMA_REGION_DEFINE_WITH_FILTER` is an extended version of
     * `EMA_REGION_DEFINE` that takes an optional filter argument.
     */
    EMA_REGION_DECLARE(region);
    EMA_REGION_DEFINE_WITH_FILTER(&region, "region", filter);
    /* Alternatively: `EMA_REGION_DEFINE(&region, "region");`. */

    /* Start a measurement.
     *
     * `EMA_REGION_BEGIN` starts a measurement and stores the data in the
     * region handle.
     */
    EMA_REGION_BEGIN(region);

    /* The code to be measured. */
    sleep(1);

    /* Stop the measurement.
     *
     * `EMA_REGION_END` stops a measurement and stores the data in the region
     * handle.
     */
    EMA_REGION_END(region);

    /* Release the filter.
     *
     * `EMA_filter_finalize` releases a previously created filter.
     *
     * This is just required if a filter was initialized.
     */
    EMA_filter_finalize(filter);

    /* Finalize EMA.
     *
     * `EMA_finalize` cleans up the EMA framework and prints out the
     * measurement data.
     */
    EMA_finalize();

    return 0;
}
```

### Compiling applications

```bash
gcc -O2 -Wall -o main main.c -I<install_dir>/include -L<install_dir>/lib -lEMA
```

### Output

After the execution output files will be generated. Each process generates a
CSV-like output file `output.EMA.<pid>`. It contains information about the
measurements. The first line is the header. Subsequent lines represent
measurement results per region and device.

| name | description |
| ---- | ----------- |
| thread | Thread ID. |
| region_idf | Region identifier as defined by the user (e.g. with `EMA_REGION_DEFINE`). |
| file | File in which the region was defined. |
| line | Line number in which the region was defined. |
| function | Name of the function in which the region was defined. |
| visits | Number of visitis for that region. |
| device_name | Name of the device under measurement. |
| energy | Measured energy consumption in uJ (micro joules). |
| time | Measured duration in us (micro seconds). |

### Troubleshooting

#### Accessing RAPL values

After every system start, the system administrator need to set read permissions
for users/groups to access the following files:
  - /sys/class/powercap/intel-rapl:<socket>/energy_uj
  - /sys/class/powercap/intel-rapl:<socket>:<rapl_device>/energy_uj

## Authors

| name | email |
| ---- | ----- |
| Johannes Spazier | spazier@perfacct.eu |
| Danny Puhan | puhan@perfacct.eu |
| Serhii Yahdzhyiev | yahdzhyiev@perfacct.eu |

## External Contributors

| name | email |
| ---- | ----- |
| Hannes Signer | hannes.signer@uni-potsdam.de |

## License

This project is licensed under the terms of the BSD-3 license.
See LICENSE.md

## Funding

The development of EMA is funded by the BMBF Germany in the context of the
[NAAICE](https://gauss-allianz.de/en/project/title/NAAICE) project
([GreenHPC grant](https://gauss-allianz.de/en/project/call/Richtlinie%20zur%20F%C3%B6rderung%20von%20Verbundprojekten%20auf%20dem%20Gebiet%20des%20energieeffizienten%20High-%E2%80%8BPerformance%20Computings%20%28GreenHPC%29)).

<div align=center>
  <img
    src="./assets/img/internet_in_farbe_de.jpg"
    alt="BMBF logo"
    width="150"
  />
</div>
