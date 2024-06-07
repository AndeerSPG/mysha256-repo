/**
* @file s_my_sha256header.h
* @author   Gonzalo Alvarez - Dpto. ATC/KAT - UPV-EHU
* @date     05/02/2024
* @brief    Include file with struct c_header_sha256
* @details  A header file(.h) with the data structure definition 
*           (c_header_sha256). This file will be used to create a  
*           "special file" that will store sha256 hash codes of a 
*           set of files.
*  
*                "SPECIAL  FILE"
*           +++++++++++++++++++++++++++++
*           + Header Files Record   0   +
*           +---------------------------+
*           +      Data File 0          +
*           +    0... N-1 bytes         +
*           +   (data of File 0)        +
*           +++++++++++++++++++++++++++++
*           + Header Files Record   1   +
*           +---------------------------+
*           +      Data File 1          +
*           +    0... N-1 bytes         +
*           +   (data of File 1)        +
*           +++++++++++++++++++++++++++++
*           + Header Files Record   2   +
*           +---------------------------+
*           +      Data File 1          +
*           +    0... N-1 bytes         +
*           +   (data of File 2)        +
*           +++++++++++++++++++++++++++++
*           +           ...             +
*           +++++++++++++++++++++++++++++
*           + Header Files Record   K-1 +
*           +---------------------------+
*           +      Data File K-1        +
*           +    0... N-1 bytes         +
*           +   (data of File K-1)      +
*           +++++++++++++++++++++++++++++
*
*/
#include <stdlib.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>



#define OK (0)
#define ERROR_WRONG_NUMBER_ARGUMENTS (1)
#define ERROR_GENERATE_SHA256_HEADER (2)
#define ERROR_OPEN_DAT_FILE (3)
#define ERROR_OPEN_SHA_REPO_FILE (4)
#define ERROR_OTHER_1 (5)
#define ERROR_OTHER_2 (6)

#define FILE_HEADER_SIZE     512
#define READ_BLOCK_SIZE     (16 * 1024)                     // 16 KBytes

// Return error Codes
#define HEADER_OK (1)
#define HEADER_ERR (2)

#define HEX_SHA256_HASH_SIZE  (SHA256_DIGEST_LENGTH*2 +1 )  // 65 Bytes


// Source OpenSSL Cryptography and SSL/TLS Toolkit
//  https://www.openssl.org/docs/manmaster/man3/SHA256_Init.html
//  SHA256_CTX are Deprecated functions. ( new EVP_Digest)

#define DATA_VALID_SIZE ( 256 + sizeof(off_t) + HEX_SHA256_HASH_SIZE )    
#define UNUSED_DATA_SIZE ( FILE_HEADER_SIZE - DATA_VALID_SIZE)   

struct c_sha256header {
        char fname[256];                   // file name
        off_t size;                        // similar to a 32-bit integer
        char hash[HEX_SHA256_HASH_SIZE];   // hash code of file fname  (hexadecimal string)
        
        mode_t permissions; // Permisos del archivo
        //...
        // to complete in subsequent versions of the project
        char unused[UNUSED_DATA_SIZE];    
};

/**
* end @file s_my_sha256header.h
**/
