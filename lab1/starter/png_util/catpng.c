#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "lab_png.h"
#include "zutil.h"
#include "crc.h"

#define PNG_MAX_SIZE (16 * 1024 * 1024)
#define LARGE_BUF (32 * 256)

U8 gp_buf_def[PNG_MAX_SIZE];
U8 gp_buf_inf[LARGE_BUF];

void initData(U8 *buf, int len);

void initData(U8 *buf, int len)
{
    int i;
    for (int i = 0; i < len; i++)
    {
        buf[i] = i % 256;
    }
}
int main(int argc, char *argv)
{
    FILE *fp;
    U8 buffer_4[8];
    for (int i = 0; i < argc - 1; i++)
    {
        fp = (i == 1) ? fopen(argv[i], "r") : fopen("all.png", "r");
        struct data_IHDR ihdr = {.width = 0, .height = 0, .bit_depth = 0, .color_type = 0, .compression = 0, .filter = 0, .interlace = 0};
        data_IHDR_p out = &ihdr;
        get_png_data_IHDR(out, fp, 0, 0);
        int width1 = get_png_width(out);
        int height1 = get_png_height(out);
        fseek(fp, 12, SEEK_SET);
        U8 ihdr_buffer_without_length[17];
        fread(ihdr_buffer_without_length, 17, 1, fp);

        fseek(fp, 8, SEEK_SET);
        U8 ihdr_buffer[21];
        fread(ihdr_buffer, 21, 1, fp);

        fseek(fp, -12, SEEK_END);
        U8 iend_buffer[12];
        fread(iend_buffer, 12, 1, fp);

        fseek(fp, 33, SEEK_SET);
        fread(buffer_4, 4, 1, fp);
        U32 *pInt = (U32 *)buffer_4;
        unsigned long length1 = htonl(*pInt);
        fseek(fp, 41, SEEK_SET);
        U8 *idat_buffer_data1 = malloc(length1 * sizeof(U8));
        fread(idat_buffer_data1, length1, 1, fp);

        fseek(fp, 37, SEEK_SET);
        U8 *type_buffer = malloc((4) * sizeof(U8));
        fread(type_buffer, 4, 1, fp);

        int size1 = height1 * (width1 * 4 + 1);
        U8 *gp_buf_inf1 = malloc((size1) * sizeof(U8));
        U64 len_inf1 = size1;
        mem_inf(gp_buf_inf1, &len_inf1, idat_buffer_data1, length1);
        fclose(fp);

        fp = fopen(argv[i + 1], "r");
        get_png_data_IHDR(out, fp, 0, 0);
        int width2 = get_png_width(out);
        int height2 = get_png_height(out);

        fseek(fp, 33, SEEK_SET);
        fread(buffer_4, 4, 1, fp);
        U32 *pInt2 = (U32 *)buffer_4;
        unsigned long length2 = htonl(*pInt2);
        fseek(fp, 41, SEEK_SET);
        U8 *idat_buffer_data2 = malloc(length2 * sizeof(U8));
        fread(idat_buffer_data2, length2, 1, fp);
        int size2 = height2 * (width2 * 4 + 1);
        U8 *gp_buf_inf2 = malloc((size2) * sizeof(U8));
        U64 len_inf2 = size2;
        mem_inf(gp_buf_inf2, &len_inf2, idat_buffer_data2, length2);
        fclose(fp);

        U32 height_sum = height1 + height2;
        U32 height_sum_convert = ntohl(height_sum);

        for (int i = 0; i < 4; ++i)
        {
            ihdr_buffer[15 - i] = height_sum_convert >> (3 - i) * 8;
            ihdr_buffer_without_length[11 - i] = height_sum_convert >> (3 - i) * 8;
        }

        unsigned long ihdr_crc = crc(ihdr_buffer_without_length, 17);
        unsigned long ihdr_crc_convert = ntohl(ihdr_crc);

        U8 ihdr_crc_convert_pointer[4];

        for (int i = 0; i < 4; ++i)
        {
            ihdr_crc_convert_pointer[3 - i] = ihdr_crc_convert >> ((3 - i) * 8);
        }

        U8 *gp_buf_inf_sum = malloc((size1 + size2) * sizeof(U8));

        for (int i = 0; i < size1 + size2; i++)
        {
            if (i < size1)
            {
                gp_buf_inf_sum[i] = gp_buf_inf1[i];
            }
            else
            {
                gp_buf_inf_sum[i] = gp_buf_inf2[i - size1];
            }
        }

        U8 *gp_buf_def = malloc((size1 + size2) * sizeof(U8));

        U64 len_def = size1 + size2;

        mem_def(gp_buf_def, &len_def, gp_buf_inf_sum, size1 + size2, Z_DEFAULT_COMPRESSION);

        unsigned long idat_length_convert = ntohl(len_def);

        U8 idat_length_convert_pointer[4];

        for (int i = 0; i < 4; ++i)
        {
            idat_length_convert_pointer[3 - i] = idat_length_convert >> ((3 - i) * 8);
        }

        U8 *idat_buffer_without_length = malloc((len_def + 4) * sizeof(U8));

        for (int i = 0; i < len_def + 4; i++)
        {
            if (i < 4)
            {
                idat_buffer_without_length[i] = type_buffer[i];
            }
            else
            {
                idat_buffer_without_length[i] = gp_buf_def[i - 4];
            }
        }

        unsigned long idat_crc = crc(idat_buffer_without_length, len_def + 4);

        unsigned long idat_crc_convert = ntohl(idat_crc);

        U8 idat_crc_convert_pointer[4];

        for (int i = 0; i < 4; ++i)
        {
            idat_crc_convert_pointer[3 - i] = idat_crc_convert >> ((3 - i) * 8);
        }

        FILE *fp_png;
        fp_png = fopen("all.png", "w");
        fwrite(PNG_SIG_DATA, sizeof(PNG_SIG_DATA[0]), 8, fp_png);
        fwrite(ihdr_buffer, sizeof(ihdr_buffer[0]), 21, fp_png);
        fwrite(ihdr_crc_convert_pointer, sizeof(ihdr_crc_convert_pointer[0]), 4, fp_png);
        fwrite(idat_length_convert_pointer, sizeof(idat_length_convert_pointer[0]), 4, fp_png);
        fwrite(idat_buffer_without_length, sizeof(idat_buffer_without_length[0]), len_def + 4, fp_png);
        fwrite(idat_crc_convert_pointer, sizeof(idat_crc_convert_pointer[0]), 4, fp_png);
        fwrite(iend_buffer, sizeof(iend_buffer[0]), 12, fp_png);
        fclose(fp_png);

        free(idat_buffer_data1);
        free(gp_buf_inf1);
        free(idat_buffer_data2);
        free(gp_buf_inf2);
        free(gp_buf_inf_sum);
        free(gp_buf_def);
        free(idat_buffer_without_length);
        free(type_buffer);
    }

    return 0;
}