# mlx75322-spi-driver

SPI driver build:
=================

On any environment, to build the library, we need:

- a target GCC toolchain installed and available;
- 'make' tool

Debian-based system is preferred.

To build the library:
---------------------

1) For the common (empty-target, with weak interface functions) run:
  'make all'
2) For Raspberry Pi (HAL, pthread library, and stdlib is used) run:
  'make all TARGET=75322-raspi'

The output library and its API header-files set will appear in "build/" folder.

To get the documentation run: 'make doxy'
