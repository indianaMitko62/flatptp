#include <stdint.h>

#define ERR_ENCODING_FRAME -1
#define ERR_BUFFER_OVERFLOWING -2
#define ERR_INVALID_FRAME -3

#define INFO_BYTE_EATEN -4

// *********************
// FLATPTP encoding
// *********************
typedef struct
{
    uint8_t receive_sequence_number;
    uint8_t send_sequence_number;
    uint8_t poll_flag_bit;
    uint8_t type;
} hdlc_encode_ctl_t;

// flatptp_encode_data encodes the data stored in data as a HDLC frame
ssize_t flatptp_encode_data(uint8_t address, int8_t *data, size_t data_size, int8_t *frame_buf);

// *********************
// FLATPTP ecoding
// *********************
typedef struct
{
    int8_t *buf;
    size_t buf_max_size;
    size_t buf_index;
    size_t msg_length;

    uint8_t address;
    hdlc_encode_ctl_t ctl;
    int8_t *data;
    uint16_t frame_crc;
} flatptp_decode_ctx_t;

void flatptp_decode_start(flatptp_decode_ctx_t *ctx, int8_t *buf, uint16_t max_size);

// if the eat() call eats the last byte of a complete frame,
// it should return the size of the frame (and data field should
// contain the decoded frame content)
ssize_t flatptp_decode_eat(flatptp_decode_ctx_t *ctx, int8_t c);

// *********************
//  FLATPTP help
// *********************
void print_encoded_frame(int8_t *frame, size_t buf_size);
void print_decoded_frame_ctx(flatptp_decode_ctx_t *ctx);
