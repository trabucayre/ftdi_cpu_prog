#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <assert.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libudev.h>
#include <libusb.h>
#include <ftdi.h>
#include <string.h>

#include "serial_ftdi.h"

#undef DEBUG
#undef USE_DEVNUM

struct serial {
	struct ftdi_context *ftdi;
	int vid;
	int pid;
	char serial[64]; // ggm : must be resized correctly
	int fd;
	int interface;
	int reset;
	int def_reset;
	int dload;
	int def_dload;
};

void serial_flush(const serial_t *h) {
	ftdi_usb_purge_tx_buffer(h->ftdi); 
	ftdi_usb_purge_rx_buffer(h->ftdi); 
}

unsigned int udevstufftoint(const char *udevstring, int base)
{
	char *endp;
	int ret;
	errno = 0;

	if (udevstring == NULL)
		return (-1);

	ret = (unsigned int)strtol(udevstring, &endp, base);
	if (errno) {
		fprintf(stderr,
			"udevstufftoint: Unable to parse number Error : %s (%d)\n",
			strerror(errno), errno);
		return (-2);
	}
	if (endp == optarg) {
		fprintf(stderr, "udevstufftoint: No digits were found\n");
		return (-3);
	}
	return (ret);
}

#ifdef USE_DEVNUM
void find_ftdi_and_open(serial_t *h, int devnum)
{
    //int ret;
    struct ftdi_device_list *devlist, *curdev;
	
	/*ret = */ftdi_usb_find_all(h->ftdi, &devlist, h->vid, h->pid);
	/* test needed */
	/* ... */

	for (curdev = devlist; curdev != NULL; curdev = curdev->next) {
		// test needed
		if (libusb_get_device_address(curdev->dev) == devnum)
			break;
	}

	if (curdev == NULL)
		h->fd = -1;
	else 
		h->fd = ftdi_usb_open_dev(h->ftdi, curdev->dev);
	
	ftdi_list_free(&devlist);
}
#endif

void serial_set_cbus(const serial_t *h, int cbus) {
	ftdi_set_bitmode(h->ftdi, cbus, BITMODE_CBUS);
}

serial_t* serial_open(const char *device, int reset, int def_reset,
			int dload, int def_dload) 
{
	serial_t *h = calloc(sizeof(serial_t), 1);
	struct udev *udev;
	struct udev_device *dev, *usbdeviceparent;
	char devtype;

	struct stat statinfo;
	if (stat(device, &statinfo) < 0) {
		printf("unable to stat file\n");
		return NULL;
	}

	/* get device type */
	switch (statinfo.st_mode & S_IFMT) {
	case S_IFBLK:
		devtype = 'b';
		break;
	case S_IFCHR:
		devtype = 'c';
		break;
	default:
		printf("not char or block device\n");
		return NULL;
	}
#ifdef DEBUG
	printf("Using %s (UID=%ld GID=%ld perm=%lo)  %d:%d\n",
	       device, (long)statinfo.st_uid, (long)statinfo.st_gid,
	       (unsigned long)statinfo.st_mode,
	       major(statinfo.st_rdev), minor(statinfo.st_rdev));
#endif

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		return NULL;
	}

	dev = udev_device_new_from_devnum(udev, devtype, statinfo.st_rdev);

	if (dev == NULL) {
		printf("no dev\n");
		udev_unref(udev);
		exit(EXIT_FAILURE);
	}

	/* Get closest usb device parent (we need VIP/PID)  */
	usbdeviceparent =
	    udev_device_get_parent_with_subsystem_devtype(dev, "usb",
							  "usb_device");
	if (!usbdeviceparent) {
		printf
		    ("Unable to find parent usb device! Is this actually an USB device ?\n");
		udev_device_unref(dev);
		udev_unref(udev);
		return NULL;
	}

	h->vid =
	    udevstufftoint(udev_device_get_sysattr_value
			   (usbdeviceparent, "idVendor"), 16);
	h->pid =
	    udevstufftoint(udev_device_get_sysattr_value
			   (usbdeviceparent, "idProduct"), 16);
#ifdef USE_DEVNUM
	int devnum =
	    udevstufftoint(udev_device_get_sysattr_value
			   (usbdeviceparent, "devnum"), 10);
#else
	sprintf(h->serial, "%s", udev_device_get_sysattr_value(usbdeviceparent, "serial"));
	printf("%s\n", h->serial);
#endif
#ifdef DEBUG
	printf("%04x %04x %04x\n", h->vid, h->pid, devnum);
#endif

	udev_device_unref(dev);
	udev_unref(udev);

	/* libftdi init */
	h->interface = INTERFACE_A;//NY;

	h->ftdi = ftdi_new();
	if (h->ftdi == 0) {
		fprintf(stderr, "ftdi_new failed\n");
		return NULL;
	}
	ftdi_set_interface(h->ftdi, h->interface);
#ifdef USE_DEVNUM
	find_ftdi_and_open(h, devnum);
#else
	h->fd = ftdi_usb_open_desc(h->ftdi, h->vid, h->pid, NULL, h->serial);
#endif
	/* h->fd = ftdi_usb_open(h->ftdi, h->vid, h->pid);*/
	if (h->fd < 0) {
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", h->fd, 
						ftdi_get_error_string(h->ftdi));
		return NULL;
	}
	h->ftdi->usb_write_timeout = 10000;
	h->ftdi->usb_read_timeout = 10000;
#ifdef DEBUG
    if (h->ftdi->type == TYPE_R)
    {
		unsigned int chipid;
		printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(h->ftdi, &chipid));
		printf("FTDI chipid: %X\n", chipid);
	}
#endif
	h->reset = reset;
	h->def_reset = def_reset;
	h->dload = dload;
	h->def_dload = def_dload;
	
	return h;
}

/* cf. ftdi.c same function */
static void ftdi_usb_close_internal (struct ftdi_context *ftdi)
{
	libusb_close (ftdi->usb_dev);
	ftdi->usb_dev = NULL;
}

void serial_close(serial_t *h) {
	assert(h && h->ftdi !=NULL);
	struct ftdi_context *ftdi = h->ftdi;
	int rtn;

	//serial_set_cbus(h, CBUS_DIR);

	serial_flush(h);

	/*ftdi_usb_close(h->ftdi);
	 * repompe de la fonction et des suivantes
	 */
	 if (ftdi->usb_dev != NULL) {
		rtn = libusb_release_interface(ftdi->usb_dev, ftdi->interface);
		if (rtn < 0) {
			printf("release interface failed %d\n", rtn);
			return;
		}
		if (ftdi->module_detach_mode == AUTO_DETACH_SIO_MODULE) {
			rtn = libusb_attach_kernel_driver(ftdi->usb_dev, ftdi->interface);
			if( rtn != 0)
				printf("detach error %d\n",rtn);
		}
	}
	ftdi_usb_close_internal(ftdi);

	ftdi_free(ftdi);
	free(h);
}

void ftdi_send_reset(const serial_t *serial, int cbus)
{
	// 1 : reset 0
	if (serial->def_reset == 1)
		cbus &= ~serial->reset;
	else
		cbus |= serial->reset;
	serial_set_cbus(serial, cbus);
	usleep(70000);//100);
	usleep(24000);
	// 2 : reset 1
	if (serial->def_reset == 1)
		cbus |= serial->reset;
	else
		cbus &= ~serial->reset;
	serial_set_cbus(serial, cbus);
	usleep(70000);//100);
}

void ftdi_bootloader(const serial_t *serial, int cbus)
{
#if 0
    /* erase high, reset high*/
	cbus = CBUS_DIR|serial->dload|serial->reset;
	serial_set_cbus(serial, cbus);
    usleep(45000);
    usleep(18000);
    /* reset sequence */
    cbus = CBUS_DIR|serial->dload;
	serial_set_cbus(serial, cbus);
    usleep(70000);
    usleep(24000);
    //usleep(30000);
	cbus = CBUS_DIR|serial->dload|serial->reset;
	serial_set_cbus(serial, cbus);
	//ftdi_send_reset(serial, cbus);
	
    usleep(45000);
    usleep(18000);
    /* erase low */
	cbus = CBUS_DIR|serial->reset;
	serial_set_cbus(serial, cbus);
    usleep(45000);
    usleep(18000);

#else

	// 1 : gestion de BOOT0 CBUS1
	if (serial->def_dload == 1)
		cbus &= ~serial->dload;
	else
		cbus |= serial->dload;
	serial_set_cbus(serial, cbus);
	//usleep(70000);
    usleep(45000);
    usleep(18000);
	// 2 : reset 0
	ftdi_send_reset(serial, cbus);
	//usleep(600000);
    usleep(45000);
    usleep(18000);
	// 3 : BOOT0 0
	if (serial->def_dload == 1)
		cbus |= serial->dload;
	else
		cbus &= ~serial->dload;
	serial_set_cbus(serial, cbus);
	usleep(600000);
#endif
}

int serial_reset(const serial_t *serial, int dtr)
{
	int cbus = CBUS_DIR;
	if (serial->def_reset == 1)
		cbus |= serial->reset;
	if (serial->def_dload == 1)
		cbus |= serial->dload;
	if (dtr == 0)
		ftdi_bootloader(serial, cbus);
	else
		ftdi_send_reset(serial, cbus);

	return 0;
}
