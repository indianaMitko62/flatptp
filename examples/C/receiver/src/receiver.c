#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "flatptp.h"
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#define TEST_MSG_CNT 10

int8_t receive_byte(int8_t *buf, int fd)
{
    size_t bytes_read = read(fd, buf, 1);
    if (0 >= bytes_read)
    {
        printf("could not read from port\n");
        return 2;
    }
    return 0;
}

int main()
{
    char buf[256];
    int8_t c;
    hdlc_decode_ctx_t decoder = hdlc_decode_start(buf, sizeof(buf) - 1); // -1 to allow adding terminating zeros for easy printing
    int fd = open("/dev/ttyACM0", O_RDONLY);
    if (0 > fd)
    {
        printf("could not open port\n");
        return 1;
    }
    uint32_t successful_messages = 0;
    for (int i = 0; i < TEST_MSG_CNT; i++)
    {
        while (true)
        {
            int err = receive_byte(&c, fd);
            if (err)
            {
                printf("Error receiving byte. Error code: %d\n", err);
                continue;
            }
            int res = hdlc_decode_eat(&decoder, c);
            if (res > 0)
            {
                printf("message received: '");
                for (int i = 0; i < res; i++)
                {
                    printf("%c", decoder.data[i]);
                }
                printf("'\n");
                successful_messages++;
                break;
            }
            if (ERR_INVALID_FRAME == res)
                break;
            if (INFO_BYTE_EATHEN != res)
            {
                printf("Error eating byte 0x%02X\t%c: %d\n\n\n", c, c, res);
                continue;
            }
        }
    }
    printf("\n\n\nTransmitted messages: %d\tSuccessfully received messages: %d\t%f%% packet loss\n", TEST_MSG_CNT, successful_messages, (1 - (float)successful_messages / TEST_MSG_CNT) * 100);
    close(fd);
    return 0;
}