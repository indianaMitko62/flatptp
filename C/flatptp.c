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
        if (FLAG == data[i] || ESCAPE == data[i])
        {
            frame[frame_index++] = ESCAPE;
            frame[frame_index++] = data[i] ^ XOR_VALUE;
            continue;
        }
        frame[frame_index++] = data[i];
    }

    uint16_t crc = crc16_ccitt(frame + 1, frame_index - 1); //+/-1 is to avoid adding the entry flag to FCS calculation
    frame[frame_index++] = crc >> 8;
    frame[frame_index++] = crc & 0x00FF;
    frame[frame_index++] = FLAG;

    return frame_index;
}

size_t hdlc_encode_data(uint8_t address, int8_t *data, size_t data_size, int8_t *frame_buf)
{
    static hdlc_encode_ctl_t ctl = {0, 0, 1};
    size_t frame_size = hdlc_encode_info_frame(frame_buf, address, &ctl, data, data_size);
    if (0 == frame_size)
    {
        printf("error encoding frame");
        return 1;
    }
    return frame_size;
}

void hdlc_decode_start(hdlc_decode_ctx_t *ctx, int8_t *data, uint16_t max_size)
{
    ctx->data = data;
    ctx->buf_max_size = max_size;
    ctx->data_index = 0;
    ctx->in_frame = 0;
}

void hdlc_decode_eat(hdlc_decode_ctx_t *ctx, int8_t b)
{
    if (FLAG == b)
    {
        ctx->in_frame = !ctx->in_frame;
        return;
    }
    else
    {
        if (ctx->in_frame)
        {
            switch (ctx->data_index)
            {
            case 0:
                ctx->address = b;
                break;
            case 1:
                ctx->ctl->receive_sequence_number = b & 0xE0;
                ctx->ctl->poll_flag_bit = b & 0x10;
                ctx->ctl->send_sequence_number = b & 0x0E;
                ctx->ctl->type = b & 0x01;
                break;
            default:
                ctx->data[ctx->data_index++] = b;
                break;
            }
        }
    }
    if (ctx->data_index + 2 == ctx->buf_max_size)
    {
        return; // return error for not enough size for data
    }
}

uint16_t hdlc_decode_has_complete_frame(hdlc_decode_ctx_t *ctx)
{
    if (ctx->in_frame)
    {
        return 0;
    }
    ctx->crc = ctx->data[ctx->data_index - 1] << 8 | ctx->data[ctx->data_index - 2];
    if (ctx->crc == crc16_ccitt(ctx->data, ctx->data_index - 2))
    {
        return ctx->data_index - 2; // removing CRC from data buffer.
    }
    else
    {
        return 0; // return wrong FCS
    }
}

void print_encoded_frame(int8_t *frame, size_t frame_size)
{
    int i;
    printf("Flag:\t0x%02X\n", frame[0]);
    printf("Addr:\t0x%02X\n", frame[1]);
    printf("Ctrl:\t0x%02X\n", frame[2]);
    printf("\tRecN:\t%d\n", frame[2] & 0xE0);
    printf("\tP/Fb:\t%d\n", frame[2] & 0x10);
    printf("\tSenN:\t%d\n", frame[2] & 0x0E);
    printf("\tType:\t%d\n", frame[2] & 0x01);
    printf("Data:\n");
    for (i = 3; i < frame_size - 3; i++)
    {
        printf("0x%02X, ", frame[i]);
    }
    printf("\nEnd of Data\n");
    printf("FCS:\t0x%02X%02X\n", (uint8_t)frame[i], (uint8_t)frame[i + 1]);
    printf("Flag:\t0x%X\n\n", frame[frame_size - 1]);
}

void print_decoded_frame_ctx(hdlc_decode_ctx_t *ctx)
{
    printf("Addr:\t0x%02X\n", ctx->address);
    printf("Ctrl:\t0x%02X\n", ctx->ctl);
    printf("\tRecN:\t%d\n", ctx->ctl->receive_sequence_number);
    printf("\tP/Fb:\t%d\n", ctx->ctl->poll_flag_bit);
    printf("\tSenN:\t%d\n", ctx->ctl->send_sequence_number);
    printf("\tType:\t%d\n", ctx->ctl->type);
    printf("Data:\n");
    for (int i = 0; i < ctx->data_index - 2; i++)
    {
        printf("0x%02X, ", ctx->data[i]);
    }
    printf("\nEnd of Data\n");
    printf("FCS:\t0x%02X%02X\n", ctx->crc);
}
