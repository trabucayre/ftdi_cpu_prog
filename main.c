#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial_ftdi.h"

void print_usage(char *filename);
int get_check_params(int argc, char **argv, int *dload, int *def_dload,
						int *reset, int *def_reset,
						int *val, char *device);

int main(int argc, char **argv)
{
	serial_t *h;
	int reset, def_reset = 0, dload, def_dload = 0;
	int val;
	char device[64];
	bzero(device, 64);
	
	if (get_check_params(argc, argv, &dload, &def_dload, 
							&reset, &def_reset, &val,
							device) == EXIT_FAILURE)
		return EXIT_FAILURE;

	printf("%d reset %d %d dload %d %d dev %s\n", val, reset, def_reset,
				dload, def_dload, device);
	reset = 1 << reset;
	dload = 1 << dload;

	printf("%d reset %d %d dload %d %d dev %s\n", val, reset, def_reset,
				dload, def_dload, device);

	h = serial_open(device, reset, def_reset, dload, def_dload);
	if (h == NULL) {
		printf("erreur d'ouverture de %s\n", device);
		return EXIT_FAILURE;
	}

	/* 0 bootloader, 1 simple reset */
	serial_reset(h, val); 

	serial_close(h);

	return EXIT_SUCCESS;
}

int get_check_params(int argc, char **argv, int *dload, int *def_dload,
						int *reset, int *def_reset,
						int *val, char *device)
{
	int i, size_param, param_val;
	char param;

	if (argc == 1)
		goto error_params;

	for (i=1; i<argc; i+=2) {
		param = argv[i][1];
		param_val = atoi(argv[i+1]);
		size_param = strlen(argv[i]);
		switch (param) {
		case 'r':
			if (size_param > 1 && argv[i][2] == 'd')
				*def_reset = param_val;
			else
				*reset = param_val;
			break;
		case 'b':
			if (size_param > 1 && argv[i][2] == 'd')
				*def_dload = param_val;
			else
				*dload = param_val;
			break;
		case 'm':
			*val = param_val;
			break;
		case 'd':
			sprintf(device,"%s",argv[i+1]);
		}
	}

	/* check config */
	if (*reset < 0 || *reset > 3) {
		printf("erreur reset %d\n", *reset); 
		goto error_params;
	}
	if (*dload < 0 || *dload > 3) {
		printf("erreur dload %d\n", *dload); 
		goto error_params;
	}
	if (*val != 0 && *val != 1) {
		printf("erreur mode %d\n", *dload);
		goto error_params;
	}
	if (strlen(device) == 0) {
		printf("manque le device\n");
		goto error_params;
	}
	return EXIT_SUCCESS;

error_params:
	print_usage(argv[0]);
	return EXIT_FAILURE;
}

void print_usage(char *filename) 
{
	printf("%s -b [0-3] [-bd [0-1]] -r [0-3] [-rd [0-1]] -m [0-1] -d device\n",filename);
	printf("\t-b pin connected to CBUS\n");
	printf("\t-bd default state for BOOT\n");
	printf("\t-r pin connected to reset\n");
	printf("\t-rd default state for reset\n");
	printf("\t-m mode 0: bootloader, 1: simple reset\n");
}
