//https://www.willhaley.com/blog/debian-arm-qemu/

/*  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <math.h>

#include "configurator.h"

int devmem(int argc, char **argv) {     // devmem2 to write in mem
    int fd;
    void *map_base, *virt_addr; 
	unsigned long read_result, writeval;
	off_t target;
	int access_type = 'w';
	
	if(argc < 2) {
		fprintf(stderr, "\nUsage:\t%s { address } [ type [ data ] ]\n"
			"\taddress : memory address to act upon\n"
			"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n"
			"\tdata    : data to be written\n\n",
			argv[0]);
		exit(1);
	}
	target = strtoul(argv[1], 0, 0);

	if(argc > 2)
		access_type = tolower(argv[2][0]);


    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    printf("/dev/mem opened.\n"); 
    fflush(stdout);
    
    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;
    printf("Memory mapped at address %p.\n", map_base); 
    fflush(stdout);
    
    virt_addr = map_base + (target & MAP_MASK);
    switch(access_type) {
		case 'b':
			read_result = *((unsigned char *) virt_addr);
			break;
		case 'h':
			read_result = *((unsigned short *) virt_addr);
			break;
		case 'w':
			read_result = *((unsigned long *) virt_addr);
			break;
		case 'i':
			read_result = *((unsigned int *) virt_addr);
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			exit(2);
	}
    printf("Value at address 0x%X (%p): 0x%X\n", target, virt_addr, read_result); 
    fflush(stdout);

	if(argc > 3) {
		writeval = strtoul(argv[3], 0, 0);
		switch(access_type) {
			case 'b':
				*((unsigned char *) virt_addr) = writeval;
				read_result = *((unsigned char *) virt_addr);
				break;
			case 'h':
				*((unsigned short *) virt_addr) = writeval;
				read_result = *((unsigned short *) virt_addr);
				break;
			case 'w':
				*((unsigned long *) virt_addr) = writeval;
				read_result = *((unsigned long *) virt_addr);
				break;
			case 'i':
				*((unsigned int *) virt_addr) = writeval;
				read_result = *((unsigned int *) virt_addr);
				break;
		}
		printf("Written 0x%X; readback 0x%X\n", writeval, read_result); 
		fflush(stdout);
	}
	
	if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}

int write_mem(unsigned long addr, char *data_type, char *write_data){   // Func to write in memory by using devmem2
    
    char address[10];
    char middle[10];

    for (int i = 0; i < 4; i++){
        
        snprintf(middle, sizeof(middle), "%lx", addr+4*i);

        strcpy(address, "0x");
        strcat(address, middle);

        char *args[] = {"devmem", address, data_type, write_data}; 
        int argc = sizeof(args) / sizeof(args[0]);
        devmem(argc, args);
        printf("\n");
    }

    return 0;
}

unsigned long dec_to_bin(int dec_num, unsigned long **bin_num[], int arr_length){    // Translate decimal to binary
    unsigned long *bin_arr;
    unsigned long *bin_reversed = (unsigned long *)malloc(arr_length * sizeof(unsigned long));
    int i, j = 0;

    for (i = 0; dec_num > 0; i++){
        bin_reversed[i] = dec_num % 2;
        dec_num /= 2;
    }

    while (i < arr_length){
        i++;
        bin_reversed[i] = 0;
    }

    bin_arr = (unsigned long *)malloc(arr_length * sizeof(unsigned long));

    for (i = i - 1; i >= 0; i--){
        bin_arr[i] = bin_reversed[j];
        j++;
    }

    *bin_num = bin_arr;

    return 0;
}

int array_connection(unsigned long *conc_arr, unsigned long *arg_arr, int length, int *i){
    int j = *i, c = 0;

    for (j = *i; j < length; j++){
        conc_arr[j] = arg_arr[c];
        c++;
    }

    *i = j;

    return 0;
}

unsigned long bin_to_hex(unsigned long *fin_arr, char *write_data){     // Translate binary to hexadecimal
    unsigned long remainder = 0, hex_num = 0;
    int i = 1;

    for (int j = CONC_BIT_LEN-1; j >= 0; j--){
        remainder = fin_arr[j];
        hex_num = hex_num + remainder * i;
        i *= 2;
    }

    char middle[10];

    snprintf(middle, sizeof(middle), "%lx", hex_num);

    strcpy(write_data, "0x");
    strcat(write_data, middle);

    return 0;
}

unsigned long count_increment(float frequency, int freq_units){     // Counting increment from frequency
    unsigned long increment;
    int units;

    switch (freq_units){
        case 1:
            units = pow(10, 3);  // kHz
            break;

        case 2:
            units = pow(10, 6);  // MHz
            break;

        default:
            units = 1;           // Hz
            break;
    }

    increment = (frequency * units * pow(2, BITS))/F_CLK;

    return increment;
} // MAX 12.5 MHz

int count_phase(int phase){         // Counting phase in percents to dec
    int phase_dec;

    phase_dec = (phase << 12) >> 8;  // Our max is 4096, that equal to 100%(270°)

    return phase_dec;
}

int count_amplitude(int amplitude){     // Counting amplitude in percents 

    amplitude = (amplitude << 8) >> 7;  // Our max is 255 equal to 100%

    return amplitude;
}  

int start_conf(){   // Starting Function

    char *pow_args[] = {"devmem", POW_CONF_ADDR, "w", POW_CONF_DATA};
    char *on_off_args[] = {"devmem", ON_OFF_ADDR, "w", ON_OFF_DATA}; 
    
    int argc = sizeof(pow_args) / sizeof(pow_args[0]);
    
    devmem(argc, pow_args);

    devmem(argc, on_off_args);
}

void reading(){     // Function for reading from map addresses
    
    int read_num = 1;
    
    char read_address[10];
    char middle[10];

    printf("Enter address to read from 1 to 12: \n");
    scanf("%d", &read_num);
    
    if (read_num > 12 || read_num <= -1){
        printf("\n\nERROR: Choose address from 1 to 12 (read address set to 1)\n\n");
        read_num = 1;
    }

    snprintf(middle, sizeof(middle), "%lx", REG_MAP_ADDR+8*(read_num-1));

    strcpy(read_address, "0x");
    strcat(read_address, middle);

    char *read_args[] = {"devmem", read_address};
    devmem(2, read_args);
    
}

int main(){

    unsigned long increment;
    int amplitude, phase;
    float frequency;
    int freq_units = 0, i = 0;
    char *data_type = "w";
    char *write_data[10];

    unsigned long *arr_amp, *arr_phase, *arr_incr, *arr_fin;

    arr_fin = (unsigned long *)malloc(CONC_BIT_LEN * sizeof(unsigned long));

    int addr_cnt = 0;

    printf("\n\n======STARTING_CONFIGURATION_OF_SIXTEEN_CHANNELS======\n\n");

    while (addr_cnt != 4){

        printf("======CONFIG_OF_%d_FOUR_CHANNELS======\n\n", addr_cnt+1);

        printf("Enter amplitude (Max 100): ");
        scanf("%d", &amplitude);   // In ???

        if (amplitude > 100 || amplitude <= 0) {
            fprintf(stderr, "\nERROR: Amplitude needs to be <= 100 and > 0\n");        
            exit(2);
        }   

        printf("Enter phase (Max 270°): ");
        scanf("%d", &phase);   // In degrees

        if (phase > 270 || phase <= 0) {
            fprintf(stderr, "\nERROR: Phase needs to be <= 270 and > 0\n");        
            exit(4);
        }

        printf("Enter frequency(Max 12.5 MHz) and  Units(Hz = 0, kHz = 1, MHz = 2): ");
        scanf("%f %d", &frequency, &freq_units);    // In Hz, kHz, MHz

        if (frequency > 12.5 && freq_units == 2 || frequency > 12500 && freq_units == 1 || frequency > 12500000 && freq_units == 0 || frequency <= 0){
            fprintf(stderr, "\nERROR: Frequency need to be <= then 12.5 MHz (12500 kHz, 12 500 000) and > 0\n");        
            exit(5);
        }

        increment = count_increment(frequency, freq_units);

        phase = count_phase(phase);

        amplitude = count_amplitude(amplitude);

        dec_to_bin(amplitude, &arr_amp, AMP_BIT_LEN);

        dec_to_bin(phase, &arr_phase, PH_INCR_BIT_LEN);

        dec_to_bin(increment, &arr_incr, PH_INCR_BIT_LEN);

        
        array_connection(arr_fin, arr_amp, AMP_BIT_LEN, &i);

        array_connection(arr_fin, arr_phase, AMP_BIT_LEN+PH_INCR_BIT_LEN, &i);

        array_connection(arr_fin, arr_incr, CONC_BIT_LEN, &i);

        bin_to_hex(arr_fin, &write_data);

        write_mem(START_ADDR+addr_cnt*16, data_type, write_data);

        addr_cnt += 1;
    }
    
    printf("\n======STARTING======\n\n");
    start_conf();

    while(1){
        reading();
    }

    return 0;
}
