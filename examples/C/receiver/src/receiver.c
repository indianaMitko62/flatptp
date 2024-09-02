#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include "flatptp.h"

#define TEST_MSG_CNT 10

int set_interface_attribs(int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
            err(2, "error %d from tcgetattr", errno);
            return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK; // disable break processing
    tty.c_lflag = 0;        // no signaling chars, no echo,
                            // no canonical processing
    tty.c_oflag = 0;        // no remapping, no delays
    tty.c_cc[VMIN] = 0;     // read doesn't block
    tty.c_cc[VTIME] = 5;    // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);   // ignore modem controls,
                                       // enable reading
    tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        err(2, "error %d from tcsetattr", errno);
    }
    return 0;
}

void set_blocking(int fd, int should_block)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        err(2, "error %d from tggetattr", errno);
    }

    tty.c_cc[VMIN] = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
            err(2, "error %d setting term attributes", errno);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        errx(1, "Missing command line argument.\nUsage: <filepath>\n");
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        err(2, "Error openning");
    }
    set_interface_attribs(fd, B115200, 0); // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking(fd, 0);                   // set no blocking

    flatptp_decode_ctx_t decoder;
    int8_t buf[256];
    flatptp_decode_start(&decoder, buf, sizeof(buf));
    int8_t c;
    uint32_t successful_messages = 0;
    for (int i = 0; i < TEST_MSG_CNT; i++)
    {
        while (1)
        {
            int res = read(fd, &c, 1);
            if (res < 0)
            {
                errx(3, "Error reading");
            }
            res = flatptp_decode_eat(&decoder, c);
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
            {
                for (int i = 0; i < decoder.msg_length; i++)
                {
                    printf("%#04x, ", decoder.buf[i]);
                }
                printf("\n");
                break;
            }
            if (INFO_BYTE_EATEN != res)
            {
                warn("Error eating byte 0x%02X\t%c: %d\n\n\n", c, c, res);
            }
        }
    }
    printf("\n\n\nTransmitted messages: %d\tSuccessfully received messages: %d\t%f%% packet loss\n", TEST_MSG_CNT, successful_messages, (1 - (float)successful_messages / TEST_MSG_CNT) * 100);
    close(fd);
    return 0;
}
