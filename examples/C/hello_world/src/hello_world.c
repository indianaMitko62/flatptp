#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "flatptp.h"
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#define MILLISECONDS(val) val * 1000

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        errx(1, "Missing command line argument.\nUsage: <filepath>\n");
    }
    uint8_t data[] = "Hello World";
    uint8_t *send_buf = malloc(9 + sizeof(data) * 2);
    uint8_t address = 0x02;
    int fd = open(argv[1], O_WRONLY);
    if (fd < 0)
    {
        err(2, "Error opening");
    }
    for (int i = 0; i < 100; i++)
    {
        int n = 0;
        n = hdlc_encode_data(address, data, sizeof(data), send_buf);
        if (write(fd, send_buf, n) != n)
        {
            errx(2, "Could not send %d bytes\n", n);
        }
        printf("Sent bytes count: %d\n", n);
        print_encoded_frame(send_buf, n);
        usleep(MILLISECONDS(500));
    }
    close(fd);
    return 0;
}
