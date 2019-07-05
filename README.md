# AVR ADC to I2C bridge

This is a tiny firmware for AVR microcontrollers to provide their ADC reading via I2C, including some examples to obtain that reading on a Linux/POSIX system using C and Python code.
It uses [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile) and the [Arduino](https://www.arduino.cc/) toolchain, but none of the Arduino libraries.
The version in this repository has been built and tested with the ATmega328P controller and the USBasp programmer. For other controllers the `Makefile` should be modified accordingly.

## Getting Started

For a quick start, use a Linux distribution of your choice and set up your toolchain as follows:

```
mkdir ~/.arduino-toolchain
cd ~/.arduino-toolchain
wget https://downloads.arduino.cc/arduino-1.8.9-linux64.tar.xz
tar xvf arduino-1.8.9-linux64.tar.xz
mv arduino-1.8.9 arduino
git clone https://github.com/sudar/Arduino-Makefile.git arduino-mk
```

The used version of the Arduino toolchain can be adapted. Per default, the toolchain and Arduino-Makefile are expected in `~/.arduino-toolchain`, but that can be changed in the `Makefile`.

To just build the firmware, run: `make`

To build and flash with an USBasp, run: `make ispload`

To build and flash it to an Arduino, run: `make upload` (Note: You might have to set the baud rate in the `Makefile`)

## Output Format

Four registers are provided on the I2C bus which should be always read in order:

* `0x00`: The request of this address copies the last values into a temporary buffer and returns `0xff`.
* `0x01`: Returns the MSB of the last raw reading.
* `0x02`: Returns the LSB of the last raw reading.
* `0x03`: Returns MSB xor LSB as a basic checksum.
* All other commands return `0xaa`.

The default I2C slave address is `0x42` and can be easily changed in `main.c`.

It should be noted that the raw reading of the AVR 10-bit ADC is returned and the internal voltage reference is used by default.
The reading is updated every 100 milliseconds by default. This can be changed in `main.c` as well.

Example code to access and decode the data on a Linux machine (tested on a Raspberry Pi) can be found under `examples/`.

## Configuration

The firmware as well as its build process can be easily adapted to individual needs and further AVR boards.

### Make Options

* `AVR_TOOLS_DIR=`: Use the system's toolchain instead of the Arduino one.
* `VERBOSE=1`: Output compile commands and additional information.
* `debug=1`: Compile with debug symbols and the `DEBUG` define.

Further options are documented in the `Makefile` and the documentation of [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile).

### Porting to Another Board / MCU

See the top of the `Makefile` for board settings. You should adapt at least the model, clock values, and fuse bits.
Afterwards, check `main.c` and adapt the ADC and debug pin settings to that of your controller.

## Debugging

First of all, the device should be visible on the I2C bus. Check with: `i2cdetect -y <bus ID>`

If that is the case, the values documented above can be read with: `i2cdump <bus ID> <address>`

During each loop iteration, a debug pin (by default `PB0`) should be toggled. This can be checked using an oscilloscope to verify that the loop runs properly.

## License

This code is licensed under the MIT license. See [LICENSE](LICENSE) for details.
