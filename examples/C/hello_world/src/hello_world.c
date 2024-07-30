#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "flatptp.h"
#include <stdint.h>

size_t send_bytes(int8_t *buf, size_t buf_size)
{
    print_frame(buf, buf_size);
    return buf_size;
}

size_t receive_bytes(int8_t *buf)
{
    return 0;
}

int main()
{
    uint8_t data[] = "Hello World";
    uint8_t *send_buf = malloc(9 + sizeof(data) * 2);
    uint8_t address = 0x02;
    for (int i = 0; i < 10; i++)
    {
        int n = 0;
        n = hdlc_encode_data(address, data, sizeof(data), send_buf);
        if (send_bytes(send_buf, n) != n)
        {
            printf("Could not send %d bytes", n);
            return 1;
        }
        printf("Sent bytes count: %d\n\n\n", n);
        // sleep_ms(1000);
    }
    return 0;
}