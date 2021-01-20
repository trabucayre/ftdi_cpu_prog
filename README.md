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

