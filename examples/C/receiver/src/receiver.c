#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "flatptp.h"
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

int8_t receive_byte(int8_t *buf)
{
    int fd = open("/dev/ttyACM0", O_RDONLY);
    if (0 > fd)
    {
        printf("could not open port\n");
        return 1;
    }
    size_t bytes_read = read(fd, buf, 1);
    if (0 >= bytes_read)
    {
        printf("could not read from port\n");
        return 2;
    }
    close(fd);
    return 0;
}

int main()
{
    char buf[256];
    int8_t c;
    hdlc_decode_ctx_t *decoder;
    hdlc_decode_start(decoder, buf, sizeof(buf) - 1); // -1 to allow adding terminating zeros for easy printing
    while (true)
    {
        // sleep(1);
        int err = receive_byte(&c);
        if (err)
        {
            printf("Error receiving byte. Error code: %d\n", err);
            continue;
        }
        int res = hdlc_decode_eat(decoder, c);
        if (res > 0)
        {
            printf("message received: ");
            for (int i = 0; i < res; i++)
            {
                printf("%c", decoder->data[i]);
            }
            printf("\n");
            continue;
        }
        if (res != INFO_BYTE_EATHEN)
        {
            printf("Error eating byte 0x%02X: %d", c, res);
        }
        printf("Sucessfully eaten 0x%02X: %d", c, res);
    }
    return 0;
}