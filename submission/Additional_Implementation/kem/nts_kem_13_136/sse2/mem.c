/**
 *  mem.c
 *  NTS-KEM
 *
 *  Parameter: NTS-KEM(13, 136)
 *
 *  This file is part of the reference implemention of NTS-KEM
 *  submitted as part of NIST Post-Quantum Cryptography
 *  Standardization Process.
 **/

#include "mem.h"

void CT_memset(void *ptr, uint8_t v, size_t len)
{
    volatile uint8_t *p = ptr;
    while (len--) *p++ = v;
}
