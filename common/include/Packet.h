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

#define FAILED              -1
#define MAX_LOGIN_LENGTH    32
#define MAX_PASSWORD_LENGTH 100
#define DEBUG_MODE

/**
 * \brief Type of the sent or recieved packet. All the packets must have a Packet_Type.
 * The reciever must take proper actions according to the type
 */
enum Packet_Type
{
    PACKET_QUERY,           ///< Complex command from the Client. Server must parse and execute
    PACKET_RESPONSE,        ///< A repsponse either from the Client or from the Server
    PACKET_REQUEST,         ///< A request from the Client or from the Server
    PACKET_AUTH_REQUEST,    ///< Authentication request from the Server. Client must provide credentials
    PACKET_AUTH_RESPONSE,   ///< Authentication response from the Client. Server must check credentials
    PACKET_READY            ///< Notification from the Server that it is ready for another complex command
};

/**
 * \brief The application level packet that gets send back and forth
 * between the client and the server. All the fields and mandatory for completion
 */
struct SSH_Packet
{
    Packet_Type         packet_type;  ///< Type of the packet
    unsigned int        payload_length;  ///< Length of SSH_Payload member
    unsigned char       sha256_verification[SHA256_DIGEST_LENGTH]; ///< Computed SHA256 on the payload.content buffer
    struct SSH_Payload
    {
        unsigned int            content_length; ///< Length of the content buffer
        unsigned int            padding_length; ///< Length of the padding buffer
        unsigned char*          content; ///< The actual data that the packet transports
        unsigned char*          padding; ///< Security padding with random length
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

/**
 * \brief Structure that goes into payload.content field.
 * Designed to ease the credentials transmission
 */
struct Login_Payload
{
    unsigned int login_length;
    unsigned int password_length;
    char login[MAX_LOGIN_LENGTH + 1] = {0};
    char password[MAX_PASSWORD_LENGTH + 1] = {0};
};

/**
 * \brief Sends and SSH_Packet to the SSL socket given
 * 
 * \param ssl Pointer to initialized and ready for transmission SSL structure
 * \param type Type of the packet to be sent
 * \param data A pointer to data buffer
 * \param data_len The length of the data buffer
 */
void send_packet(SSL* ssl, Packet_Type type, void* data, size_t data_len);
void recv_packet(SSL* ssl, SSH_Packet* read_to);

int sha_digest(void* msg, size_t msg_len, void* output);

#endif // PACKET_H