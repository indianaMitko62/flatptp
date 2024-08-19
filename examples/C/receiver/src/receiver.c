#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "flatptp.h"
#include <stdint.h>

void receive_byte(int8_t *buf)
{
    int fd = open("/dev/ttyACM0");
    if (0 > fd)
    {
        printf("could not open port");
        return;
    }
    size_t bytes_read = read(fd, buf, 1);
    close(fd);
    return;
}

int main()
{
    char buf[256];
    int8_t c;
    hdlc_decode_ctx_t *decoder;
    hdlc_decode_start(decoder, buf, sizeof(buf) - 1); // -1 to allow adding terminating zeros for easy printing
    while (true)
    {
        receive_byte(&c);
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
        sleep(0.2);
    }
    return 0;
}