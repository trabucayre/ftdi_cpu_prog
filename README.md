# ftdi_cpu_prog
tool to automate the programming of microcontrollers

In order for this tool to work, the CBUS pins must be configured as
GPIO. This configuration can be achieved by using the tool at
[ft232_cbus_config](https://github.com/trabucayre/ft232_cbus_config)
which configures all *CBUS* pins as GPIOs. This configuration, updates internal EEPROM
and needs only be performed once on a given FTDI chip.

## compile

This application uses **libftdi1** and **libudev-dev**, so these libraries must be installed (and,
depending of the distribution, headers too)
```bash
apt-get install libftdi1-2 libftdi1-dev libudev-dev
```
and if not already done, install **pkg-config**, **make** and **gcc**.

To build the app:
```bash
$ make
```

## udev
By default, users have no access to converters. A rule file
(*99-ftdi_cpu_progr.rules*) for *udev* is provided at the root directory
of this repository. These rules set access right and group (*plugdev*)
when a converter is plugged.

```bash
$ sudo cp 99-ftdi_cpu_prog.rules /etc/udev/rules.d/
$ sudo udevadm control --reload-rules && udevadm trigger # force udev to take new rule
$ sudo usermod -a YourUserName -G plugdev # add user to plugdev group
```
After that you need to unplug and replug your device.

## Usage

```bash
./ftdi_cpu_prog -b [0-3] [-bd [0-1]] -r [0-3] [-rd [0-1]] -m [0-1] -d device
        -b pin connected to BOOT
        -bd default state for BOOT
        -r pin connected to reset
        -rd default state for reset
        -m mode 0: bootloader, 1: simple reset
```

For `-b` and `-r` args, value must match indexes for CBUSx connected to `BOOT` and `RESET`
