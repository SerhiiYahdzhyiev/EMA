# EMA Measurement Command Line Utility

Measure arbitrary applications executions with EMA from command line as a
single measurement region.

## Considerations

- Not Portable (UNIX specific, uses `_GNU_SOURCE` feature test macro).

## Build

### Prerequisites

1. Make.
2. Gcc.
3. EMA Installed.
4. `EMA_INSTALL_DIR` environment variable setup and pointing to the EMA's
   installation location.

### Steps

1. Run `make` from this directory.

## Usage

Make built executable (`ema_measure`) available in your `PATH` for convenience
(this is completely optional you can invoke it as `./ema_measure`),
then run it passing application execution commands, e.g `ema_measure sleep 10`.
