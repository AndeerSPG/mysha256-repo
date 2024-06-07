/* *
 * * @file mysha256-repo.c
 * * @author G.A.
 * * @date 6/02/2024
 * * @brief First version of mysha256-repo
 * * @details  Create a tar file with only one "data file"
 * *
 * */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "s_my_sha256header.h"

extern int sha256sum_file(char *path, char outputBuffer[]); // HEX_SHA256_HASH_SIZE Bytes
extern void sha256_hash_to_string(unsigned char *hash, unsigned char *outputBuffer);
extern int insertar(char *f_mysha256_Repo, char *f_dat);
extern int extraer_fichero(char *f_mysha256_Repo, char *f_dat);

#define OK (0)

#define ERROR_GENERATE_SHA_REPO_FILE (4)
#define ERROR_GENERATE_SHA_REPO_FILE2 (5)

#define HEADER_OK (1)
#define HEADER_ERR (2)
#define SHA256_GEN_ERR (3)

#define INVALID_PARAMETER (6)

#define ERROR_INSERTING_FILE (7)
#define ERROR_EXTRACTING_FILE (8)

/**
    Escribe el encabezado del archivo
*/
int Buildsha256RepoHeader(char *FileName, struct c_sha256header *psha256header)
{
    char calc_hash[HEX_SHA256_HASH_SIZE];
    int Return_Code;

    // Fill all struct psha256header members

    bzero(psha256header, sizeof(struct c_sha256header)); // Fill all struct data with zeros

    strcpy(psha256header->fname, FileName); // File name

    psha256header->size = 0;

    // Call to function (sha256_file) to generate sha256 sum of file data
    Return_Code = sha256sum_file(FileName, calc_hash);

    if (Return_Code != 0)
    {
        fprintf(stderr, "🚨 ERROR: Error on hash function call (err=%d) \n", Return_Code);
        return SHA256_GEN_ERR;
    }

    strcpy(psha256header->hash, calc_hash); // fill  sha256 hash psha256header of argv[1]
                                            // in psha256header
    return HEADER_OK;
}

unsigned long GetBytesInFile(int fd_DataFile)
{
    char FileDataBlock[READ_BLOCK_SIZE];
    unsigned long NumWriteBytes = 0;
    long bytesRead;

    while ((bytesRead = read(fd_DataFile, FileDataBlock, READ_BLOCK_SIZE)) != 0)
    {
        NumWriteBytes += bytesRead;
    }

    lseek(fd_DataFile, 0, SEEK_SET);

    return NumWriteBytes;
}

/**
    Escribe el contenido del archivo en bloques de tamaño READ_BLOCK_SIZE.
    Rellena el resto del bloque con 0s.
*/
unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_RepoFile)
{
    char FileDataBlock[READ_BLOCK_SIZE];
    unsigned long NumWriteBytes;
    unsigned long bytesRead, bytesWrite;

    // write the data file (blocks of READ_BLOCK_SIZE  bytes) (4KB)
    NumWriteBytes = 0;
    bzero(FileDataBlock, sizeof(FileDataBlock));

    while ((bytesRead = read(fd_DataFile, FileDataBlock, READ_BLOCK_SIZE)) != 0)
    {
        if ((bytesWrite = write(fd_RepoFile, FileDataBlock, READ_BLOCK_SIZE)) == -1)
        {
            perror("(mysha256-repo.c > WriteFileDataBlocks) Ha ocurrido un error al escribir en el archivo repositorio");
            return NumWriteBytes;
        }
        NumWriteBytes += bytesRead;
    }

    if (bytesRead < 0)
    {
        perror("(mysha256-repo.c > WriteFileDataBlocks) Ha ocurrido un error al leer el bloque de datos");
        return NumWriteBytes;
    }

    printf("\n Total : %ld  bytes writen \n", NumWriteBytes); // Traze
    return NumWriteBytes;
}

int main(int argc, char *argv[])
{
    char action[1];         // Acción: 'I'=Insertar, 'E'=Extraer
    char FileName[256];     // Nombre del archivo dat (origen)
    char RepoFileName[256]; // Nombre del archivo repo (destino)

    int fd_DatFile, fd_RepoFile;
    int num_files_in_repo;

    // Comprobación de que llegan todos los argumentos
    if (argc != 4)
    {
        fprintf(stderr, "🚨 ERROR: El comando debe seguir la siguiente estructura: %s [acción (I/E)] [nombre del archivo] [nombre del repositorio]\n", argv[0]);
        return 1;
    }

    strcpy(action, argv[1]);
    strcpy(FileName, argv[2]);
    strcpy(RepoFileName, argv[3]);

    // Si la acción es 'I', inserta fichero
    if (strcmp(action, "I") == 0)
    {
        num_files_in_repo = insertar(RepoFileName, FileName);
        if (num_files_in_repo < 0)
        {
            fprintf(stderr, "\n❌ FICHERO NO INSERTADO DEBIDO A UN ERROR\n");
            return ERROR_INSERTING_FILE;
        }
        printf("\nArchivo %s insertado con éxito en el repositorio %s 🥳\nNúmero de archivos en el repositorio: %d\n\n", FileName, RepoFileName, num_files_in_repo);
    }
    else if (strcmp(action, "E") == 0)
    {
        if (extraer_fichero(RepoFileName, FileName) < 0)
        {
            fprintf(stderr, "\n❌ FICHERO NO EXTRAÍDO DEBIDO A UN ERROR\n");
            return ERROR_EXTRACTING_FILE;
        }
        printf("\nArchivo %s del repositorio %s extraído con éxito 🥳\n\n", FileName, RepoFileName);
    }
    else
    {
        fprintf(stderr, "🚨 ERROR: Invalid value for parameter [action]; Expected 'I' or 'E' but %s given\n", action);
        return INVALID_PARAMETER;
    }

    return OK;
}