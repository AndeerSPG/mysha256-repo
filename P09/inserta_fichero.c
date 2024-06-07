#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>

#include "s_my_sha256header.h"

#define E_OPEN1 (-1)
#define E_OPEN2 (-2)
#define E_REPOFORM (-3)
#define E_DESCO (-99)

extern int Buildsha256RepoHeader(char *FileName, struct c_sha256header *psha256header);
extern unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_RepoFile);
extern unsigned long GetBytesInFile(int fd_DataFile);

int insertar(char *f_mysha256_Repo, char *f_dat)
{
  int fd_repo; // Identificador archivo repo (destino)

  struct stat f_dat_stat; // Donde guardamos la información del archivo dat (origen)

  DIR *ddir;                // Donde guardamos la información del directorio en caso de que el archivo sea directorio
  struct dirent *archivo;   // Donde se guarda la información de un archivo dentro del directorio si es necesario
  char path_archivo[256];       // Donde guardaremos el string con la ruta del archivo concatenado con el nombre del archivo interior
  struct stat archivo_stat; // Donde guardamos la información del archivo dentro del directorio

  int respuesta_insertar; // Usamos esta variable para saber el estado de insertar un único archivo al repositorio

  int number_of_files_in_repo = 0; // Número de archivos contenidos en el archivo f_mysha256_Repo

  // Abrimos el archivo repo (destino), si no existe se crea por poner el O_CREAT
  if ((fd_repo = open(f_mysha256_Repo, O_CREAT | O_RDWR, 0644)) == -1)
  {
    fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Ha ocurrido un error al abrir el archivo \"%s\" (f_mysha256_Repo). ERROR: ", f_mysha256_Repo);
    perror("");
    return E_OPEN2;
  }

  // Cogemos la información del archivo dat pasado por parámetros
  if (stat(f_dat, &f_dat_stat) != 0)
  {
    fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Ha ocurrido un error al obtener la información del archivo \"%s\" (f_dat). ERROR: ", f_dat);
    perror("");
    close(fd_repo); // Cerramos el archivo repo (destino)
    return E_DESCO;
  }

  if (S_ISDIR(f_dat_stat.st_mode))
  {
    // Si es un directorio, abrimos el directorio e insertamos su contenido
    if ((ddir = opendir(f_dat)) == NULL)
    {
      fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Ha ocurrido un error al abrir el directorio \"%s\" \n", f_dat);
      perror("");
      return E_OPEN1;
    }

    while ((archivo = readdir(ddir)) != NULL)
    {
      // Si la ruta dada no acaba en "/", se lo añadimos ("directorio" -> "directorio/")
      size_t len = strlen(f_dat);
      if (len > 0 && f_dat[len - 1] != '/') {
          strcat(f_dat, "/");
      }

      // Unimos el nombre del directorio con el nombre del archivo a una variable ("directorio/archivo.dat")
      snprintf(path_archivo, sizeof(path_archivo), "%s%s", f_dat, archivo->d_name);

      // Cogemos la información del archivo dat pasado por parámetros
      if (stat(path_archivo, &archivo_stat) != 0)
      {
        fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Ha ocurrido un error al obtener la información del archivo \"%s\" (archivo) dentro del directorio \"%s\" (f_dat). ERROR: ", archivo->d_name, f_dat);
        perror("");
        closedir(ddir); // Cerramos el archivo directorio (origen)
        close(fd_repo); // Cerramos el archivo repo (destino)
        return E_DESCO;
      }

      if (S_ISREG(archivo_stat.st_mode))
      {
        // Si es un archivo regular lo insertamos
        if ((respuesta_insertar = insertar_fichero(path_archivo, fd_repo, archivo_stat.st_mode, f_mysha256_Repo)) != OK)
        {
          return respuesta_insertar;
        }
      }
    }

    closedir(ddir); // Cerramos el archivo directorio (origen)
  }
  else if (S_ISREG(f_dat_stat.st_mode))
  {
    // Si es un archivo regular, lo insertamos directamente
    if ((respuesta_insertar = insertar_fichero(f_dat, fd_repo, f_dat_stat.st_mode, f_mysha256_Repo)) != OK)
    {
      return respuesta_insertar;
    }
  }

  number_of_files_in_repo = getNumberOfFilesInRepo(fd_repo, f_mysha256_Repo);

  close(fd_repo); // Cerramos el archivo repo (destino)

  return number_of_files_in_repo;
}

int insertar_fichero(char *f_dat, int fd_repo, mode_t st_mode, char *f_mysha256_Repo)
{
  struct c_sha256header header; // Header del archivo dat (origen)
  int fd_dat;                   // Identificador archivo dat (origen)

  // Abrimos el archivo dat (origen)
  if ((fd_dat = open(f_dat, O_RDONLY)) == -1)
  {
    fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Ha ocurrido un error al abrir el archivo \"%s\" (f_dat). ERROR: ", f_dat);
    perror("");
    close(fd_repo); // Cerramos el archivo repo (destino)
    return E_OPEN1;
  }


  // Guardamos en la variable header el contenido del header del archivo dat (origen)
  if (Buildsha256RepoHeader(f_dat, &header) == -1)
  {
    fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Error al obtener el header para el archivo \"%s\" (f_dat). ERROR: ", f_dat);
    perror("");
    close(fd_dat);  // Cerramos el archivo dat (origen)
    close(fd_repo); // Cerramos el archivo repo (destino)
    return E_REPOFORM;
  }

  // Movemos el puntero al final del archivo repo (en el que queremos escribir)
  if (lseek(fd_repo, 0, SEEK_END) == -1)
  {
    fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Error al mover el puntero en el archivo \"%s\" (f_mysha256_Repo). ERROR: ", f_mysha256_Repo);
    perror("");
    close(fd_dat);  // Cerramos el archivo dat (origen)
    close(fd_repo); // Cerramos el archivo repo (destino)
    return E_DESCO;
  }

  header.permissions = st_mode;
  header.size = GetBytesInFile(fd_dat);

  // Escribimos el contenido del header en el archivo repo (destino)
  if (write(fd_repo, &header, sizeof(header)) == -1)
  {
    fprintf(stderr, "🚨 ERROR: (inserta_fichero.c) Error escribir el header en el archivo \"%s\" (f_mysha256_Repo). ERROR: ", f_mysha256_Repo);
    perror("");
    close(fd_dat);  // Cerramos el archivo dat (origen)
    close(fd_repo); // Cerramos el archivo repo (destino)
    return E_DESCO;
  }

  WriteFileDataBlocks(fd_dat, fd_repo);

  return OK;
}

/**
  Cuenta el número de archivos en el repositorio
*/
int getNumberOfFilesInRepo(int fd_repo, char *f_mysha256_Repo)
{
  int counter = 0; // Contador de archivos contenidos en el repositorio
  int number_of_blocks_in_header, number_of_bytes_to_jump;
  struct c_sha256header header;

  // Movemos el puntero al principio del archivo repo (en el que hemos añadido un nuevo repo)
  lseek(fd_repo, 0, SEEK_SET);

  while (read(fd_repo, &header, sizeof(header)) > 0)
  {
    counter++;
    /* Guardamos en number_of_blocks_in_header el número de bloques que se han
        debido de utilizar para guardar el tamaño del archivo */
    number_of_blocks_in_header = ceil((double)header.size / (double)READ_BLOCK_SIZE);
    number_of_bytes_to_jump = number_of_blocks_in_header * READ_BLOCK_SIZE;
    // Nos movemos a después del contenido del archivo
    lseek(fd_repo, number_of_bytes_to_jump, SEEK_CUR);
  }

  return counter;
}
