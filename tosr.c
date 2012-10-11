/* Linux TOSR-0n controller */
/* (c) 2012 Héctor Cordobés */
/* Based on the basic serial examples at tldp.org */

/* You may use this sw in any commercial and non-comercial project */
/* The sw is provided as is with no guarantee of any kind in any scenario */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* baudrate settings are defined in <asm/termbits.h>, which is
   included by <termios.h> */
#define BAUDRATE B9600
/* change this definition for the correct port */
#define DEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */


enum {
    SW_VERSION='Z',
    RELAY_STATE='[',
    DC_INPUT=']',
    ENABLE_ALL='d',
    ENABLE_1,
    ENABLE_2,
    ENABLE_3,
    ENABLE_4,
    ENABLE_5,
    ENABLE_6,
    ENABLE_7,
    ENABLE_8,
    DISABLE_ALL='n',
    DISABLE_1,
    DISABLE_2,
    DISABLE_3,
    DISABLE_4,
    DISABLE_5,
    DISABLE_6,
    DISABLE_7,
    DISABLE_8,
    ENABLE_RELAYS,
    DISABLE_RELAYS
} commands;

void usage(char** argv) {
    printf("TOSR-0n controller by Héctor Cordobés.\n");
    printf("Usage: %s [options]\n\n", argv[0]);
    printf("Available options are:\n");
    printf("  -t <device>          Serial device to use\n");
    printf("  -e <relay numbers>   Enables relay(s)\n");
    printf("  -E                   Enables all relays\n");
    printf("  -d <relay numbers>   Disables relay(s)\n");
    printf("  -D                   Disables all relays\n");
    printf("  -S                   Returns relay state\n");
    printf("  -s <relaynumber>     Returns shell code 0 if relay is active\n");
    printf("  -i                   Returns TOSR SW version\n");
    printf("  -v                   Returns DC input voltage\n");
    printf("  -h                   Shows this help message\n\n");
    printf("Besides the device name, only one option can be used at a time.\n");
    printf("If a device is not specified, /dev/ttyUSB0 will be used.\n");
}

int send_command(int fd, int command) {
    int res;
    char code = command;
    res = write(fd, &code, 1);
    return(res==1);
}

int read_response(int fd, unsigned char* buf, int length) {
    int res;
        res = read(fd, (void*)buf, length);
    if (res != length)
        printf("Read error: requested %d bytes, read %d\n", length, res);
    return(res==length);
}

int parse_relays(char* arg, unsigned char* relays) {
    *relays=0;
    char* optchar = arg;
    while(*optchar!=0 && *optchar!=' ') {
        if(*optchar<'1' || *optchar>'8') {
            printf("Error: '%c' is not a valid value for a relay.\nAdmitted values for relays are [1..8]\n", *optchar);
            return(0);
        }
        *relays |= 1<<(*optchar - '1');
        optchar++;
    }
    return(1);
}

int get_sw_version(int fd) {
    unsigned char buf[2];
    send_command(fd, SW_VERSION);
    read_response(fd, buf, 2);
    printf("Module ID: %d - Software version: %d\n", buf[0], buf[1]);
    return(1);
}

int set_relays(int fd, unsigned char relays, int action) {
    int count=0;
    if (relays == 0xFF) {
        if (action==ENABLE_RELAYS)
            send_command(fd, ENABLE_ALL);
        else
            send_command(fd, DISABLE_ALL);
    } else {
        while(relays) {
            if (relays%2) {
                /* Compact, but dependant on the message format: wrong! */
                if (action==ENABLE_RELAYS)
                    send_command(fd, ENABLE_1 + count);
                else
                    send_command(fd, DISABLE_1 + count);
            }
            count++;
            relays>>=1;
        }
    }
    return(1);
}

int get_relay_state(int fd, unsigned char* state) {
    send_command(fd, RELAY_STATE);
    return(read_response(fd, state, 1));
}

void print_relay_state(unsigned char state) {
    int count;
    printf("Relay state (little endian): '");
    for (count=7; count>=0; count--) {
        if(state&(1<<count))
            printf("1");
        else
            printf("0");
    }
    printf("' (%x)\n", state);
}

int get_dc_input(int fd) {
    unsigned char buf;
    send_command(fd, DC_INPUT);
    read_response(fd, &buf, 1);
    printf("DC (in dV): %d\n", buf);
    return(0);
}

int main(int argc, char** argv)
{
    /* Serial vars */
    int fd;
    struct termios oldtio,newtio;

    int opt;
    int action = 0;
    unsigned char relays = 0;

    char default_device[] = DEVICE;
    char* device = default_device;

    /* We need to be told what to do... */
    if (argc<2) {
        usage(argv);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "t:e:Ed:Ds:Sivh")) != -1) {
        if (action!=0 && opt!='t') {
            usage(argv);
            exit(EXIT_FAILURE);
        }
        switch (opt) {
            case 't':
                device = optarg;
                break;
            case 'e':
                action = ENABLE_RELAYS;
                if(!parse_relays(optarg, &relays)) {
		  exit(EXIT_FAILURE);
		}
                break;
            case 'd':
                action = DISABLE_RELAYS;
                if(!parse_relays(optarg, &relays)) {
		  exit(EXIT_FAILURE);
		}
                break;
            case 'E':
                action = ENABLE_RELAYS;
                relays = 0xFF;
                break;
            case 'D':
                action = DISABLE_RELAYS;
                relays = 0xFF;
                break;
            case 's':
                action = RELAY_STATE;
		parse_relays(optarg, &relays);
		if((relays&(relays-1))!=0) { /* Is relays a power of two? */
		  printf("Per-relay state is allowed only for one relay.\n");
		  exit(1);
		}
                break;
            case 'S':
                action = RELAY_STATE;
                break;
            case 'v':
                action = DC_INPUT;
                break;
            case 'i':
                action = SW_VERSION;
                break;
            case 'h':
                usage(argv);
                exit(0);
                break; /* what the...? */
            default: /* '?' */
                usage(argv);
                exit(EXIT_FAILURE);
        }
    }


    /* Open modem device for reading and writing and not as controlling tty
       because we don't want to get killed if linenoise sends CTRL-C.  */
    fd = open(device, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(device); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current serial port settings */
    memset(&newtio, 0, sizeof(newtio)); /* clear struct for new port settings */

    /* BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters                  */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

    /* IGNPAR  : ignore bytes with parity errors              */
    newtio.c_iflag = IGNPAR;

    /* Raw output.                                            */
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1; /* inter-character timer 100ms */
    newtio.c_cc[VMIN] = 2; /* blocking read until 2 chars received
                              (longest response */

    /* clean the modem line and activate the settings for the port */
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    unsigned char state;
    int return_value = 0;

    switch(action) {
        case ENABLE_RELAYS:
        case DISABLE_RELAYS:
            set_relays(fd, relays, action);
            break;
        case RELAY_STATE:
            get_relay_state(fd, &state);
	    if (relays!=0) {
	      if ((state&relays)==0)
		return_value=1;
	      printf("Relays %u State %u return %u\n", relays, state, state&relays);
	    }
	    else
	      print_relay_state(state);
            break;
        case SW_VERSION:
            get_sw_version(fd);
            break;
        case DC_INPUT:
            get_dc_input(fd);
            break;
        default:
            break;
            /* This should not happen */
    }

    /* restore the old port settings */
    tcsetattr(fd,TCSANOW,&oldtio);
    return(return_value);
}
