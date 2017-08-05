/*
* aes-core.h
*
*  Created on: 2016Äê9ÔÂ8ÈÕ
*      Author: qiang.zhang
*/

#ifndef AES_CORE_H_
#define AES_CORE_H_

#ifndef HEADER_AES_LOCL_H
# define HEADER_AES_LOCL_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define AES_MAXNR 14
struct aes_key_st {
	//# ifdef AES_LONG
	unsigned long rd_key[4 * (AES_MAXNR + 1)];
	//# else
	//    unsigned int rd_key[4 * (AES_MAXNR + 1)];
	//# endif
	int rounds;
};

//#define SELF_TEST

extern unsigned char aesivv[20];
extern unsigned char aeskey[40];

typedef unsigned long int       uint32;
typedef struct aes_key_st AES_KEY;

typedef void(*block128_f) (unsigned char in[16],
	unsigned char out[16], void *key);

# undef FULL_UNROLL

#endif
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
#define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ ((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))
#define PUTU32(ct, st) { (ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); (ct)[2] = (u8)((st) >>  8); (ct)[3] = (u8)(st); }

/* only use */
int AES_cbc_encrypt(unsigned char *in, unsigned char *out,
	int len, AES_KEY *key,
	unsigned char *ivec, int enc,int is_4096);
/* only use */

int CRYPTO_cbc128_decrypt(unsigned char *in, unsigned char *out,
	int len, void *key,
	unsigned char ivec[16], block128_f block,int is_4096);
void CRYPTO_cbc128_encrypt(unsigned char *in, unsigned char *out,
	int len, void *key,
	unsigned char ivec[16], block128_f block,int is_4096);
void AES_decrypt(unsigned char *in, unsigned char *out,
	AES_KEY *key);
void AES_encrypt(unsigned char *in, unsigned char *out,
	AES_KEY *key);

/* only use */
int AES_set_decrypt_key(unsigned char *userKey, int bits,
	AES_KEY *key);
int AES_set_encrypt_key(unsigned char *userKey, int bits,
	AES_KEY *key);
int AES_Image_decrypt(unsigned char *key, unsigned char *iv, unsigned char *in, unsigned char *out, int len);
int AES_Image_encrypt(unsigned char *key, unsigned char *iv, unsigned char *in, unsigned char *out, int len);
/* only use */

int AES_base64_encrypt(unsigned char *key, unsigned char *iv, unsigned char *in, unsigned char *out, int len);
int AES_base64_decrypt(unsigned char *key, unsigned char *iv, unsigned char *in, unsigned char *out, int len);

int base64_encode(unsigned char *in,  unsigned long len, unsigned char *out);
int base64_decode(unsigned char *in, unsigned long len, unsigned char *out);

#endif /* AES_CORE_H_ */
