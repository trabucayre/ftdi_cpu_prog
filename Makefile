CC=gcc
CFLAGS=$(shell pkg-config libudev libftdi1 libusb-1.0 --cflags) -g -Wall
LDFLAGS=$(shell pkg-config --libs libudev libftdi1 libusb-1.0)
DEST=ftdi_cpu_prog
SRC=main.c serial_ftdi.c

LIBFTDI_VERS=$(shell pkg-config --modversion libftdi1)

ifeq ($(shell expr $(LIBFTDI_VERS) \>= 1.5), 1)
CFLAGS+=-DLIBFTDI_OLD=0
else
CFLAGS+=-DLIBFTDI_OLD=1
endif

all: $(DEST)

$(DEST): Makefile $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(DEST) $(LDFLAGS)
clean:
	@rm -rf $(DEST) *.o
