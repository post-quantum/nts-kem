/**
 *  ntskem_test.c
 *  NTS-KEM test
 *
 *  Parameter: NTS-KEM(13, 80)
 *  Platform: Intel 64-bit
 *
 *  This file is part of the optimized implemention of NTS-KEM
 *  submitted as part of NIST Post-Quantum Cryptography
 *  Standardization Process.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"
#include "ntskem_test.h"
#include "random.h"

#undef DETERMINISTIC

#ifdef DETERMINISTIC
uint8_t* hexstr_to_char(const char* hexstr, int32_t *size)
{
    int i, j;
    uint8_t* buffer = NULL;
    size_t len = strlen(hexstr);
    
    *size = 0;
    if (len & 1)
        return NULL;
    len >>= 1;
    
    if (!(buffer = (unsigned char*)malloc((len+1) * sizeof(uint8_t))))
        return NULL;
    
    for (i=0, j=0; j<len; i+=2, j++)
        buffer[j] = ((((hexstr[i] & 31) + 9) % 25) << 4) + ((hexstr[i+1] & 31) + 9) % 25;
    buffer[len] = '\0';
    *size = (int32_t)len;
    
    return buffer;
}
#endif

int testkem_nts(int iterations)
{
    int i, it = 0, status = 1;
    uint8_t *pk, *sk;
    uint8_t *encap_key, *decap_key, *ciphertext;
#if !defined(DETERMINISTIC)
    FILE *fp = NULL;
#endif
    unsigned char *nonce = NULL;
    int32_t nonce_size = 48;

    unsigned char entropy_input[] = {
        0x8a, 0x94, 0x42, 0x02, 0xc2, 0x7f, 0xf7, 0x28,
        0xc8, 0xa5, 0x8e, 0x72, 0x21, 0x7b, 0x1e, 0x5f,
        0x06, 0x3e, 0x57, 0x15, 0x5f, 0x83, 0xb4, 0x15,
        0x90, 0xae, 0x15, 0xff, 0x96, 0x1a, 0x77, 0x07,
        0x2e, 0xd2, 0x3a, 0x83, 0x22, 0x9f, 0xbe, 0x6d,
        0x94, 0x5a, 0xba, 0x18, 0x00, 0x00, 0x00, 0x00
    };

    fprintf(stdout, "NTS-KEM(%d, %d) Test\n", NTSKEM_M, NTSKEM_T);

    do {
#if defined(DETERMINISTIC)
        const char* nonce_str = "06da3ca3a77433e4dca45d64b148bb4c0dfb3e3a4127472f43cd71ab332d17c8a7c42dd8b1b14b49d82dcaf77a566290";
        nonce = hexstr_to_char(nonce_str, &nonce_size);
        // memset(nonce, 0, sizeof(nonce));
#else
        nonce = (unsigned char*)calloc(sizeof(unsigned char), nonce_size);
        if ((fp = fopen("/dev/urandom", "r"))) {
            if (nonce_size != fread(nonce, 1, nonce_size, fp)) {
                status = 0;
                break;
            }
        }
        fclose(fp);
#endif
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		i = ( (((uint32_t)it) << 24) & 0xFF000000 ) |
		    ( (((uint32_t)it) <<  8) & 0x00FF0000 ) |
		    ( (((uint32_t)it) >>  8) & 0x0000FF00 ) |
		    ( (((uint32_t)it) >> 24) & 0x000000FF );
        memcpy(&entropy_input[48-sizeof(i)], &i, sizeof(i));
#else
        memcpy(&entropy_input[48-sizeof(it)], &it, sizeof(it));
#endif
        
        fprintf(stdout, "Iteration: %d, Seed: ", it);
        for (i=0; i<nonce_size; i++) fprintf(stdout, "%02x", entropy_input[i]);
        fprintf(stdout, ", "); fflush(stdout);
        fprintf(stdout, "Nonce: "); for (i=0; i<nonce_size; i++) fprintf(stdout, "%02x", nonce[i]);
        fprintf(stdout, "\n");

        randombytes_init(entropy_input, nonce, 256);
        random_reset();

        pk = (uint8_t *)calloc(CRYPTO_PUBLICKEYBYTES, sizeof(uint8_t));
        sk = (uint8_t *)calloc(CRYPTO_SECRETKEYBYTES, sizeof(uint8_t));
        if (crypto_kem_keypair(pk, sk))
            status = 0;
        
        ciphertext = (uint8_t *)calloc(CRYPTO_CIPHERTEXTBYTES, sizeof(uint8_t));
        encap_key = (uint8_t *)calloc(CRYPTO_BYTES, sizeof(uint8_t));
        decap_key = (uint8_t *)calloc(CRYPTO_BYTES, sizeof(uint8_t));

        if (crypto_kem_enc(ciphertext, encap_key, pk))
            status = 0;
        
        if (crypto_kem_dec(decap_key, ciphertext, sk))
            status = 0;
        
        status &= (0 == memcmp(encap_key, decap_key, CRYPTO_BYTES));
        
        free(decap_key);
        free(encap_key);
        free(ciphertext);
        free(sk);
        free(pk);
        if (nonce) free(nonce);
    }
    while (status && ++it < iterations);

    return status;
}
