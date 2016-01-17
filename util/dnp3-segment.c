/*
 * DNP3 Segmenter
 *
 * This is a simple utility that reads a stream of bytes from stdin
 * and splits it into dnp3 segments, which are then send as UDP packets
 * to localhost, port 20000.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    unsigned char c;
    int state = 0;
    int frameLen;
    int frameBytesRecvd;
    char buf[292];
    int bufOffset;
    struct sockaddr_in dest;
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("127.0.0.1");
    dest.sin_port = htons(20000);

    buf[0] = 0x05;
    buf[1] = 0x64;

    for (;;) {
        if (read(0, &c, 1) != 1)
            return 0;

        if (state < 3 && c == 0x05) {
            state = 1;
        } else if (state == 1) {
            if (c == 0x64)
                state = 2;
            else
                state = 0;
        } else if (state == 2) {
            if (c > 5) {
                frameLen = c - 5;
                state = 3;
                frameBytesRecvd = 0;
                bufOffset = 3;
                buf[2] = c;
            } else {
                state = 0;
            }
        } else if (state == 3) {
            buf[bufOffset++] = c;
            frameBytesRecvd++;
            if (frameBytesRecvd == 5) {
                read(0, buf + bufOffset, 1);
                read(0, buf + bufOffset + 1, 1);
                bufOffset += 2;
                state = 4;
                frameBytesRecvd = 0;
            }
        } else if (state == 4) {
            buf[bufOffset++] = c;
            frameBytesRecvd++;

            if (frameBytesRecvd == frameLen || (frameBytesRecvd % 16) == 0) {
                read(0, buf + bufOffset, 1);
                read(0, buf + bufOffset + 1, 1);
                bufOffset += 2;
            }
            if (frameBytesRecvd == frameLen) {
                sendto(sock, buf, bufOffset, 0, (struct sockaddr *)&dest, sizeof(dest));
                bufOffset = 0;
                state = 0;
            }
        }
    }
}

