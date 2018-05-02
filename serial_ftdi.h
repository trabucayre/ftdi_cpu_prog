#ifndef SERIAL_FTDI_H
#define SERIAL_FTDI_H

#include <ftdi.h>

#define CBUS_DIR    0xF0
#define RST         0x02
#define BOOT0       0x01

//#define RST         0x04
//#define BOOT0       0x02


typedef struct serial serial_t;

//void find_ftdi_and_open(serial_t *h, int devnum);
//void serial_set_cbus(const serial_t *h, int cbus);
serial_t* serial_open(const char *device, 
		int reset, int def_reset, 
		int dload, int def_dload);
void serial_close(serial_t *h);
int serial_reset(const serial_t *serial, int dtr);

#endif /* SERIAL_FTDI_H */
