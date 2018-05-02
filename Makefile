CC=gcc
CFLAGS=$(shell pkg-config --libs libudev libftdi1 libusb-1.0) -g -Wall
CFLAGS+=$(shell pkg-config libudev libftdi1 libusb-1.0 --cflags)
DEST=ftdi_cpu_prog
SRC=main.c serial_ftdi.c

all: $(DEST)

$(DEST): Makefile $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(DEST) 
clean:
	@rm -rf $(DEST) *.o
