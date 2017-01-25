#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/if.h>

/**
 * This program opens a CAN socket and keeps reading imcoming CAN frames through the socket.
 */

int main(int argc, char *argv[])
{
    /**
     * Like TCP/IP, you first need to open a socket for communicating over a
     * CAN network. Since SocketCAN implements a new protocol family, you
     * need to pass PF_CAN as the first argument to the socket(2) system
     * call. Currently, there are two CAN protocols to choose from, the raw
     * socket protocol and the broadcast manager (BCM). So to open a socket.
     *
     * In this example, we create a raw CAN socket.
     */
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    /**
     * The sockaddr_can structure has an interface index like the
     * PF_PACKET socket, that also binds to a specific interface:
     *
     *   struct sockaddr_can {
     *       sa_family_t can_family;
     *       int         can_ifindex;
     *       union {
     *           // transport protocol class address info (e.g. ISOTP)
     *           struct { canid_t rx_id, tx_id; } tp;
     *
     *           // reserved for future CAN protocols address information
     *       } can_addr;
     *   }
     */
    /* resolve interface index by name */
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");
    ioctl(s, SIOCGIFINDEX, &ifr); // SIOCGIFINDEX is name -> if_index mapping. See /usr/include/linux/sockios.h.
    
	struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(s, (struct sockaddr *) &addr, sizeof(addr));

    /**
     * The basic CAN frame structure and the sockaddr structure are defined
     * in include/linux/can.h:
     * 
     * struct can_frame {
     *     canid_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
     *     __u8    can_dlc; // frame payload length in byte (0 .. 8)
     *     __u8    __pad;   // padding
     *     __u8    __res0;  // reserved / padding
     *     __u8    __res1;  // reserved / padding
     *     __u8    data[8] __attribute__((aligned(8)));
     * };
     */
    struct can_frame frame;
    
    while (1) {
        /**
         * Reading CAN frames from a bound CAN_RAW socket consists of
         * reading a struct can_frame.
         */
        int nbytes = read(s, &frame, sizeof(struct can_frame));
        if (nbytes > 0) {
            printf("result=%d id=%d data(%d)=[", nbytes, frame.can_id, frame.can_dlc);
            int i;
            char *comma = "";
            for (i = 0; i < frame.can_dlc; ++i) {
                printf("%s%02x", comma, frame.data[i]);
                comma = ",";
            }
            printf("]\n");
        }
    }
}
