#include <stdint.h>

// *********************
// HDLC encoding
// *********************
typedef struct
{
    uint8_t receive_sequence_number;
    uint8_t send_sequence_number;
    uint8_t poll_flag_bit;
    uint8_t type;
} hdlc_encode_ctl_t;

// hdlc_encode_data encodes the data stored in data as a HDLC frame and sends the frame using the user provided send_bytes function
ssize_t hdlc_encode_data(uint8_t address, int8_t *data, size_t data_size, int8_t *frame_buf);

// *********************
// HDLC decoding
// *********************
typedef struct
{
    int8_t *buf;
    size_t buf_max_size;
    size_t buf_index;

    uint8_t address;
    hdlc_encode_ctl_t *ctl;
    int8_t *data;
    uint16_t frame_crc;
} hdlc_decode_ctx_t;

void hdlc_decode_start(hdlc_decode_ctx_t *ctx, int8_t *data, uint16_t max_size);

// if the eat() call eats the last byte of a complete frame,
// it should return the size of the frame (and data field should
// contain the decoded frame content)
ssize_t hdlc_decode_eat(hdlc_decode_ctx_t *ctx, int8_t c);

// *********************
// HDLC help
// *********************
void print_encoded_frame(int8_t *frame, size_t buf_size);
void print_decoded_frame_ctx(hdlc_decode_ctx_t *ctx);