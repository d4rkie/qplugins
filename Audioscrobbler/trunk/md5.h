#ifndef __MD5_H
#define __MD5_H

void md5(unsigned char *output, const unsigned char *input);
void md5_32(char* strDest, const unsigned char *input);
void md5_convert_to_ascii(char* strDest, const unsigned char md5Digest[16]);

#endif