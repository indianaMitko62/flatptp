#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "flatptp.h"
#include <stdint.h>

size_t send_bytes(int8_t *buf, size_t buf_size)
{
    print_encoded_frame(buf, buf_size);
    return buf_size;
}

void receive_byte(int8_t *buf)
{
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
        hdlc_decode_eat(decoder, c);
        if (hdlc_decode_has_complete_frame(decoder))
        {
            print_decoded_frame_ctx(decoder);
            sleep(1);
        }
        sleep(0.2);
    }
    return 0;
}