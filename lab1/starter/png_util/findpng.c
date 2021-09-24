#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>   /* for printf(), perror()...   */
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
 * FUNCTIONS 
 *****************************************************************************/

void searchPngFiles(char *path_base);
int checkifpng(char *file_path);
int check_png_signature(U8 *buf, size_t n);

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

    if (status == 0)
    {
        printf("%s\n", file_path);
        png_num_checker++;
    }

    free(p_buffer); /* free dynamically allocated memory */

    return 0;
}
