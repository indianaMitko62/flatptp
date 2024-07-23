#include <stdint.h>

// *********************
// HDLC encoding
// *********************

typedef struct
{
    uint8_t receive_sequence_number;
    uint8_t send_sequence_number;
    uint8_t poll_flag_bit;
} hdlc_encode_ctl_t;

// hdlc_send_data encodes the data stored in data as a HDLC frame and sends the frame using the user provided send_bytes function
size_t hdlc_send_data(uint8_t address, int8_t *data, size_t data_size, size_t (*send_bytes)(int8_t *buf, size_t buf_size));
void print_frame(int8_t *frame, size_t buf_size);

// *********************
// HDLC decoding
// *********************
typedef struct
{
    char *buf;
    uint16_t max_size;
    // and other stuff
} hdlc_decode_ctx_t;

void hdlc_decode_start(hdlc_decode_ctx_t *ctx, char *buf, uint16_t max_size);
void hdlc_decode_eat(hdlc_decode_ctx_t *ctx, char c);

// if the previous eat() call ate the last byte of a complete frame,
// this function should return the size of the frame (and buf should
// contain the decoded frame content)
uint16_t hdlc_decode_has_complete_frame(hdlc_decode_ctx_t *ctx);
uint16_t hdlc_decode_has_error(hdlc_decode_ctx_t *ctx);