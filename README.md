# mlx75322-spi-driver

SPI driver build:
=================

On any environment, to build the library, we need:

- a target GCC toolchain installed and available;
- 'make' tool

Raspberry Pi3 with Raspbian system is preferred (for default configuration).

To build the library:
---------------------

2) For Raspberry Pi (HAL, pthread library, and stdlib is used) run:
  'make all [TARGET=75322-raspi]' (default is 75322-raspi)
1) For the common (empty-target, with weak interface functions) run:
  'make all TARGET=75322'

The output library and its API header-files set will appear in "build/" folder.

To get the documentation run: 'make doxy'
