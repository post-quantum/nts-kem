/**
 *  random.c
 *  NTS-KEM
 *
 *  Parameter: NTS-KEM(12, 64)
 *  Platform: SSE2/SSE4.1
 *
 *  This file is part of the additional implemention of NTS-KEM
 *  submitted as part of NIST Post-Quantum Cryptography
 *  Standardization Process.
 **/

#include <stdio.h>
#include <string.h>
#include "random.h"
#include "nts_kem_errors.h"

#define PARAM_RND_SIZE      16
#define PARAM_RND_BIT_SIZE  128

int32_t _global_bits_consumed = PARAM_RND_BIT_SIZE;
uint8_t _global_rnd_buffer[PARAM_RND_SIZE];

#if defined(NIST_DRBG_AES)

#include "aes_drbg.h"

#else /* defined(NIST_DRBG_AES) */

#if   defined(_WIN32)
#include <windows.h>
#pragma comment(lib, "advapi32.lib")
#elif defined(__linux) 
#if defined(USE_ARC4RANDOM)
#include <bsd/stdlib.h>
#endif
#else /* BSD system */
#if defined(USE_ARC4RANDOM)
#include <stdlib.h>
#endif
#endif /* defined(_WIN32) */

int randombytes(unsigned char *buffer, unsigned long long buf_len)
{
#if defined(_WIN32)
    HCRYPTPROV hProvider = 0;
    if (!::CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
        return NTS_KEM_RNG_INVALID_PROVIDER;
    if (!::CryptGenRandom(hProvider, buf_len, buffer)) {
        ::CryptReleaseContext(hProvider, 0);
        return NTS_KEM_UNEXPECTED_ERROR;
    }
    if (!::CryptReleaseContext(hProvider, 0)) {
        return NTS_KEM_UNEXPECTED_ERROR;
    }
#else /* defined(_WIN32) */
#if defined(USE_ARC4RANDOM)     /* arc4random */
    arc4random_buf(buffer, buf_len);
#elif defined(USE_DEV_RANDOM)   /* /dev/random */
    FILE *fp = NULL;
    if (!(fp = fopen("/dev/random", "r"))) {
        return NTS_KEM_RNG_INVALID_PROVIDER;
    }
    if (buf_len != fread(buffer, sizeof(uint8_t), buf_len, fp)) {
        return NTS_KEM_RNG_INVALID_OUTPUT_BUFFER;
    }
#else /* default to /dev/urandom */
    FILE *fp = NULL;
    if (!(fp = fopen("/dev/urandom", "r"))) {
        return NTS_KEM_RNG_INVALID_PROVIDER;
    }
    if (buf_len != fread(buffer, sizeof(uint8_t), buf_len, fp)) {
        return NTS_KEM_RNG_INVALID_OUTPUT_BUFFER;
    }
#endif /* defined(USE_ARC4RANDOM) */
#endif /* defined(_WIN32) */
    return NTS_KEM_SUCCESS;
}

void randombytes_init(const unsigned char* entropy_input,
                      const unsigned char* personalization_string,
                      int security_strength)
{
    /* A place-holder, not doing anything unless it's NIST AES-DRBG */
}

#endif /* defined(NIST_DRBG_AES) */

uint16_t random_uint16_bounded(uint16_t bound)
{
    uint16_t d, u, x;
    
    /* Knuth-Yao DDG */
    d = 0; u = 1; x = 0;
    do {
        while (u < bound) {
            u = 2*u;
            x = 2*x + randombit();
        }
        d = u - bound;
        u = d;
    } while (x < d);
    
    return x - d;
}

uint8_t randombit()
{
    uint8_t b = 0;
    
    /**
     * Have we depleted our random source?
     **/
    if (_global_bits_consumed >= PARAM_RND_BIT_SIZE) {
        /**
         * If so, generate PARAM_RND_SIZE bytes
         * of random data as our random source
         **/
        randombytes(_global_rnd_buffer, sizeof(_global_rnd_buffer));
        _global_bits_consumed = 0;
    }
    
    b = (_global_rnd_buffer[_global_bits_consumed >> 3] & (1 << (_global_bits_consumed & 7))) >> (_global_bits_consumed & 7);
    _global_bits_consumed++;
    
    return b;
}

void random_reset()
{
    _global_bits_consumed = PARAM_RND_BIT_SIZE;
    memset(_global_rnd_buffer, 0, sizeof(_global_rnd_buffer));
}