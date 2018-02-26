/**
 *
 *  bitslice_bma_128.c
 *  NTS-KEM
 *
 *  Parameter: NTS-KEM(13, 80)
 *  Platform: SSE2/SSE4.1
 *
 *  This file is part of the additional implemention of NTS-KEM
 *  submitted as part of NIST Post-Quantum Cryptography
 *  Standardization Process.
 **/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bits.h"
#include "bitslice_bma_128.h"
#include "vector_utils.h"

#define PARAM_M         13
#define PARAM_T         80

extern void bitslice_mul13_128(__m128i* d, const __m128i* a, const __m128i* b);

static inline uint64_t MUX(uint64_t ctl, uint64_t a, uint64_t b)
{
    return b ^ (-ctl & (a ^ b));
}

static inline uint64_t EQ0(int64_t a)
{
    uint64_t b = (uint64_t)a;
    return ~(b | -b) >> 63;
}

static inline uint64_t LT(uint64_t a, uint64_t b)
{
    uint64_t c = a - b;
    return (c ^ ((a ^ b) & (b ^ c))) >> 63;
}

static inline __m128i vMUX(uint64_t ctl, __m128i a, __m128i b)
{
    return _mm_xor_si128(b,
                         _mm_and_si128(_mm_set1_epi64x(-ctl),
                                       _mm_xor_si128(a, b)));
}

static inline uint32_t vector_popcount_128(__m128i a)
{
    const __m128i a_hi = _mm_unpackhi_epi64(a, a);
    return popcount(_mm_cvtsi128_si64(a_hi)) + popcount(_mm_cvtsi128_si64(a));
}

static inline __m128i shift_left(__m128i a, int cnt)
{
    __m128i a1, a2;
    
    a1 = _mm_slli_epi64(a, cnt);
    a2 = _mm_slli_si128(a, 8);
    a2 = _mm_srli_epi64(a2, 64 - cnt);
    a1 = _mm_or_si128(a1, a2);
    
    return a1;
}

static inline __m128i shift_right(__m128i a, int cnt)
{
    __m128i a1, a2;
    
    a1 = _mm_srli_epi64(a, cnt);
    a2 = _mm_srli_si128(a, 8);
    a2 = _mm_slli_epi64(a2, 64 - cnt);
    a1 = _mm_or_si128(a1, a2);
    
    return a1;
}

static inline __m128i bit_reverse(__m128i x)
{
    __m128i a0, a1;
    a0 = _mm_and_si128(x, _mm_set_epi64x(0x5555555555555555ULL, 0x5555555555555555ULL));
    a1 = _mm_and_si128(x, _mm_set_epi64x(0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL));
    x = _mm_or_si128(shift_left(a0, 1), shift_right(a1, 1));
    a0 = _mm_and_si128(x, _mm_set_epi64x(0x3333333333333333ULL, 0x3333333333333333ULL));
    a1 = _mm_and_si128(x, _mm_set_epi64x(0xCCCCCCCCCCCCCCCCULL, 0xCCCCCCCCCCCCCCCCULL));
    x = _mm_or_si128(shift_left(a0, 2), shift_right(a1, 2));
    a0 = _mm_and_si128(x, _mm_set_epi64x(0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL));
    a1 = _mm_and_si128(x, _mm_set_epi64x(0xF0F0F0F0F0F0F0F0ULL, 0xF0F0F0F0F0F0F0F0ULL));
    x = _mm_or_si128(shift_left(a0, 4), shift_right(a1, 4));
    return _mm_shuffle_epi8(x, _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
}

void bitslice_bma(__m128i (*out)[13], __m128i s[][PARAM_M], int *xi)
{
    int32_t i, j, c;
    int64_t R = 0LL, L = 0LL;
    uint64_t _d = 0, d_eq_0, control;
    __m128i sigma[PARAM_M] = {{0}}, beta[PARAM_M] = {{0}};
    __m128i d[PARAM_M], delta[PARAM_M] = {{0}};
    __m128i psi[PARAM_M] = {{0}}, tmp[PARAM_M];
    __m128i ss[2][PARAM_M] = {{{0}}};
    
    /* Initialise sigma and beta */
    sigma[0] = _mm_set_epi64x(0ULL, 1ULL);
    beta [0] = _mm_set_epi64x(0ULL, 2ULL);
    delta[0] = _mm_set1_epi64x(-1);
    psi  [0] = sigma[0];
    
    *xi = 0;
    for (j=0; j<PARAM_M; j++)
        ss[0][j][0] = s[0][j][0] & 1ULL;
    for (i=0; i<63; i++) {
        bitslice_mul13_128(d, sigma, ss[0]);
        for (_d=0,j=0; j<PARAM_M; j++) {
            c = (vector_popcount_128(d[j]) & 1);
            d[j] = _mm_set1_epi64x((long long)-c);
            _d |= c;
        }
        
        d_eq_0 = EQ0(_d);
        control = d_eq_0 || LT(i, L<<1);
        L = MUX(control, L, i-L+1);
        R = MUX(control, R + d_eq_0, 0);
        
        bitslice_mul13_128(tmp, delta, sigma);
        bitslice_mul13_128(psi, d, beta);
        
        for (j=0; j<PARAM_M; j++) {
            beta[j][0] = MUX(control, beta[j][0], sigma[j][0]) << 1;
            psi[j] = _mm_xor_si128(tmp[j], psi[j]);
            delta[j][0] = MUX(control, delta[j][0], d[j][0]);
            sigma[j][0] = psi[j][0];
            ss[0][j] = shift_left(ss[0][j], 1);
            ss[0][j][0] |= ((s[0][j][0] & (1ULL << (i+1))) >> (i+1));
        }
    }
    for (;i<127; i++) {
        bitslice_mul13_128(d, sigma, ss[0]);
        for (_d=0,j=0; j<PARAM_M; j++) {
            c = (vector_popcount_128(d[j]) & 1);
            d[j] = _mm_set1_epi64x((long long)-c);
            _d |= c;
        }
        
        d_eq_0 = EQ0(_d);
        control = d_eq_0 || LT(i, L<<1);
        L = MUX(control, L, i-L+1);
        R = MUX(control, R + d_eq_0, 0);
        
        bitslice_mul13_128(tmp, delta, sigma);
        bitslice_mul13_128(psi, d, beta);
        
        for (j=0; j<PARAM_M; j++) {
            beta[j] = vMUX(control, beta[j], sigma[j]);
            beta[j] = shift_left(beta[j], 1);
            psi[j] = _mm_xor_si128(tmp[j], psi[j]);
            delta[j] = vMUX(control, delta[j], d[j]);
            
            sigma[j][0] = psi[j][0]; sigma[j][1] = psi[j][1];
            ss[0][j] = shift_left(ss[0][j], 1);
            ss[0][j][0] |= ((s[0][j][1] & (1ULL << (i-63))) >> (i-63));
        }
    }
    for (;i<2*PARAM_T-1; i++) {
        bitslice_mul13_128(d, sigma, ss[0]);
        for (_d=0,j=0; j<PARAM_M; j++) {
            c = (vector_popcount_128(d[j]) & 1);
            d[j] = _mm_set1_epi64x((long long)-c);
            _d |= c;
        }
        
        d_eq_0 = EQ0(_d);
        control = d_eq_0 || LT(i, L<<1);
        L = MUX(control, L, i-L+1);
        R = MUX(control, R + d_eq_0, 0);
        
        bitslice_mul13_128(tmp, delta, sigma);
        bitslice_mul13_128(psi, d, beta);
        
        for (j=0; j<PARAM_M; j++) {
            beta[j] = vMUX(control, beta[j], sigma[j]);
            beta[j] = shift_left(beta[j], 1);
            psi[j] = _mm_xor_si128(tmp[j], psi[j]);
            delta[j] = vMUX(control, delta[j], d[j]);
            
            sigma[j][0] = psi[j][0]; sigma[j][1] = psi[j][1];
            ss[0][j] = shift_left(ss[0][j], 1);
            ss[0][j][0] |= ((s[1][j][0] & (1ULL << (i-127))) >> (i-127));
        }
    }
    bitslice_mul13_128(d, sigma, ss[0]);
    for (_d=0,j=0; j<PARAM_M; j++) {
        c = (vector_popcount_128(d[j]) & 1);
        d[j] = _mm_set1_epi64x((long long)-c);
    }
    
    bitslice_mul13_128(sigma, d, beta);
    bitslice_mul13_128(tmp, delta, psi);
    for (_d=0,j=0; j<PARAM_M; j++) {
        psi[j] = _mm_xor_si128(tmp[j], sigma[j]);
        _d |= psi[j][1];
    }
    __asm__ __volatile__ ("bsrq %1, %q0" : "=r" (L) : "rm" (_d));
    *xi = CT_is_less_than(64+((int32_t)L), (int32_t)(PARAM_T - (R>>1)));
    for (j=0; j<PARAM_M; j++) {
        out[0][j] = shift_right(bit_reverse(psi[j]), 127-PARAM_T+(*xi));
    }
}
