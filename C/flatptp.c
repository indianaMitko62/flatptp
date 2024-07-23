#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>
#include "flatptp.h"
#include <stdio.h>

#define FLAG 0x7E
#define ESCAPE 0x7D
#define XOR_VALUE 0x20

size_t hdlc_encode_info_frame(int8_t **frame, uint8_t address, hdlc_encode_ctl_t *ctl, const int8_t *data, size_t data_size)
{
    // static uint8_t receive_sequence_number = 0;
    // static uint8_t send_sequence_number = 0;

    size_t frame_size = 4; // for first flag, last flag, address field and control field
    uint32_t frame_index = 0;

    for (int i = 0; i < data_size; i++)
    {
        if (data[i] == FLAG || data[i] == ESCAPE)
            frame_size++;
        frame_size++;
    }

    // uint8_t frame_arr[frame_size];
    *frame = malloc(frame_size);
    if (*frame == NULL)
    {
        printf("unsuccessful malloc");
        return 0;
    }

    (*frame)[frame_index++] = FLAG;
    (*frame)[frame_index++] = address;
    (*frame)[frame_index++] = (ctl->receive_sequence_number << 5) | (ctl->poll_flag_bit << 4) | (ctl->send_sequence_number << 1);
    ctl->send_sequence_number = (ctl->send_sequence_number + 1) % 8;

    for (int i = 0; i < data_size; i++)
    {
        if (data[i] == FLAG || data[i] == ESCAPE)
        {
            (*frame)[frame_index++] = ESCAPE;
            (*frame)[frame_index++] = data[i] ^ XOR_VALUE;
            continue;
        }
        (*frame)[frame_index++] = data[i];
    }

    (*frame)[frame_index++] = FLAG;

    return frame_size;
}

size_t hdlc_send_data(uint8_t address, int8_t *data, size_t data_size, size_t (*send_bytes)(int8_t *buf, size_t buf_size))
{
    static hdlc_encode_ctl_t ctl = {5, 6, 1};
    int8_t *frame;
    size_t frame_size = hdlc_encode_info_frame(&frame, address, &ctl, data, data_size);
    if (frame_size == 0)
    {
        printf("error encoding frame");
        return 1;
    }
    size_t sent_size = send_bytes(frame, frame_size);
    if (sent_size < frame_size)
    {
        printf("could not send whole frame");
        return 1;
    }
    free(frame);
}

void print_frame(int8_t *frame, size_t buf_size)
{
    printf("Flag:\t0x%x\n", frame[0]);
    printf("Addr:\t0x%x\n", frame[1]);
    printf("Ctrl:\t0x%x\n", frame[2]);
    printf("\tRecN:\t%d\n", (frame[2] >> 5) & 0x07);
    printf("\tP/Fb:\t%d\n", (frame[2] >> 4) & 0x01);
    printf("\tSenN:\t%d\n", (frame[2] >> 1) & 0x07);
    printf("\tType:\t%d\n", frame[2] & 0x01);
    printf("Data:\n");
    for (int i = 3; i < buf_size - 1; i++)
    {
        printf("0x%x, ", frame[i]);
    }
    printf("\nEnd of Data\n");
    printf("Flag:\t0x%x\n", frame[buf_size - 1]);
}