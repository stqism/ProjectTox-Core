/* network.h
 *
 * Datatypes, functions and includes for the core networking.
 *
 *  Copyright (C) 2013 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef WIN32 /* Put win32 includes here */
//Windows XP
#define WINVER 0x0501
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#undef VANILLA_NACL /* make sure on windows we use libsodium */

#else //Linux includes

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#endif

#ifndef VANILLA_NACL
/* we use libsodium by default */
#include <sodium.h>
#else
#include <crypto_box.h>
#define crypto_box_MACBYTES (crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_UDP_PACKET_SIZE 65507


/* Current time, unix format */
#define unix_time() ((uint64_t)time(NULL))


typedef union {
    uint8_t c[4];
    uint16_t s[2];
    uint32_t i;
} IP;

typedef struct {
    IP ip;
    uint16_t port;
    /* not used for anything right now */
    uint16_t padding;
} IP_Port;

typedef struct {
    int16_t family;
    uint16_t port;
    IP ip;
    uint8_t zeroes[8];
#ifdef ENABLE_IPV6
    uint8_t zeroes2[12];
#endif
} ADDR;

/* returns current time in milleseconds since the epoch. */
uint64_t current_time(void);

/* return a random number
    NOTE: this function should probably not be used where cryptographic randomness is absolutely necessary */
uint32_t random_int(void);

/* Basic network functions: */

/* Function to send packet(data) of length length to ip_port */
int sendpacket(IP_Port ip_port, uint8_t *data, uint32_t length);

/* Function to receive data, ip and port of sender is put into ip_port
    the packet data into data
    the packet length into length. */
int receivepacket(IP_Port *ip_port, uint8_t *data, uint32_t *length);

/* initialize networking
    bind to ip and port
    ip must be in network order EX: 127.0.0.1 = (7F000001)
    port is in host byte order (this means don't worry about it)
    returns 0 if no problems
    returns -1 if there were problems */
int init_networking(IP ip, uint16_t port);

/* function to cleanup networking stuff(doesn't do much right now) */
void shutdown_networking(void);

/*
  resolve_addr():
    address should represent IPv4 or a hostname with A record

    returns a data in network byte order that can be used to set IP.i or IP_Port.ip.i
    returns 0 on failure

    TODO: Fix ipv6 support
*/
uint32_t resolve_addr(const char *address);

#ifdef __cplusplus
}
#endif

#endif
