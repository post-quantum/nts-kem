/**
 *  mem.h
 *  NTS-KEM
 *
 *  Parameter: NTS-KEM(13, 80)
 *
 *  This file is part of the reference implemention of NTS-KEM
 *  submitted as part of NIST Post-Quantum Cryptography
 *  Standardization Process.
 **/

#ifndef __NTSKEM_MEM_H
#define __NTSKEM_MEM_H

#include <stdint.h>
#include <stddef.h>

void CT_memset(void *ptr, uint8_t v, size_t len);

#endif /* __NTSKEM_MEM_H */
