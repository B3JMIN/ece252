#include <stdio.h>   /* for printf(), perror()...   */
#include <stdlib.h>  /* for malloc()                */
#include <errno.h>   /* for errno                   */
#include "crc.h"     /* for crc()                   */
#include "zutil.h"   /* for mem_def() and mem_inf() */
#include "lab_png.h" /* simple PNG data structures  */
#include "crc.c"     /* for crc()                   */
#include "zutil.c"   /* for mem_def() and mem_inf() */
#define BUF_LEN (256 * 16)
#define BUF_LEN2 (256 * 32)

/******************************************************************************
 * GLOBALS 
 *****************************************************************************/
U8 gp_buf_def[BUF_LEN2]; /* output buffer for mem_def() */
U8 gp_buf_inf[BUF_LEN2]; /* output buffer for mem_inf() */

/******************************************************************************
 * FUNCTION PROTOTYPES 
 *****************************************************************************/

void init_data(U8 *buf, int len);

/******************************************************************************
 * FUNCTIONS 
 *****************************************************************************/

/**
 * @brief initialize memory with 256 chars 0 - 255 cyclically 
 */
void init_data(U8 *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        buf[i] = i % 256;
    }
}

int is_png(U8 *buf, size_t n)
{
    printf("is_png working...");
    return 0;
}

int get_png_height_using_buf(U8 *buf)
{
    return (((unsigned char)buf[20] << 24) + ((unsigned char)buf[21] << 16) + ((unsigned char)buf[22] << 8) + ((unsigned char)buf[23] << 0));
}

int get_png_width_using_buf(U8 *buf)
{
    return (((unsigned char)buf[16] << 24) + ((unsigned char)buf[17] << 16) + ((unsigned char)buf[18] << 8) + ((unsigned char)buf[19] << 0));
}

int get_png_data_IHDR(struct data_IHDR *out, FILE *fp, long offset, int whence)
{
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

int main(int argc, char **argv)
{

    U8 *p_buffer = NULL; /* a buffer that contains some data to play with */
    /*U32 crc_val = 0;      CRC value                                     */
    int ret = 0;     /* return value for various routines             */
    U64 len_def = 0; /* compressed data length                        */
    U64 len_inf = 0; /* uncompressed data length                      */

    /* Step 1: Initialize some data in a buffer */
    /* Step 1.1: Allocate a dynamic buffer */
    p_buffer = malloc(BUF_LEN);
    if (p_buffer == NULL)
    {
        perror("malloc");
        return errno;
    }

    /* Step 1.2: Fill the buffer with some data */
    init_data(p_buffer, BUF_LEN);

    /* Step 2: Demo how to use zlib utility */
    ret = mem_def(gp_buf_def, &len_def, p_buffer, BUF_LEN, Z_DEFAULT_COMPRESSION);
    if (ret == 0)
    { /* success */
        /*printf("original len = %d, len_def = %lu\n", BUF_LEN, len_def);*/
    }
    else
    { /* failure */
        /*fprintf(stderr, "mem_def failed. ret = %d.\n", ret);*/
        return ret;
    }

    ret = mem_inf(gp_buf_inf, &len_inf, gp_buf_def, len_def);
    if (ret == 0)
    { /* success */
        /*printf("original len = %d, len_def = %lu, len_inf = %lu\n",
               BUF_LEN, len_def, len_inf);*/
    }
    else
    { /* failure */
        /*fprintf(stderr, "mem_def failed. ret = %d.\n", ret);*/
    }

    /* Step 3: Demo how to use the crc utility */
    /*crc_val = crc(gp_buf_def, len_def); // down cast the return val to U32
    printf("crc_val = %u\n", crc_val);*/

    /* .............................................. */
    /* open the file in read mode */
    FILE *f = fopen(argv[1], "r");

    if (f == NULL)
    {
        perror("fopen");
        free(p_buffer);
        return -1;
    }

    /* reading the IHDR chunk which has a constant length of 33 */
    int size = fread(p_buffer, 1, 33, f);

    if (size == 0)
    {
        printf("Nothing read from the png file!\n");
    }

    /* extracting and checking against png signature */
    int status = check_png_signature(p_buffer, 33);

    if (status != 0)
    {
        printf("%s: Not a PNG file\n", argv[1]);
        exit(-1);
    }

    /* since file is a png file in this block, find the width and height*/
    int width = get_png_width_using_buf(p_buffer);
    int height = get_png_height_using_buf(p_buffer);

    /* if it is a png file, continue with life */

    struct simple_PNG *simple_PNG_p = malloc(3 * (2 * sizeof(U32)) + (2 * sizeof(U8)));
    struct chunk *chunk_p = malloc((2 * sizeof(U32)) + (2 * sizeof(U8)));

    U32 ihdr_actual;
    U32 idat_actual;
    U32 iend_actual;

    /*********** POPULATING IHDR CHUNK- BEGINS..........*********************************/

    /* populating the type of the chunk - IHDR*/
    for (int i = 0; i < 4; i++)
    {
        chunk_p->type[i] = p_buffer[i + 12];
    }

    /* populating the length of the chunk - IHDR*/
    chunk_p->length = ((unsigned char)p_buffer[8] << 24) + ((unsigned char)p_buffer[9] << 16) + ((unsigned char)p_buffer[10] << 8) + ((unsigned char)p_buffer[11] << 0);

    /* populating the crc of the chunk - IHDR*/
    chunk_p->crc = ((unsigned char)p_buffer[29] << 24) + ((unsigned char)p_buffer[30] << 16) + ((unsigned char)p_buffer[31] << 8) + ((unsigned char)p_buffer[32] << 0);
    ihdr_actual = chunk_p->crc;

    /* populating the data field of the chunk (we pass in type and data) - IHDR*/
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

    /*********** POPULATING IHDR CHUNK- ENDS..........********************************************/

    /* getting computed crc value using the crc function */
    U32 crc_computed_ihdr = crc(ihdr_data, chunk_p->length + 4);

    int crc_deviation_ihdr = crc_computed_ihdr - chunk_p->crc;

    /*********** POPULATING IDAT CHUNK- BEGINS..........*****************************************/

    fseek(f, 33, SEEK_SET);
    size = fread(p_buffer, 1, 33, f);

    /* populating the type of the chunk - IDAT*/
    for (int i = 0; i < 4; i++)
    {
        chunk_p->type[i] = p_buffer[i + 4];
    }

    /* populating the length of the chunk - IHDR*/
    chunk_p->length = ((unsigned char)p_buffer[0] << 24) + ((unsigned char)p_buffer[1] << 16) + ((unsigned char)p_buffer[2] << 8) + ((unsigned char)p_buffer[3] << 0);

    /* defining a buffer to store the idat data*/
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

    /* populating the crc of the chunk - IHDR*/
    fseek(f, 41 + chunk_p->length, SEEK_SET);
    size = fread(p_buffer, 1, 16, f);

    chunk_p->crc = ((unsigned char)p_buffer[0] << 24) + ((unsigned char)p_buffer[1] << 16) + ((unsigned char)p_buffer[2] << 8) + ((unsigned char)p_buffer[3] << 0);
    idat_actual = chunk_p->crc;

    simple_PNG_p->p_IDAT = chunk_p;

    /*********** POPULATING IDAT CHUNK- ENDS..........******************************************/

    /* getting computed crc value using the crc function */
    U32 crc_computed_idat = crc(idat_data_type, chunk_p->length + 4);
    int crc_deviation_idat = crc_computed_idat - chunk_p->crc;
    free(idat_data_type);

    /*********** POPULATING IEND CHUNK- BEGINS..........*****************************************/

    fseek(f, 41 + chunk_p->length + 4, SEEK_SET);
    size = fread(p_buffer, 1, 12, f);

    /* populating the type of the chunk - IEND*/
    for (int i = 0; i < 4; i++)
    {
        chunk_p->type[i] = p_buffer[i + 4];
    }

    /*IEND length*/
    chunk_p->length = ((unsigned char)p_buffer[0] << 24) + ((unsigned char)p_buffer[1] << 16) + ((unsigned char)p_buffer[2] << 8) + ((unsigned char)p_buffer[3] << 0);

    /* populating the crc of the chunk - IEND*/
    chunk_p->crc = ((unsigned char)p_buffer[8] << 24) + ((unsigned char)p_buffer[9] << 16) + ((unsigned char)p_buffer[10] << 8) + ((unsigned char)p_buffer[11] << 0);
    iend_actual = chunk_p->crc;

    /*no data field for iend*/
    chunk_p->p_data = NULL;

    simple_PNG_p->p_IEND = chunk_p;

    /*********** POPULATING IEND CHUNK- ENDS..........********************************************/

    /* getting computed crc value using the crc function */
    U32 crc_computed_iend = crc(chunk_p->type, 4);
    int crc_deviation_iend = crc_computed_iend - chunk_p->crc;
    /*printf("hhhh %d\n", crc_deviation_iend);
    printf("hhhh %d\n", crc_deviation_ihdr);
    printf("hhhh %d\n", crc_deviation_idat);*/

    if (crc_deviation_iend == 0 && crc_deviation_idat == 0 && crc_deviation_ihdr == 0)
    {
        /*not corrupted*/
        printf("%s: %d x %d\n", argv[1], width, height);
    }
    else
    {
        /*corrupted*/
        printf("%s: %d x %d\n", argv[1], width, height);
        if (crc_deviation_ihdr != 0)
        {
            printf("IHDR chunk CRC error: computed %x, expected %x\n", crc_computed_ihdr, ihdr_actual);
        }
        else if (crc_deviation_idat != 0)
        {
            printf("IDAT chunk CRC error: computed %x, expected %x\n", crc_computed_idat, idat_actual);
        }
        else if (crc_deviation_iend != 0)
        {
            printf("IEND chunk CRC error: computed %x, expected %x\n", crc_computed_iend, iend_actual);
        }
    }

    /* Clean up */
    free(p_buffer); /* free dynamically allocated memory */

    free(simple_PNG_p);
    free(chunk_p);
    free(ihdr_data_exclusive);
    free(ihdr_data);
    free(idat_data_buf);
    fclose(f);
    return 0;
}