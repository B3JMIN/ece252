

#pragma once

void checkForIHDR(FILE *fp, U32 buffer_4, char *fileName);
void checkForIDAT(FILE *fp, U32 buffer_4);
void checkForCRC(FILE *fp, U32 buffer_4);