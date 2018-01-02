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
#define MAX_LOGIN_LENGTH 32
#define MAX_PASSWORD_LENGTH 100
#define DEBUG_MODE

enum Packet_Type
{
    PACKET_QUERY,
    PACKET_RESPONSE,
    PACKET_REQUEST,
    PACKET_AUTH_REQUEST,
    PACKET_AUTH_RESPONSE,
    PACKET_READY
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

    SSH_Packet()
    {
        this->payload.content = nullptr;
        this->payload.padding = nullptr;
    }

    ~SSH_Packet() 
    {
        if (this->payload.content) delete this->payload.content;
        if (this->payload.padding) delete this->payload.padding;
    }
};

struct Login_Payload
{
    unsigned int login_length;
    unsigned int password_length;
    char login[MAX_LOGIN_LENGTH + 1] = {0};
    char password[MAX_PASSWORD_LENGTH + 1] = {0};
};

void send_packet(SSL* ssl, Packet_Type type, void* data, size_t data_len);
void recv_packet(SSL* ssl, SSH_Packet* read_to);

int sha_digest(void* msg, size_t msg_len, void* output);

#endif // PACKET_H