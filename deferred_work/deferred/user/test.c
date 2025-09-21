
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../include/deferred.h"

#define DEVICE_PATH	"/dev/deferred"

/* prints error message and exits */
void error(char *message)
{
	perror(message);
	exit(EXIT_FAILURE);
}


void print_options()
{
	printf("\n\n======= test options =======\n"
			"\ts <seconds> - set timer to run after <seconds> seconds\n"
			"\tc           - cancel timer\n"
			"\ta <seconds> - allocate memory after <seconds> seconds\n"
			"\tp <pid>     - monitor pid\n"
			"\tq		   - quit\n"
			"\n\n");
}
/* prints usage message and exits */
void usage()
{
	print_options();
	exit(1);

}

int main(int argc, char **argv)
{
    int fd;
    char buffer[256];
    char cmd;
    char *num_str;

    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0)
        error(DEVICE_PATH);

    while(1)
    {
        print_options();
        printf("Enter command and argument: ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break; /* End of file */
        }
        
        char *token = strtok(buffer, " \n");
        if (token == NULL) {
            continue;
        }
        cmd = token[0];
        
        num_str = strtok(NULL, " \n");

        switch (cmd) {
            case 's':
            case 'a':
            case 'p':
                if (num_str == NULL) {
                    usage();
                    continue;
                }
                unsigned long val = strtoul(num_str, NULL, 10);
                printf("Processing command '%c' with value %lu\n", cmd, val);
                
                if (cmd == 's') {
                    if (ioctl(fd, MY_IOCTL_TIMER_SET, val) < 0)
                        error("ioctl set timer error");
                } else if (cmd == 'a') {
                    if (ioctl(fd, MY_IOCTL_TIMER_ALLOC, val) < 0)
                        error("ioctl allocate memory error");
                } else if (cmd == 'p') {
                    if (ioctl(fd, MY_IOCTL_TIMER_MON, val) < 0)
                        error("ioctl monitor pid error");
                }
                break;
            case 'c':
                printf("Cancel timer\n");
                if (ioctl(fd, MY_IOCTL_TIMER_CANCEL) < 0)
                    error("ioctl cancel timer error");
                break;
            case 'q':
                printf("Exiting...\n");
                close(fd);
                return 0;
            default:
                printf("Wrong parameter.\n");
                usage();
        }
    }
    
    close(fd);
    return 0;
}