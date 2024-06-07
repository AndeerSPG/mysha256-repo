#include <fcntl.h>

#include "s_my_sha256header.h"
#include <math.h>

#define E_OPEN1 (-1)
#define E_OPEN2 (-2)
#define E_REPOFORM (-3)
#define E_DESCO (-99)

int extraer_fichero(char *f_mysha256_Repo, char *f_dat)
{
  char buffer[4096]; // Buffer para guardar el contenido del archivo antes de la extracción

  int fd_repo; // Identificador archivo repo (origen)
  int fd_dat;  // Identificador archivo dat (el que se quiere copiar el contenido)

  struct c_sha256header header;

  int should_continue = 1;        // Auxiliar utilizada para el bucle de archivos en el repositorio
  int number_of_blocks_in_header; // Número de bloques que se utiliza para guardar el contenido del archivo
  long number_of_bytes_left;      // Auxiliar para controlar cuántos bytes del contenido de un archivo quedan por leer
  long number_of_bytes_to_read;   // Auxiliar para controlar cuántos bytes se van a leer en cada iteración

  // Abrimos el archivo repo (origen)
  if ((fd_repo = open(f_mysha256_Repo, O_RDONLY, 0644)) == -1)
  {
    fprintf(stderr, "🚨 ERROR: (extrae_fichero.c) Ha ocurrido un error al abrir el archivo \"%s\" (f_mysha256_Repo). ERROR: ", f_mysha256_Repo);
    perror("");
    return E_OPEN2;
  }

  // Movemos el puntero al principio del archivo repo (en el que hemos añadido un nuevo repo)
  lseek(fd_repo, 0, SEEK_SET);

  while (should_continue == 1)
  {
    // Leemos el header del archivo
    if (read(fd_repo, &header, sizeof(header)) > 0)
    {
      // Calcular el número de bloques utilizados para el archivo
      number_of_blocks_in_header = ceil((float)header.size / (float)READ_BLOCK_SIZE);

      // Los nombres de los archivos son iguales
      if (strcmp(f_dat, header.fname) == 0)
      {
        // Como se ha encontrado el archivo, el bucle debería terminar
        should_continue = 0;

        // Abrimos el archivo destino, si no existe lo crea y si existe borra todo lo qeue tenia antes, para sobreescribirlo
        int statchmod = header.permissions & (S_IRWXU | S_IRWXG | S_IRWXO);
        if ((fd_dat = open(f_dat, O_CREAT | O_RDWR,statchmod)) == -1)
        {
            fprintf(stderr, "🚨 ERROR: (extrae_fichero.c) Ha ocurrido un error al abrir el archivo \"%s\" (f_dat). ERROR: ", f_dat);
            perror("");
            close(fd_repo); // Cerramos el archivo repo (origen)
            return E_OPEN1;
        }

        // El número restante de bytes por leer (el tamaño especificado en el header)
        number_of_bytes_left = header.size;

        // Recorremos el contenido
        for (number_of_blocks_in_header; number_of_blocks_in_header > 0; number_of_blocks_in_header--)
        {
          number_of_bytes_to_read = READ_BLOCK_SIZE;
          number_of_bytes_left -= READ_BLOCK_SIZE;

          /* Si el número de bytes pendientes es negativo querrá decir que
             es el número de bytes que se han rellenado con 0s*/
          if (number_of_bytes_left < 0)
          {
            // Sumamos el número negativo de bytes para no leerlos, ya que son 0s
            number_of_bytes_to_read += number_of_bytes_left;
          }

          // Guardamos el contenido de un bloque en un buffer
          if (read(fd_repo, buffer, number_of_bytes_to_read) == -1)
          {
            fprintf(stderr, "🚨 ERROR: (extrae_fichero.c) Ha ocurrido un error al leer el contenido original del archivo \"%s\" (f_dat). ERROR: ", f_dat);
            perror("");
            close(fd_repo); // Cerramos el archivo repo (origen)
            close(fd_dat);  // Cerramos el archivo dat (destino)
            return E_DESCO;
          }

          // Entrará si los bytes que quedan por leer son 0s
          if (number_of_bytes_left < 0)
          {
            // Movemos al cursor del repo al momento después del número de bytes que han sobrado (los 0s)
            lseek(fd_repo, -(number_of_bytes_left), SEEK_CUR);
          }

          // Escribir el contenido del archivo
          if (write(fd_dat, &buffer, number_of_bytes_to_read) == -1)
          {
            fprintf(stderr, "🚨 ERROR: (extrae_fichero.c) Ha ocurrido un error al volver a escribir el contenido del archivo original \"%s\" (f_dat). ERROR: ", f_dat);
            perror("");
            close(fd_repo); // Cerramos el archivo repo (origen)
            close(fd_dat);  // Cerramos el archivo dat (destino)
            return E_DESCO;
          }
        }
      }
      else
      {
        // No es el archivo que estamos buscando, avanzamos el puntero al siguiente header
        number_of_bytes_to_read = number_of_blocks_in_header * READ_BLOCK_SIZE;
        // Nos movemos a después del contenido del archivo
        lseek(fd_repo, number_of_bytes_to_read, SEEK_CUR);
      }
    }
    else
    {
      should_continue = 0;
      fprintf(stderr, "🚨 ERROR: (extrae_fichero.c) Ha ocurrido un error. Parece que no existe el archivo \"%s\" (f_dat) en el repositorio \"%s\" (f_mysha256_Repo). ERROR: ", f_dat, f_mysha256_Repo);
      perror("");
      close(fd_repo); // Cerramos el archivo repo (origen)
      close(fd_dat);  // Cerramos el archivo dat (destino)
      return E_DESCO;
    }
  }

  close(fd_repo); // Cerramos el archivo repo (origen)
  close(fd_dat);  // Cerramos el archivo dat (destino)

  return OK;
}
