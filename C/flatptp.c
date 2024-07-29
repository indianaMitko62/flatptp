#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>
#include "flatptp.h"
#include <stdio.h>

#define FLAG 0x7E
#define ESCAPE 0x7D
#define XOR_VALUE 0x20

#define CRC_START_VAL 0xFFFF
#define CRC_POLY 0x1021

uint16_t crc16_ccitt(int8_t *data, size_t length)
{
    uint16_t crc = CRC_START_VAL;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ CRC_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

size_t hdlc_encode_info_frame(int8_t *frame, uint8_t address, hdlc_encode_ctl_t *ctl, const int8_t *data, size_t data_size)
{
    uint32_t frame_index = 0;

    frame[frame_index++] = FLAG;
    frame[frame_index++] = address;
    frame[frame_index++] = (ctl->receive_sequence_number << 5) | (ctl->poll_flag_bit << 4) | (ctl->send_sequence_number << 1);
    ctl->send_sequence_number = (ctl->send_sequence_number + 1) % 8;

    for (int i = 0; i < data_size; i++)
    {
        if (data[i] == FLAG || data[i] == ESCAPE)
        {
            frame[frame_index++] = ESCAPE;
            frame[frame_index++] = data[i] ^ XOR_VALUE;
            continue;
        }
        frame[frame_index++] = data[i];
    }

    uint16_t crc = crc16_ccitt(frame + 1, frame_index - 1); // FIXME
    frame[frame_index++] = crc >> 8;
    frame[frame_index++] = crc & 0x00FF;
    frame[frame_index++] = FLAG;

    return frame_index;
}

size_t hdlc_encode_data(uint8_t address, int8_t *data, size_t data_size, int8_t *frame_buf)
{
    static hdlc_encode_ctl_t ctl = {0, 0, 1};
    size_t frame_size = hdlc_encode_info_frame(frame_buf, address, &ctl, data, data_size);
    if (frame_size == 0)
    {
        printf("error encoding frame");
        return 1;
    }
    return frame_size;
}

void print_frame(int8_t *frame, size_t buf_size)
{
    int i;
    printf("Flag:\t0x%02X\n", frame[0]);
    printf("Addr:\t0x%02X\n", frame[1]);
    printf("Ctrl:\t0x%02X\n", frame[2]);
    printf("\tRecN:\t%d\n", (frame[2] >> 5) & 0x07);
    printf("\tP/Fb:\t%d\n", (frame[2] >> 4) & 0x01);
    printf("\tSenN:\t%d\n", (frame[2] >> 1) & 0x07);
    printf("\tType:\t%d\n", frame[2] & 0x01);
    printf("Data:\n");
    for (i = 3; i < buf_size - 3; i++)
    {
        printf("0x%02X, ", frame[i]);
    }
    printf("\nEnd of Data\n");
    printf("FCS:\t0x%02X%02X\n", (uint8_t)frame[i], (uint8_t)frame[i + 1]);
    printf("Flag:\t0x%X\n\n", frame[buf_size - 1]);
}
