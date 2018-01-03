#include "../include/Packet.h"

void send_packet(SSL* ssl, Packet_Type type, void* data, size_t data_len)
{
    if (data_len == 0 || data == nullptr) return;
    
    unsigned int content_length = data_len;
    std::mt19937 gen(std::random_device{}());
    unsigned int padding_length = gen() % 1024;
    unsigned char* padding = new unsigned char[padding_length];
    for (unsigned int i = 0; i < padding_length; i++)
    {
        padding[i] = (unsigned char)(gen() % 256);
    }
    unsigned char* digest = new unsigned char[SHA256_DIGEST_LENGTH];
    if (sha_digest(data, data_len, digest) == FAILED)
    {
        throw PacketIOException("Unable to compute SHA256 of the packet content");
    }

    unsigned int output_len = 
              sizeof(type)                              // packet_type
            + sizeof(unsigned int)                      // payload_length
            + SHA256_DIGEST_LENGTH * sizeof(char)       // sha256_verification[]
            + sizeof(unsigned int)                      // content_length
            + sizeof(unsigned int)                      // padding_length
            + content_length                            // content
            + padding_length;                           // padding

    unsigned int payload_length = 
            sizeof(unsigned int)                        // content_length
            + sizeof(unsigned int)                      // padding_length
            + content_length                            // content
            + padding_length;                           // padding

    unsigned char* output = new unsigned char[output_len];
    unsigned char* cpo = output;
    memcpy(cpo, &type, sizeof(type));
    cpo += sizeof(type);
    memcpy(cpo, &payload_length, sizeof(unsigned int));
    cpo += sizeof(unsigned int);
    memcpy(cpo, digest, sizeof(unsigned char) * SHA256_DIGEST_LENGTH);
    cpo += SHA256_DIGEST_LENGTH * sizeof(unsigned char);
    memcpy(cpo, &content_length, sizeof(unsigned int));
    cpo += sizeof(unsigned int);
    memcpy(cpo, &padding_length, sizeof(unsigned int));
    cpo += sizeof(unsigned int);
    memcpy(cpo, data, content_length);
    cpo += content_length;
    memcpy(cpo, padding, padding_length);
    cpo += padding_length;

    int result = SSL_write(ssl, output, output_len);
    if (result > 0)
    {
        if (result != output_len)
        {
            throw PacketIOException("Unable to sent the whole packet. Sent only [%d] bytes", result);
        }
    }
    else if (result == 0)
    {
        int shutdown_status = SSL_get_shutdown(ssl);
        if ((shutdown_status & SSL_RECEIVED_SHUTDOWN) != 0)
        {
            SSL_shutdown(ssl);
            throw ShutdownException("Server requested shutdown");
        }
        else if ((shutdown_status & SSL_SENT_SHUTDOWN) != 0)
        {
            throw ShutdownException("Shutdown requested from client");
        }
    }
    else if (result < 0)
    {
        int error = SSL_get_error(ssl, result);
        throw PacketIOException("Packet write error %d", error);
    }
    #ifdef DEBUG_MODE
        printf("Sending a packet:\n\tType:%u\n\tPayload Size:%u\n\tPadding:%u\n\tContent:%u\n\tData:%s\n",
            type, payload_length, padding_length, content_length, (char*)(data));
        printf("\tSHA digest: ");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            printf("%02X ", digest[i]);
        }
        printf("\n");
    #endif
}

void recv_packet(SSL* ssl, SSH_Packet* read_to)
{
    int result;
    result = SSL_read(ssl, &read_to->packet_type, sizeof(read_to->packet_type));
    if (result != sizeof(read_to->packet_type))
    {
        throw PacketIOException("Unable to read full packet type");
    }

    result = SSL_read(ssl, &read_to->payload_length, sizeof(read_to->payload_length));
    if (result != sizeof(read_to->payload_length))
    {
        throw PacketIOException("Unable to read full payload length");
    }

    result = SSL_read(ssl, read_to->sha256_verification, sizeof(char) * SHA256_DIGEST_LENGTH);
    if (result != SHA256_DIGEST_LENGTH * sizeof(char))
    {
        throw PacketIOException("Unable to read full sha256");
    }

    result = SSL_read(ssl, &read_to->payload.content_length, sizeof(read_to->payload.content_length));
    if (result != sizeof(read_to->payload.content_length))
    {
        throw PacketIOException("Unable to read full content length");
    } 

    result = SSL_read(ssl, &read_to->payload.padding_length, sizeof(read_to->payload.padding_length));
    if (result != sizeof(read_to->payload.padding_length))
    {
        throw PacketIOException("Unable to read full padding length");
    }   

    read_to->payload.content = new unsigned char[read_to->payload.content_length + 1]();
    if (read_to->payload.content == nullptr)
    {
        throw PacketIOException("Unable to allocate content space");
    } 
    result = SSL_read(ssl, read_to->payload.content, read_to->payload.content_length);
    if (result != read_to->payload.content_length)
    {
        throw PacketIOException("Unable to read full content");
    }

    read_to->payload.padding = new unsigned char[read_to->payload.padding_length + 1]();
    if (read_to->payload.padding == nullptr)
    {
        throw PacketIOException("Unable to allocate padding space");
    } 
    result = SSL_read(ssl, read_to->payload.padding, read_to->payload.padding_length);
    if (result != read_to->payload.padding_length)
    {
        throw PacketIOException("Unable to read full padding");
    }  

    unsigned char* actual_digest = new unsigned char[SHA256_DIGEST_LENGTH];
    sha_digest(read_to->payload.content, read_to->payload.content_length, actual_digest);

    for (unsigned int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if (actual_digest[i] != read_to->sha256_verification[i])
        {
            throw PacketIOException("Invalid SHA256 digest");
        }
    }
    #ifdef DEBUG_MODE
        printf("Recieved a packet:\n\tType:%u\n\tPayload Size:%u\n\tPadding:%u\n\tContent:%u\n\tData:%s\n",
                read_to->packet_type, 
                read_to->payload_length,
                read_to->payload.padding_length, 
                read_to->payload.content_length, 
                (char*)(read_to->payload.content));
        printf("\tSHA digest: ");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            printf("%02X ", read_to->sha256_verification[i]);
        }
        printf("\n");
    #endif           
}


int sha_digest(void* msg, size_t msg_len, void* output)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    if (SHA256_Init(&sha256) == 0) return -1;
    if (SHA256_Update(&sha256, msg, msg_len) == 0) return -1;
    if (SHA256_Final(hash, &sha256) == 0) return -1;
    memcpy(output, hash, SHA256_DIGEST_LENGTH * sizeof(unsigned char));
    return 0;
}