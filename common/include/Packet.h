#ifndef PACKET_H
#define PACKET_H

#include "Exceptions.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <random>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/sha.h"



#define FAILED -1

enum Packet_Type
{
    PACKET_QUERY,
    PACKET_RESPONSE,
    PACKET_REQUEST,
    PACKET_AUTH
};

struct SSH_Packet
{
    Packet_Type         packet_type;
    unsigned int        payload_length;
    unsigned char       sha256_verification[SHA256_DIGEST_LENGTH];
    struct SSH_Payload
    {
        unsigned int            content_length;
        unsigned int            padding_length;
        unsigned char*          content;
        unsigned char*          padding;
    } payload;
};

void send_packet(SSL* ssl, Packet_Type type, void* data, size_t data_len);
void recv_packet(SSL* ssl, SSH_Packet* read_to);

int sha_digest(void* msg, size_t msg_len, void* output);

#endif // PACKET_H