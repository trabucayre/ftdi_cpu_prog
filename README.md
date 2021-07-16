# ftdi_cpu_prog
tool to automate the programming of microcontrollers

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
        -b pin connected to CBUS
        -bd default state for BOOT
        -r pin connected to reset
        -rd default state for reset
        -m mode 0: bootloader, 1: simple reset
```

For `-b` and `-r` index is CBUSx
