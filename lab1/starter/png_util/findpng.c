#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>   /* for printf(), perror()...   */
#include <stdlib.h>  /* for malloc()                */
#include <errno.h>   /* for errno                   */
#include "crc.h"     /* for crc()                   */
#include "crc.c"     /* for crc()                   */
#include "zutil.h"   /* for mem_def() and mem_inf() */
#include "lab_png.h" /* simple PNG data structures  */
#include <sys/types.h>
#include <dirent.h>
#include "pnginfo.h"

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>   /* for printf(), perror()...   */
#include <stdlib.h>  /* for malloc()                */
#include <errno.h>   /* for errno                   */
#include "crc.h"     /* for crc()                   */
#include "crc.c"     /* for crc()                   */
#include "zutil.h"   /* for mem_def() and mem_inf() */
#include "lab_png.h" /* simple PNG data structures  */
#include <sys/types.h>
#include <dirent.h>

/******************************************************************************
 * DEFINED MACROS 
 *****************************************************************************/
#define BUF_LEN (256 * 16)
#define BUF_LEN2 (256 * 32)

/******************************************************************************
 * GLOBALS 
 *****************************************************************************/
U8 gp_buf_def[BUF_LEN2]; /* output buffer for mem_def() */
U8 gp_buf_inf[BUF_LEN2]; /* output buffer for mem_inf() */
int png_num_checker = 0;

/******************************************************************************
 * FUNCTION PROTOTYPES 
 *****************************************************************************/

/******************************************************************************
 * FUNCTIONS 
 *****************************************************************************/

void searchPngFiles(char *path_base);
int checkifpng(char *file_path);
int check_png_signature(U8 *buf, size_t n);
void init_data(U8 *buf, int len);

int main(int argc, char **argv)
{

    if (argc == 1)
    {
        printf("Usage: %s <desired directory name>\n", argv[1]);
        exit(1);
    }

    searchPngFiles(argv[1]);
    if (png_num_checker == 0)
    {
        printf("findpng: No PNG file found\n");
    }
    return 0;
}

void searchPngFiles(char *path_base)
{
    struct dirent *dp;

    DIR *dir = opendir(path_base);

    if (dir == NULL)
    {
        return;
    }

    char file_path[10000];

    while ((dp = readdir(dir)) != NULL)
    {
        char *path_name_buf = malloc(BUF_LEN2);

        if (strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)
        {
            strcpy(file_path, path_base);

            strcat(file_path, "/");

            char *ptr = strstr(dp->d_name, ".png");

            strcat(file_path, dp->d_name);

            memset(path_name_buf, 0, BUF_LEN2);
            int temp = 1;

            if (ptr == NULL)
            {
                
            }
            else
            {
                sprintf(path_name_buf, "%s/%s", path_base, dp->d_name);
                FILE *f = fopen(file_path, "r");
                temp = 0;
            }

            if (temp == 0)
            {
                int png_checker = checkifpng(path_name_buf);
            }

            free(path_name_buf);

            searchPngFiles(file_path);
        }
    }

    closedir(dir);
}

int checkifpng(char *file_path)
{

    U8 *p_buffer = NULL; 
    int ret = 0;         
    U64 len_def = 0;     
    U64 len_inf = 0;     

    p_buffer = malloc(BUF_LEN);
    if (p_buffer == NULL)
    {
        perror("malloc");
        return errno;
    }

    init_data(p_buffer, BUF_LEN);

    FILE *f = fopen(file_path, "r");

    if (f == NULL)
    {
        perror("fopen");
        free(p_buffer);
        return -1;
    }

    int size = fread(p_buffer, 1, 33, f);

    if (size == 0)
    {
        printf("Nothing read from the png file!\n");
    }

    int status = check_png_signature(p_buffer, 33);

    if (status != 0)
    {
        return -1;
    }

    struct simple_PNG *simple_PNG_p = malloc(3 * (2 * sizeof(U32)) + (2 * sizeof(U8)));
    struct chunk *chunk_p = malloc((2 * sizeof(U32)) + (2 * sizeof(U8)));

    U32 ihdr_actual;
    U32 idat_actual;
    U32 iend_actual;

    for (int i = 0; i < 4; i++)
    {
        chunk_p->type[i] = p_buffer[i + 12];
    }

    chunk_p->length = ((unsigned char)p_buffer[8] << 24) + ((unsigned char)p_buffer[9] << 16) + ((unsigned char)p_buffer[10] << 8) + ((unsigned char)p_buffer[11] << 0);

    chunk_p->crc = ((unsigned char)p_buffer[29] << 24) + ((unsigned char)p_buffer[30] << 16) + ((unsigned char)p_buffer[31] << 8) + ((unsigned char)p_buffer[32] << 0);
    ihdr_actual = chunk_p->crc;

    U8 *ihdr_data = malloc(chunk_p->length + 4);
    for (int i = 0; i < 17; i++)
    {
        ihdr_data[i] = p_buffer[12 + i];
    }

    U8 *ihdr_data_exclusive = malloc(chunk_p->length);
    for (int i = 0; i < chunk_p->length; i++)
    {
        ihdr_data_exclusive[i] = ihdr_data[i + 4];
    }

    chunk_p->p_data = ihdr_data_exclusive;

    simple_PNG_p->p_IHDR = chunk_p;

    U32 crc_computed_ihdr = crc(ihdr_data, chunk_p->length + 4);

    int crc_deviation_ihdr = crc_computed_ihdr - chunk_p->crc;

    fseek(f, 33, SEEK_SET);
    size = fread(p_buffer, 1, 33, f);

    for (int i = 0; i < 4; i++)
    {
        chunk_p->type[i] = p_buffer[i + 4];
    }

    chunk_p->length = ((unsigned char)p_buffer[0] << 24) + ((unsigned char)p_buffer[1] << 16) + ((unsigned char)p_buffer[2] << 8) + ((unsigned char)p_buffer[3] << 0);

    U8 *idat_data_buf = malloc(chunk_p->length);
    fseek(f, 41, SEEK_SET);
    size = fread(idat_data_buf, 1, chunk_p->length, f);
    chunk_p->p_data = idat_data_buf;

    U8 *idat_data_type = malloc(chunk_p->length + 4);

    for (int i = 0; i < chunk_p->length + 4; i++)
    {
        if (i == 0 || i == 1 || i == 2 || i == 3)
        {
            idat_data_type[i] = chunk_p->type[i];
        }
        else
        {
            idat_data_type[i] = idat_data_buf[i - 4];
        }
    }

    fseek(f, 41 + chunk_p->length, SEEK_SET);
    size = fread(p_buffer, 1, 16, f);

    chunk_p->crc = ((unsigned char)p_buffer[0] << 24) + ((unsigned char)p_buffer[1] << 16) + ((unsigned char)p_buffer[2] << 8) + ((unsigned char)p_buffer[3] << 0);
    idat_actual = chunk_p->crc;

    simple_PNG_p->p_IDAT = chunk_p;

    U32 crc_computed_idat = crc(idat_data_type, chunk_p->length + 4);
    int crc_deviation_idat = crc_computed_idat - chunk_p->crc;
    free(idat_data_type);

    fseek(f, 41 + chunk_p->length + 4, SEEK_SET);
    size = fread(p_buffer, 1, 12, f);

    for (int i = 0; i < 4; i++)
    {
        chunk_p->type[i] = p_buffer[i + 4];
    }

    chunk_p->length = ((unsigned char)p_buffer[0] << 24) + ((unsigned char)p_buffer[1] << 16) + ((unsigned char)p_buffer[2] << 8) + ((unsigned char)p_buffer[3] << 0);

    chunk_p->crc = ((unsigned char)p_buffer[8] << 24) + ((unsigned char)p_buffer[9] << 16) + ((unsigned char)p_buffer[10] << 8) + ((unsigned char)p_buffer[11] << 0);
    iend_actual = chunk_p->crc;

    chunk_p->p_data = NULL;

    simple_PNG_p->p_IEND = chunk_p;

    U32 crc_computed_iend = crc(chunk_p->type, 4);
    int crc_deviation_iend = crc_computed_iend - chunk_p->crc;

    if (status == 0)
    {
        printf("%s\n", file_path);
        png_num_checker++;
    }

    free(p_buffer); /* free dynamically allocated memory */

    free(simple_PNG_p);
    free(chunk_p);
    free(ihdr_data_exclusive);
    free(ihdr_data);
    free(idat_data_buf);

    return 0;
}

int check_png_signature(U8 *buf, size_t n)
{
    if (!((unsigned char)buf[0] == 0x89 || (unsigned char)buf[1] == 'P' || (unsigned char)buf[2] == 'N' || (unsigned char)buf[3] == 'G' ||
          (unsigned char)buf[4] == 0x0D || (unsigned char)buf[5] == 0x0A || (unsigned char)buf[6] == 0x1A || (unsigned char)buf[7] == 0x0A))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void init_data(U8 *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        buf[i] = i % 256;
    }
}