/**
 * @biref To demonstrate how to use zutil.c and crc.c functions
 *
 * Copyright 2018-2020 Yiqing Huang
 *
 * This software may be freely redistributed under the terms of MIT License
 */

#include <stdio.h>  /* for printf(), perror()...   */
#include <stdlib.h> /* for malloc()                */
#include <errno.h>  /* for errno                   */

#include "lab_png.h" /* simple PNG data structures  */
#include <string.h>
#include "crc.h"   /* for crc()                   */
#include "zutil.h" /* for mem_def() and mem_inf() */

/*LIMITING PNGS TO 16MB*/
#define ESTIMATED_PNG_MAX (16 * 1024 * 1024)

/******************************************************************************
 * DEFINED MACROS 
 *****************************************************************************/
#define BUF_LEN (256 * 16)
#define BUF_LEN2 (256 * 32)

/******************************************************************************
 * GLOBALS 
 *****************************************************************************/
U8 gp_buf_def[ESTIMATED_PNG_MAX]; /* output buffer for mem_def() */
U8 gp_buf_inf[BUF_LEN2];          /* output buffer for mem_inf() */

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

int main(int argc, char **argv)
{
    U32 crc_val = 0; /* CRC value                                     */
    int ret = 0;     /* return value for various routines             */
    U64 len_def = 0; /* compressed data length                        */
    U64 len_inf1;
    U64 len_inf2;

    /*printf("in mainnnn");*/

    unsigned char *p_buffer1 = (unsigned char *)malloc(ESTIMATED_PNG_MAX);
    unsigned char *p_buffer2 = (unsigned char *)malloc(ESTIMATED_PNG_MAX);
    unsigned char *p_buffer_data_ty1 = (unsigned char *)malloc(BUF_LEN);
    unsigned char *p_buffer_data_ty2 = (unsigned char *)malloc(BUF_LEN);
    unsigned char *concatenated_buf;
    unsigned char *crc_calc;
    unsigned char *concat_final = (unsigned char *)malloc(ESTIMATED_PNG_MAX);
    unsigned char *input_buffer1;
    unsigned char *data_p_buffer1 = (unsigned char *)malloc(ESTIMATED_PNG_MAX);
    unsigned char *p_buffer1_data;
    unsigned char *p_buffer2_data;
    unsigned char *data_p_buffer2 = (unsigned char *)malloc(ESTIMATED_PNG_MAX);

    U64 height_sum = 0;

    for (int file_instance_num = 0; file_instance_num < argc - 2; file_instance_num++)
    {

        FILE *f1 = fopen(argv[file_instance_num + 1], "r"); /*file1+file2 in 2nd iteration and so onnn*/
        FILE *f2 = fopen(argv[file_instance_num + 2], "r");

        if (f1 == NULL || f2 == NULL)
        {
            perror("fopen");
            if (f1 == NULL)
            {
                free(p_buffer1);
            }
            else
            {
                free(p_buffer2);
            }

            return -1;
        }

        fread(p_buffer1, 1, ESTIMATED_PNG_MAX, f1);

        fread(p_buffer2, 1, ESTIMATED_PNG_MAX, f2);

        /**********code to store file 1 in the respective structs****************************************************************************************/
        struct simple_PNG *simple_PNG_p1 = malloc(3 * (2 * sizeof(U32)) + (2 * sizeof(U8)));
        struct chunk *chunk_p1 = malloc((2 * sizeof(U32)) + (2 * sizeof(U8)));

        U32 ihdr_actual1;
        U32 idat_actual1;
        U32 iend_actual1;

        /* populating the type of the chunk - IHDR*/
        for (int i = 0; i < 4; i++)
        {
            chunk_p1->type[i] = p_buffer_data_ty1[i + 12];
        }

        /* populating the length of the chunk - IHDR*/
        chunk_p1->length = ((unsigned char)p_buffer_data_ty1[8] << 24) + ((unsigned char)p_buffer_data_ty1[9] << 16) + ((unsigned char)p_buffer_data_ty1[10] << 8) + ((unsigned char)p_buffer_data_ty1[11] << 0);

        /* populating the crc of the chunk - IHDR*/
        chunk_p1->crc = ((unsigned char)p_buffer_data_ty1[29] << 24) + ((unsigned char)p_buffer_data_ty1[30] << 16) + ((unsigned char)p_buffer_data_ty1[31] << 8) + ((unsigned char)p_buffer_data_ty1[32] << 0);
        ihdr_actual1 = chunk_p1->crc;

        /* populating the data field of the chunk (we pass in type and data) - IHDR*/
        U8 *ihdr_data1 = malloc(chunk_p1->length + 4);
        for (int i = 0; i < 17; i++)
        {
            ihdr_data1[i] = p_buffer_data_ty1[12 + i];
        }

        U8 *ihdr_data_exclusive1 = malloc(chunk_p1->length);
        for (int i = 0; i < chunk_p1->length; i++)
        {
            ihdr_data_exclusive1[i] = ihdr_data1[i + 4];
        }

        chunk_p1->p_data = ihdr_data_exclusive1;

        simple_PNG_p1->p_IHDR = chunk_p1;

        /*********** POPULATING IHDR CHUNK- ENDS..........********************************************/

        /* getting computed crc value using the crc function */
        U32 crc_computed_ihdr1 = crc(ihdr_data1, chunk_p1->length + 4);

        /*********** POPULATING IDAT CHUNK- BEGINS..........*****************************************/

        fseek(f1, 33, SEEK_SET);
        /*int size = fread(p_buffer_data_ty1, 1, 33, f1);*/
        memset(p_buffer_data_ty1, 0, BUF_LEN);

        /* populating the type of the chunk - IDAT*/
        for (int i = 0; i < 4; i++)
        {
            chunk_p1->type[i] = p_buffer_data_ty1[i + 4];
        }

        /* populating the length of the chunk - IHDR*/
        chunk_p1->length = ((unsigned char)p_buffer_data_ty1[0] << 24) + ((unsigned char)p_buffer_data_ty1[1] << 16) + ((unsigned char)p_buffer_data_ty1[2] << 8) + ((unsigned char)p_buffer_data_ty1[3] << 0);

        /* defining a buffer to store the idat data*/
        U8 *idat_data_buf1 = malloc(chunk_p1->length);
        fseek(f1, 41, SEEK_SET);
        /*size = fread(idat_data_buf1, 1, chunk_p1->length, f1);*/
        memset(p_buffer_data_ty1, 0, BUF_LEN);

        chunk_p1->p_data = idat_data_buf1;

        U8 *idat_data_type1 = malloc(chunk_p1->length + 4);

        for (int i = 0; i < chunk_p1->length + 4; i++)
        {
            if (i == 0 || i == 1 || i == 2 || i == 3)
            {
                idat_data_type1[i] = chunk_p1->type[i];
            }
            else
            {
                idat_data_type1[i] = idat_data_buf1[i - 4];
            }
        }

        /* populating the crc of the chunk - IHDR*/
        fseek(f1, 41 + chunk_p1->length, SEEK_SET);
        /*size = fread(p_buffer_data_ty1, 1, 16, f1);*/
        memset(p_buffer_data_ty1, 0, BUF_LEN);

        chunk_p1->crc = ((unsigned char)p_buffer_data_ty1[0] << 24) + ((unsigned char)p_buffer_data_ty1[1] << 16) + ((unsigned char)p_buffer_data_ty1[2] << 8) + ((unsigned char)p_buffer_data_ty1[3] << 0);
        idat_actual1 = chunk_p1->crc;

        simple_PNG_p1->p_IDAT = chunk_p1;

        /*********** POPULATING IDAT CHUNK- ENDS..........******************************************/

        /* getting computed crc value using the crc function */
        U32 crc_computed_idat1 = crc(idat_data_type1, chunk_p1->length + 4);
        free(idat_data_type1);

        /*********** POPULATING IEND CHUNK- BEGINS..........*****************************************/

        fseek(f1, 41 + chunk_p1->length + 4, SEEK_SET);
        /*size = fread(p_buffer_data_ty1, 1, 12, f1);*/
        memset(p_buffer_data_ty1, 0, BUF_LEN);

        /* populating the type of the chunk - IEND*/
        for (int i = 0; i < 4; i++)
        {
            chunk_p1->type[i] = p_buffer_data_ty1[i + 4];
        }

        /*IEND length*/
        chunk_p1->length = ((unsigned char)p_buffer_data_ty1[0] << 24) + ((unsigned char)p_buffer_data_ty1[1] << 16) + ((unsigned char)p_buffer_data_ty1[2] << 8) + ((unsigned char)p_buffer_data_ty1[3] << 0);

        /* populating the crc of the chunk - IEND*/
        chunk_p1->crc = ((unsigned char)p_buffer_data_ty1[8] << 24) + ((unsigned char)p_buffer_data_ty1[9] << 16) + ((unsigned char)p_buffer_data_ty1[10] << 8) + ((unsigned char)p_buffer_data_ty1[11] << 0);
        iend_actual1 = chunk_p1->crc;

        /*no data field for iend*/
        chunk_p1->p_data = NULL;

        simple_PNG_p1->p_IEND = chunk_p1;

        /*************************************************************************************************/

        /***********code to store the 2nd file in structs*****************************************************************************************************/

        struct simple_PNG *simple_PNG_p2 = malloc(3 * (2 * sizeof(U32)) + (2 * sizeof(U8)));
        struct chunk *chunk_p2 = malloc((2 * sizeof(U32)) + (2 * sizeof(U8)));

        U32 ihdr_actual2;
        U32 idat_actual2;
        U32 iend_actual2;

        /* populating the type of the chunk - IHDR*/
        for (int i = 0; i < 4; i++)
        {
            chunk_p2->type[i] = p_buffer_data_ty2[i + 12];
        }

        /* populating the length of the chunk - IHDR*/
        chunk_p2->length = ((unsigned char)p_buffer_data_ty2[8] << 24) + ((unsigned char)p_buffer_data_ty2[9] << 16) + ((unsigned char)p_buffer_data_ty2[10] << 8) + ((unsigned char)p_buffer_data_ty2[11] << 0);

        /* populating the crc of the chunk - IHDR*/
        chunk_p2->crc = ((unsigned char)p_buffer_data_ty2[29] << 24) + ((unsigned char)p_buffer_data_ty2[30] << 16) + ((unsigned char)p_buffer_data_ty2[31] << 8) + ((unsigned char)p_buffer_data_ty2[32] << 0);
        ihdr_actual2 = chunk_p2->crc;

        /* populating the data field of the chunk (we pass in type and data) - IHDR*/
        U8 *ihdr_data2 = malloc(chunk_p2->length + 4);
        for (int i = 0; i < 17; i++)
        {
            ihdr_data2[i] = p_buffer_data_ty2[12 + i];
        }

        U8 *ihdr_data_exclusive2 = malloc(chunk_p2->length);
        for (int i = 0; i < chunk_p2->length; i++)
        {
            ihdr_data_exclusive2[i] = ihdr_data2[i + 4];
        }

        chunk_p2->p_data = ihdr_data_exclusive2;

        simple_PNG_p2->p_IHDR = chunk_p2;

        /*********** POPULATING IHDR CHUNK- ENDS..........********************************************/

        /* getting computed crc value using the crc function */
        U32 crc_computed_ihdr2 = crc(ihdr_data2, chunk_p2->length + 4);

        /*********** POPULATING IDAT CHUNK- BEGINS..........*****************************************/

        fseek(f2, 33, SEEK_SET);
        /*size = fread(p_buffer_data_ty2, 1, 33, f2);*/
        memset(p_buffer_data_ty2, 0, BUF_LEN);

        /* populating the type of the chunk - IDAT*/
        for (int i = 0; i < 4; i++)
        {
            chunk_p2->type[i] = p_buffer_data_ty2[i + 4];
        }

        /* populating the length of the chunk - IHDR*/
        chunk_p2->length = ((unsigned char)p_buffer_data_ty2[0] << 24) + ((unsigned char)p_buffer_data_ty2[1] << 16) + ((unsigned char)p_buffer_data_ty2[2] << 8) + ((unsigned char)p_buffer_data_ty2[3] << 0);

        /* defining a buffer to store the idat data*/
        U8 *idat_data_buf2 = malloc(chunk_p2->length);
        fseek(f2, 41, SEEK_SET);
        /*size = fread(idat_data_buf2, 1, chunk_p2->length, f2);*/
        memset(idat_data_buf2, 0, chunk_p2->length);

        chunk_p2->p_data = idat_data_buf2;

        U8 *idat_data_type2 = malloc(chunk_p2->length + 4);

        for (int i = 0; i < chunk_p2->length + 4; i++)
        {
            if (i == 0 || i == 1 || i == 2 || i == 3)
            {
                idat_data_type2[i] = chunk_p2->type[i];
            }
            else
            {
                idat_data_type2[i] = idat_data_buf2[i - 4];
            }
        }

        /* populating the crc of the chunk - IHDR*/
        fseek(f2, 41 + chunk_p2->length, SEEK_SET);
        /*size = fread(p_buffer_data_ty2, 1, 16, f2);*/
        memset(p_buffer_data_ty2, 0, BUF_LEN);

        chunk_p2->crc = ((unsigned char)p_buffer_data_ty2[0] << 24) + ((unsigned char)p_buffer_data_ty2[1] << 16) + ((unsigned char)p_buffer_data_ty2[2] << 8) + ((unsigned char)p_buffer_data_ty2[3] << 0);
        idat_actual2 = chunk_p2->crc;

        simple_PNG_p2->p_IDAT = chunk_p2;

        /*********** POPULATING IDAT CHUNK- ENDS..........******************************************/

        /* getting computed crc value using the crc function */
        U32 crc_computed_idat2 = crc(idat_data_type2, chunk_p2->length + 4);
        free(idat_data_type2);

        /*********** POPULATING IEND CHUNK- BEGINS..........*****************************************/

        fseek(f2, 41 + chunk_p2->length + 4, SEEK_SET);
        /*size = fread(p_buffer_data_ty2, 1, 12, f2);*/
        memset(p_buffer_data_ty2, 0, BUF_LEN);

        /* populating the type of the chunk - IEND*/
        for (int i = 0; i < 4; i++)
        {
            chunk_p2->type[i] = p_buffer_data_ty2[i + 4];
        }

        /*IEND length*/
        chunk_p2->length = ((unsigned char)p_buffer_data_ty2[0] << 24) + ((unsigned char)p_buffer_data_ty2[1] << 16) + ((unsigned char)p_buffer_data_ty2[2] << 8) + ((unsigned char)p_buffer_data_ty2[3] << 0);

        /* populating the crc of the chunk - IEND*/
        chunk_p2->crc = ((unsigned char)p_buffer_data_ty2[8] << 24) + ((unsigned char)p_buffer_data_ty2[9] << 16) + ((unsigned char)p_buffer_data_ty2[10] << 8) + ((unsigned char)p_buffer_data_ty2[11] << 0);
        iend_actual2 = chunk_p2->crc;

        /*no data field for iend*/
        chunk_p2->p_data = NULL;

        simple_PNG_p2->p_IEND = chunk_p2;

        /****************************************************************************************************************/

        /*checking if the images are saved properly
        //for(int i=0; i<33; i++) {
        // printf("hereeeee");
        // printf("%x ", simple_PNG_p2->p_IHDR->crc);
        // printf("%x ", simple_PNG_p1->p_IHDR->crc);

        //}*/

        if (file_instance_num == 0)
        {
            /* do nothing */
        }
        else
        {
            /* ihdr + idat data + idat crc + iend == len_def + 58*/
            memcpy(p_buffer1, concat_final, len_def + 58);
        }

        /*finding the length of the idat chunk*/
        long idat_leng1 = ((unsigned char)p_buffer1[33] << 24) + ((unsigned char)p_buffer1[34] << 16) + ((unsigned char)p_buffer1[35] << 8) + ((unsigned char)p_buffer1[36] << 0);
        long idat_leng2 = ((unsigned char)p_buffer2[33] << 24) + ((unsigned char)p_buffer2[34] << 16) + ((unsigned char)p_buffer2[35] << 8) + ((unsigned char)p_buffer2[36] << 0);

        /*allocating memory for the data to be stored in the buffer*/
        p_buffer1_data = (unsigned char *)malloc(idat_leng1);
        p_buffer2_data = (unsigned char *)malloc(idat_leng2);

        /*finding the height of the 2 files of interest*/
        int height_f1 = ((unsigned char)p_buffer1[20] << 24) + ((unsigned char)p_buffer1[21] << 16) + ((unsigned char)p_buffer1[22] << 8) + ((unsigned char)p_buffer1[23] << 0);
        int height_f2 = ((unsigned char)p_buffer2[20] << 24) + ((unsigned char)p_buffer2[21] << 16) + ((unsigned char)p_buffer2[22] << 8) + ((unsigned char)p_buffer2[23] << 0);

        /*extracting data part of the ihdr chunk*/
        for (int i = 0; i < idat_leng1; i++)
        {
            p_buffer1_data[i] = p_buffer1[i + 41];
        }

        for (int i = 0; i < idat_leng2; i++)
        {
            p_buffer2_data[i] = p_buffer2[i + 41];
        }

        /*
        
        for(int i=0; i<idat_leng1; i++) {
            printf("%x ", p_buffer1_data[i]);
        }

        for(int i=0; i<idat_leng2; i++) {
            printf("%x ", p_buffer2_data[i]);
        }
        
        
        */

        /*inflation routine */
        ret = mem_inf(data_p_buffer1, &len_inf1, p_buffer1_data, idat_leng1);
        if (ret == 0)
        { /* success */
            /*print nothinggggggggggg*/
        }
        else
        { /* failure */
            fprintf(stderr, "mem_def failed. ret = %d.\n", ret);
        }

        ret = mem_inf(data_p_buffer2, &len_inf2, p_buffer2_data, idat_leng2);
        if (ret == 0)
        { /* success */
            /*printf("original len = %d, len_def = %lu, len_inf = %lu\n", \
               BUF_LEN, idat_leng2, len_inf2);*/
        }
        else
        { /* failure */
            fprintf(stderr, "mem_def failed. ret = %d.\n", ret);
        }

        /* length of the concatenated part would be the sum of the obtained length */
        concatenated_buf = malloc(len_inf1 + len_inf2);

        /*making i long to accomodate the length of the entire parser*/
        for (long i = 0; i < (len_inf1 + len_inf2); i++)
        {
            if (i < len_inf1)
            {
                concatenated_buf[i] = data_p_buffer1[i];
            }
            else
            {
                concatenated_buf[i] = data_p_buffer2[i - len_inf1];
            }
        }

        /*
        
        for(int i=0; i<len_inf1 + len_inf2; i++) {
            printf("%x ", concatenated_buf[i]);
        }
        
        
        */

        /*deflation routine*/
        ret = mem_def(gp_buf_def, &len_def, concatenated_buf, len_inf1 + len_inf2, Z_DEFAULT_COMPRESSION);
        if (ret == 0)
        { /* success */
            /*print nothing plzzzzz*/
        }
        else
        { /* failure */
            fprintf(stderr, "mem_def failed. ret = %d.\n", ret);
            return ret;
        }

        /* storing the final all.png file in concat_final */
        /* setting to 0, our final all.png */
        memset(concat_final, 0x0, (len_def + 16 + 8 + 13 + 12 + 8));
        memcpy(concat_final, p_buffer1, 41); /* fr copying png signature, etc. */

        /* copying the data from the deflated array into the final buffer */
        for (int i = 0; i < len_def; i++)
        {
            concat_final[41 + i] = gp_buf_def[i];
        }

        /*
        
        for(int i=0; i<len_def; i++) {
            printf("%x ", concat_final[41+i]);
        }
        
        
        */

        U8 temp_buf_p[4];
        memset(temp_buf_p, 0, 4);

        /*updating length of the idat block*/
        for (int i = 0; i < 4; i++)
        {
            temp_buf_p[i] = len_def >> (8 * i) & 0xFF;
        }

        /*storing extracted value in the concatinated final buffer*/
        int concati_final_index_tracker = 36;
        for (int i = 0; i < 4; i++)
        {
            concat_final[concati_final_index_tracker] = temp_buf_p[i];
            concati_final_index_tracker--;
        }

        /*don't forget to clear the temppppppppp bufferrrrrr*/
        memset(temp_buf_p, 0, 4);
        /*setting height based on the number of file being processed*/
        if (file_instance_num != 0)
        {
            height_sum = height_sum + height_f2;
        }
        else
        {
            height_sum = height_sum + height_f1 + height_f2;
        }

        //updates height in the ihdr block
        for (int i = 0; i < 4; i++)
        {
            temp_buf_p[i] = height_sum >> (8 * i) & 0xFF;
        }

        int final_counter = 23;
        /*throw in the obtained value in the final buffer for all.png*/
        for (int i = 0; i < 4; i++)
        {
            concat_final[final_counter] = temp_buf_p[i];
            final_counter--;
        }

        /* crc updation for idatt */
        input_buffer1 = malloc(17);
        for (int i = 0; i < 17; i++)
        {
            input_buffer1[i] = concat_final[i + 12];
        }
        /* down cast the return val to U32*/

        crc_val = crc(input_buffer1, 17);

        /*reset temp buf*/
        memset(temp_buf_p, 0, 4);

        for (int i = 0; i < 4; i++)
        {
            temp_buf_p[i] = crc_val >> (8 * i) & 0xFF;
        }

        final_counter = 32;
        /*throw in the obtained value in the final buffer for all.png*/
        for (int i = 0; i < 4; i++)
        {
            concat_final[final_counter] = temp_buf_p[i];
            final_counter--;
        }

        /*processing idat*/
        crc_calc = malloc(len_inf1 + len_inf2 + 4);

        for (int i = 0; i < len_inf1 + len_inf2 + 1; i++)
        {
            crc_calc[i + 4] = concatenated_buf[i];
        }

        int temp_crc = 37;
        for (int i = 0; i < 4; i++)
        {
            crc_calc[i] = p_buffer1[temp_crc];
            temp_crc++;
        }

        U32 crc_val_idat = crc(crc_calc, len_inf1 + len_inf2 + 4);

        memset(temp_buf_p, 0, 4);

        /*update the obtained crc for idat*/
        for (int i = 0; i < 4; i++)
        {
            temp_buf_p[i] = crc_val_idat >> (8 * i) & 0xFF;
        }

        int final_tempp = 44;
        /*throw in the obtained value in the final buffer for all.png*/
        for (int i = 0; i < 4; i++)
        {
            concat_final[len_def + final_tempp] = temp_buf_p[i];
            final_tempp--;
        }

        /*HARDCODE IENDDDD*/
        /* IEND chunk updation - hardcoding since it is always the SAMEEEEE*/
        concat_final[len_def + 49] = 0x49;
        concat_final[len_def + 50] = 0x45;
        concat_final[len_def + 51] = 0x4e;
        concat_final[len_def + 52] = 0x44;
        concat_final[len_def + 53] = 0xae;
        concat_final[len_def + 54] = 0x42;
        concat_final[len_def + 55] = 0x60;
        concat_final[len_def + 56] = 0x82;

        /* create or open all.png file and write the data written in concat_final buffer*/
        FILE *final_fp = fopen("all.png", "wb");
        if (final_fp == NULL)
        {
            printf("Fatal: Failed to write to all.png :(  \n");
            return 1;
        }
        fwrite(concat_final, 1, (len_def + 57), final_fp);
        fclose(final_fp);

        free(simple_PNG_p1);
        free(chunk_p1);
        free(ihdr_data1);
        free(ihdr_data_exclusive1);
        free(idat_data_buf1);
        free(simple_PNG_p2);
        free(chunk_p2);
        free(ihdr_data2);
        free(ihdr_data_exclusive2);
        free(idat_data_buf2);
    }

    /* free memory and resources */
    free(p_buffer_data_ty1);
    free(p_buffer_data_ty2);
    free(p_buffer1);
    free(p_buffer2);
    free(concatenated_buf);
    free(data_p_buffer1);
    free(data_p_buffer2);
    free(input_buffer1);
    free(p_buffer1_data);
    free(p_buffer2_data);
    free(crc_calc);
    free(concat_final);

    return 0;
}
