#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Source OpenSSL Cryptography and SSL/TLS Toolkit
//  https://www.openssl.org/docs/manmaster/man3/SHA256_Init.html
//  SHA256_CTX are Deprecated functions. ( new EVP_Digest)

#define READ_BLOCK_SIZE  (16 * 1024)                     // 16 KBytes
#define OUTPUT_BUFFER_SIZE  (SHA256_DIGEST_LENGTH*2 +1 )  // 65 Bytes

#define ERROR_OPEN_FILE (-1)
#define ERROR_READ_FILE (-2)
#define OK               (0)

// void sha256_hash_to_string (unsigned char hash[], unsigned char outputBuffer[]) 
void sha256_hash_to_string (unsigned char *hash, unsigned char *outputBuffer) 
{
    int i = 0;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]); // Convert each byte to hexadecimal string format (two bytes) 
    }

    outputBuffer[OUTPUT_BUFFER_SIZE-1] = 0;  // Add the final NULL char (string)
}


int sha256sum_file(char *path, char outputBuffer[])  // OUTPUT_BUFFER_SIZE Bytes 
{
    unsigned char hash[SHA256_DIGEST_LENGTH];  // 32 bytes
    unsigned char buffer[READ_BLOCK_SIZE];     // (16 * 1024) bytes
    int bytesRead = 0;
    int Return_Code;
    int fd;
    
    fd = open(path, O_RDONLY);
    if(fd == -1) return ERROR_OPEN_FILE;  // ERROR_OPEN_FILE
    
    SHA256_CTX sha256;      // Declare a SHA256_CTX structure
    SHA256_Init(&sha256);   // initializes SHA256_CTX structure
    
    while((bytesRead = read(fd, buffer, READ_BLOCK_SIZE)) >0)
    {
        SHA256_Update(&sha256, buffer, bytesRead);  // Called repeatedly with chunks  of the message (buffer) to be hashed 
    }
     // Error control  on last read:
    if(bytesRead < 0 ) Return_Code = ERROR_READ_FILE;  // ERROR_FREAD_FILE
    else Return_Code =  OK;
    close(fd);
    
    // End HSA256 
    SHA256_Final(hash, &sha256);   // Places the message digest in the variable hash and erase SHA256_CTX structure
    
    // Convert SHA256 hash code (32 bytes) to Hexadecimal Vector (string) (64 +1  bytes)
    sha256_hash_to_string(hash, outputBuffer);
 

    return Return_Code;
}