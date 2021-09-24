#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>  /* for printf().  man 3 printf */
#include <stdlib.h> /* for exit().    man 3 exit   */
#include <string.h> /* for strcat().  man strcat   */
#include "lab_png.h"
#include "crc.h"

int main(int argc, char **argv)
{
  FILE *fp;
  U8 buffer_8[8];
  U8 buffer_4[8];
  struct data_IHDR ihdr = {.width = 0, .height = 0, .bit_depth = 0, .color_type = 0, .compression = 0, .filter = 0, .interlace = 0};
  data_IHDR_p out = &ihdr;
  fp = fopen(argv[1], "r");

  if (fp == NULL)
  {
    perror("fopen has the error: ");
    // free()
    return -1;
  }
  
  size_t fread_status = fread(buffer_8, 8, 1, fp);

  if (fread_status == 0)
  {
    printf("Nothing read from this PNG file!\n");
  }
  
  if (is_png(buffer_8, 0))
  {
    checkForIHDR(fp, buffer_4, argv[1]);
    checkForIDAT(fp, buffer_4);
    checkForCRC(fp, buffer_4);
  }else{
    printf("%s: Not a PNG file\n", argv[1]);
  }
  fclose(fp);
  return 0;
}

void checkForIHDR(FILE *fp, U32 buffer_4, char *fileName)
{
  struct data_IHDR ihdr = {.width = 0, .height = 0, .bit_depth = 0, .color_type = 0, .compression = 0, .filter = 0, .interlace = 0};
  data_IHDR_p out = &ihdr;
  get_png_data_IHDR(out, fp, 0, 0);
  printf("%s: %d x %d\n", fileName, get_png_width(out), get_png_height(out));
  fseek(fp, 12, SEEK_SET);
  U8 ihdr_buffer[17];
  fread(ihdr_buffer, 17, 1, fp);
  unsigned long actual = crc(ihdr_buffer, 17);
  fseek(fp, 29, SEEK_SET);
  fread(buffer_4, 4, 1, fp);
  U32 *p_int = (U32 *)buffer_4;
  unsigned long computed = htonl(*p_int);
  if (computed != actual)
  {
    printf("IHDR chunk CRC error: computed %lx, actual %lx\n", computed, actual);
  }
  free(p_int);
}

void checkForIDAT(FILE *fp, U32 buffer_4)
{
  fseek(fp, 33, SEEK_SET);
  fread(buffer_4, 4, 1, fp);
  U32 *p_int = (U32 *)buffer_4;
  unsigned long length = htonl(*p_int);
  U8 *idat_buffer = malloc((length + 4) * sizeof(U8));
  fseek(fp, 37, SEEK_SET);
  fread(idat_buffer, length + 4, 1, fp);
  unsigned long actual = crc(idat_buffer, length + 4);
  fseek(fp, -16, SEEK_END);
  fread(buffer_4, 4, 1, fp);
  p_int = (U32 *)buffer_4;
  unsigned long computed = htonl(*p_int);
  if (computed != actual)
  {
    printf("IDAT chunk CRC error: computed %lx, actual %lx\n", computed, actual);
  }
  free(p_int);
  free(idat_buffer);
}

void checkForCRC(FILE *fp, U32 buffer_4)
{
  fseek(fp, -8, SEEK_END);
  U8 iend_buffer[4];
  U32 *p_int = (U32 *)buffer_4;
  fread(iend_buffer, 4, 1, fp);
  unsigned long actual = crc(iend_buffer, 4);
  fseek(fp, -4, SEEK_END);
  fread(buffer_4, 4, 1, fp);
  p_int = (U32 *)buffer_4;
  unsigned long computed = htonl(*p_int);
  if (computed != actual)
  {
    printf("IEND chunk CRC error: computed %lx, actual %lx\n", computed, actual);
  }
  free(p_int);
}