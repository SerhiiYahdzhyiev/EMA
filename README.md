# **EMA - Energy Measurement for Applications**

# What is EMA?

The EMA framework is a tool for profiling the energy consumption of
self-defined code regions of HPC applications. It allows the integration of
various energy measurement devices with the built-in plugin system.

The development initialy started under [GreenHPC](https://greenhpc.eu)
initiative.

# Quickstart

## Installation

**Build from tarball**
```
tar -xf <filename>.tar.gz
mkdir <filename>/build
cd <filename>/build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=<install_dir>
make install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<install_dir>/lib
```

**Build from git checkout**
```
git clone <URL>
mkdir <filename>/build
cd <filename>/build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=<install_dir>
make install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<install_dir>/lib
```

## Accessing RAPL values

After every system start, the system administrator need to set read permissions
for users/groups to access the following files:
  - /sys/class/powercap/intel-rapl:<socket>/energy_uj
  - /sys/class/powercap/intel-rapl:<socket>:<rapl_device>/energy_uj

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

# License

This project is licensed under the terms of the BSD-3 license.
See LICENSE.md

# Authors/Contact

| name | email |
| ---- | ----- |
| Johannes Spazier | spazier@perfacct.eu |
| Danny Puhan | puhan@perfacct.eu |

# External Contributors

| name | email |
| ---- | ----- |
| Max Lübke | max.lubke@uni-potsdam.de |
| Hannes Signer | hannes.signer@uni-potsdam.de |

# Funding

The development of EMA was funded by the BMBF Germany in the context of the
[NAAICE](https://gauss-allianz.de/en/project/title/NAAICE) project
([GreenHPC grant](https://gauss-allianz.de/en/project/call/Richtlinie%20zur%20F%C3%B6rderung%20von%20Verbundprojekten%20auf%20dem%20Gebiet%20des%20energieeffizienten%20High-%E2%80%8BPerformance%20Computings%20%28GreenHPC%29)).

<div align=center>
  <img
    src="https://raw.githubusercontent.com/wiki/RRZE-HPC/likwid/images/BMBF.png"
    alt="BMBF logo"
    width="150"
  />
</div>
