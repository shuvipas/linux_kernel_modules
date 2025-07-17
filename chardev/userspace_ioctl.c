/*
make
sudo insmod char_device.ko
sudo chmod 666 /dev/char_dev // without this get are error: Can't open device file
sudo dmesg| tail -10
gcc userspace_ioctl.c -o userspace_ioctl
./userspace_ioctl
*/
#include "./char_device.h"

#include <stdio.h> /* standard I/O */
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <stdlib.h> /* exit */
#include <sys/ioctl.h> /* ioctl */

int ioctl_set_msg(int file_desc, char* msg){
    int ret_val = ioctl(file_desc, IOCTL_SET_MSG, msg);
    if(ret_val<0){
        printf("ioctl_set_msg failed:%d\n", ret_val);
    }
    return ret_val;
}

int ioctl_get_msg(int file_desc){
    char msg[100]={0};
    int ret_val = ioctl(file_desc, IOCTL_GET_MSG, msg);
    if(ret_val<0){
        printf("ioctl_get_msg failed:%d\n", ret_val);
    }
    printf("get_msg message:%s", msg);
    return ret_val;
}

int ioctl_get_nth_byte(int file_desc){
    int i=0;
    int c=0;
    printf("get_nth_byte message: ");
    do{
        c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);
        if (c<0){
            printf("\nioctl_get_nth_byte failed at the %d'th byte: \n", i);
            return c;
        }
        putchar(c);
    }while(c);
    return 0;
}

int main(void){
    int ret_val;
    int file_desc = open(DEVICE_PATH, O_RDWR);
    char *msg = "Message passed by ioctl\n";
    if (file_desc < 0) {
        printf("Can't open device file: %s, error:%d\n", DEVICE_PATH,
        file_desc);
        exit(EXIT_FAILURE);
    }
    ret_val = ioctl_set_msg(file_desc, msg);
    if (ret_val) goto error;
    ret_val = ioctl_get_nth_byte(file_desc);
    if (ret_val) goto error;
    ret_val = ioctl_get_msg(file_desc);
    if (ret_val) goto error;
    close(file_desc);
    return 0;

    error:
        close(file_desc);
        exit(EXIT_FAILURE);
}