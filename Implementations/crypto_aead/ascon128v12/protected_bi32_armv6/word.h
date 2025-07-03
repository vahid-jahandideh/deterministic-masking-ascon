#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>
#include <string.h>

#include "asm.h"
#include "config.h"
#include "endian.h"
#include "forceinline.h"
#include "interleave.h"
#include "shares.h"

typedef struct {
  uint32_t w[2];
} share_t;

typedef struct {
  share_t s[NUM_SHARES_KEY];
} word_t;

forceinline uint32_t ROR32(uint32_t x, int n) {
  return x >> n | x << (-n & 31);
}

forceinline uint64_t ROR32x2(uint64_t x, int n) {
  uint32_t lo = x;
  uint32_t hi = x >> 32;
  lo = ROR32(lo, n);
  hi = ROR32(hi, n);
  return (uint64_t)hi << 32 | lo;
}

forceinline uint64_t ROR64(uint64_t x, int n) {
  return x >> n | x << (-n & 63);
}

forceinline word_t MXOR(word_t a, word_t b, int ns) {
  if (ns >= 1) a.s[0].w[0] ^= b.s[0].w[0];
  if (ns >= 1) a.s[0].w[1] ^= b.s[0].w[1];
  if (ns >= 2) a.s[1].w[0] ^= b.s[1].w[0];
  if (ns >= 2) a.s[1].w[1] ^= b.s[1].w[1];
  if (ns >= 3) a.s[2].w[0] ^= b.s[2].w[0];
  if (ns >= 3) a.s[2].w[1] ^= b.s[2].w[1];
  if (ns >= 4) a.s[3].w[0] ^= b.s[3].w[0];
  if (ns >= 4) a.s[3].w[1] ^= b.s[3].w[1];
  return a;
}

forceinline word_t MXORBIC1(word_t o, word_t a, word_t b, word_t c, int ns) {
  uint32_t tmp;
    
  if (ns == 1) {
    #pragma GCC unroll 2
    for (int i=0; i<2; i++){
    o.s[0].w[i] = a.s[0].w[i];
    EOR_BIC_ROR(o.s[0].w[i], c.s[0].w[i], b.s[0].w[i], 0, tmp);
    }
    return o;  
  }
  if (ns == 2) {
    /* 
    A'0 = C0B0 ⊕ A0 ⊕ C0B1 ⊕ C0,
    A'1 = C1B1 ⊕ A1 ⊕ C1B0 ⊕ C1. 
    */
   
    #pragma GCC unroll 2
    for (int i=0; i<2; i++){
    EOR_AND_ROR(o.s[0].w[i], b.s[0].w[i], c.s[0].w[i], 0, tmp);
    EOR_ROR    (o.s[0].w[i], o.s[0].w[i], a.s[0].w[i], ROT(0) );
    EOR_AND_ROR(o.s[0].w[i], c.s[0].w[i], b.s[1].w[i], 1 - 0, tmp); 
    EOR_ROR    (o.s[0].w[i], o.s[0].w[i], c.s[0].w[i], ROT(0) );
    }
    
    #pragma GCC unroll 2
    for (int i=0; i<2; i++){
    EOR_ROR    (o.s[1].w[i], o.s[1].w[i], a.s[1].w[i], ROT(0) );  
    EOR_AND_ROR(o.s[1].w[i], b.s[1].w[i], c.s[1].w[i], 1 - 1, tmp);      
    EOR_AND_ROR(o.s[1].w[i], c.s[1].w[i], b.s[0].w[i], 0 - 1, tmp);  
    EOR_ROR    (o.s[1].w[i], o.s[1].w[i], c.s[1].w[i], ROT(0) ); 
    }  
    return o;
  }
  if (ns == 3) {
    /* A'0 = C0B0 ⊕   C1 ⊕ C1B0 ⊕ C2 ⊕ A0 ⊕ C2B0,
       A'1 = C0   ⊕ C0B1 ⊕ C1B1 ⊕ C2 ⊕ A1 ⊕ C2B1,    
       A'2 = A2   ⊕ C0B2 ⊕ C1B2 ⊕ C2 ⊕ C2B2.    
    */  
    #pragma GCC unroll 2
    for (int i=0; i<2; i++){  
    EOR_AND_ROR(o.s[0].w[i], c.s[0].w[i], b.s[0].w[i], 0 -  0, tmp);
    EOR_ROR    (o.s[0].w[i], o.s[0].w[i], c.s[1].w[i], ROT(1));    
    EOR_AND_ROR(o.s[0].w[i], b.s[0].w[i], c.s[1].w[i], 1, tmp);
    EOR_ROR    (o.s[0].w[i], o.s[0].w[i], c.s[2].w[i], ROT(2));
    EOR_ROR    (o.s[0].w[i], o.s[0].w[i], a.s[0].w[i], ROT(0));
    EOR_AND_ROR(o.s[0].w[i], b.s[0].w[i], c.s[2].w[i], 2 - 0, tmp);

    EOR_ROR    (o.s[1].w[i], o.s[1].w[i], c.s[0].w[i], ROT(-1));
    EOR_AND_ROR(o.s[1].w[i], b.s[1].w[i], c.s[0].w[i], 0 -  1, tmp);
    EOR_AND_ROR(o.s[1].w[i], c.s[1].w[i], b.s[1].w[i], 1 -  1, tmp);
    EOR_ROR    (o.s[1].w[i], o.s[1].w[i], c.s[2].w[i], ROT(1));
    EOR_ROR    (o.s[1].w[i], o.s[1].w[i], a.s[1].w[i], ROT(0));
    EOR_AND_ROR(o.s[1].w[i], b.s[1].w[i], c.s[2].w[i], 2 -  1, tmp);

    EOR_ROR    (o.s[2].w[i], o.s[2].w[i], a.s[2].w[i], ROT(0));
    EOR_AND_ROR(o.s[2].w[i], b.s[2].w[i], c.s[0].w[i], 0 -  2, tmp);
    EOR_AND_ROR(o.s[2].w[i], b.s[2].w[i], c.s[1].w[i], 1 -  2, tmp);
    EOR_ROR    (o.s[2].w[i], o.s[2].w[i], c.s[2].w[i], ROT(0));
    EOR_AND_ROR(o.s[2].w[i], b.s[2].w[i], c.s[2].w[i], 2 -  2, tmp);
    }
    return o;
  }
  return o;
}

forceinline word_t MXORBIC2(word_t o, word_t b, word_t c, word_t d, int ns) {  
 uint32_t tmp;
 
 if (ns == 1) {
  #pragma GCC unroll 2
  for (int i=0; i<2; i++){
   o.s[0].w[i] = b.s[0].w[i];
   EOR_BIC_ROR(o.s[0].w[i], d.s[0].w[i], c.s[0].w[i], 0, tmp); 
  } 
   return o; 
 }
 if (ns == 2) {
/*
B'0 = D0C0 ⊕ B0 ⊕ D0C1 ⊕ D0, 
B'1 = D1C1 ⊕ D1 ⊕ B1   ⊕ D1C0.
*/
  #pragma GCC unroll 2
  for (int i=0; i<2; i++){
   EOR_AND_ROR(o.s[0].w[i], d.s[0].w[i], c.s[0].w[i], 0, tmp);
   EOR_ROR    (o.s[0].w[i], o.s[0].w[i], b.s[0].w[i], ROT(0) );
   EOR_ROR    (o.s[0].w[i], o.s[0].w[i], d.s[0].w[i], ROT(0) );
   EOR_AND_ROR(o.s[0].w[i], d.s[0].w[i], c.s[1].w[i], 1 - 0, tmp); 
   }   
   #pragma GCC unroll 2
   for (int i=0; i<2; i++){
   EOR_AND_ROR(o.s[1].w[i], c.s[1].w[i], d.s[1].w[i], 1 - 1, tmp);
   EOR_ROR    (o.s[1].w[i], o.s[1].w[i], d.s[1].w[i], ROT(0) );  
   EOR_ROR    (o.s[1].w[i], o.s[1].w[i], b.s[1].w[i], ROT(0) );   
   EOR_AND_ROR(o.s[1].w[i], d.s[1].w[i], c.s[0].w[i], 0 - 1, tmp);
   }  
   return o; 
  }
  
 if (ns == 3) {
 /*
 B′0 = D0 ⊕ D0C0 ⊕ D1C0 ⊕ B2 ⊕ D2C0, 
 B′1 = D0 ⊕ D0C1 ⊕ D1C1 ⊕ D2 ⊕ B0   ⊕ D2C1, 
 B′2 = D0 ⊕ B1   ⊕ D0C2 ⊕ D1 ⊕ D1C2 ⊕ D2C2.
 */
   #pragma GCC unroll 2
   for (int i=0; i<2; i++){
   EOR_ROR    (o.s[0].w[i], o.s[0].w[i], d.s[0].w[i],  ROT(0));   
   EOR_AND_ROR(o.s[0].w[i], d.s[0].w[i], c.s[0].w[i],  0 - 0, tmp);
   EOR_AND_ROR(o.s[0].w[i], c.s[0].w[i], d.s[1].w[i],  1 - 0, tmp);
   EOR_ROR    (o.s[0].w[i], o.s[0].w[i], b.s[2].w[i],  ROT(2));
   EOR_AND_ROR(o.s[0].w[i], c.s[0].w[i], d.s[2].w[i],  2 - 0, tmp);

   EOR_ROR    (o.s[1].w[i], o.s[1].w[i], d.s[0].w[i],  ROT(-1));
   EOR_AND_ROR(o.s[1].w[i], c.s[1].w[i], d.s[0].w[i],  0 -  1, tmp);
   EOR_AND_ROR(o.s[1].w[i], d.s[1].w[i], c.s[1].w[i],  1 -  1, tmp);
   EOR_ROR    (o.s[1].w[i], o.s[1].w[i], d.s[2].w[i],  ROT(1));
   EOR_ROR    (o.s[1].w[i], o.s[1].w[i], b.s[0].w[i],  ROT(-1));
   EOR_AND_ROR(o.s[1].w[i], c.s[1].w[i], d.s[2].w[i],  2 -  1, tmp);
       
   EOR_ROR    (o.s[2].w[i], o.s[2].w[i], d.s[0].w[i], ROT(-2));
   EOR_ROR    (o.s[2].w[i], o.s[2].w[i], b.s[1].w[i], ROT(-1));  
   EOR_AND_ROR(o.s[2].w[i], c.s[2].w[i], d.s[0].w[i], 0 -  2, tmp);
   EOR_ROR    (o.s[2].w[i], o.s[2].w[i], d.s[1].w[i], ROT(-1));
   EOR_AND_ROR(o.s[2].w[i], c.s[2].w[i], d.s[1].w[i], 1 -  2, tmp);
   EOR_AND_ROR(o.s[2].w[i], d.s[2].w[i], c.s[2].w[i], 2 -  2, tmp);
   }
   return o;
 }
 return o;
}

forceinline word_t MXORBIC3(word_t o, word_t c, word_t d, word_t e, int ns) {
uint32_t tmp;

if (ns == 1) {
  #pragma GCC unroll 2
  for (int i=0; i<2; i++){
  o.s[0].w[i] = c.s[0].w[i];
  EOR_BIC_ROR(o.s[0].w[i], e.s[0].w[i], d.s[0].w[i], 0, tmp);  
  }
  return o;
}
if (ns == 2) {
 /*
 C'0 = E0D1 ⊕ E0 ⊕ C0 ⊕ E0D0, 
 C'1 = E1D0 ⊕ E1 ⊕ C1 ⊕ E1D1.
 */	

  #pragma GCC unroll 2
  for (int i=0; i<2; i++){
  EOR_AND_ROR(o.s[0].w[i], e.s[0].w[i], d.s[1].w[i], 1, tmp);
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], e.s[0].w[i], ROT(0) );  
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], c.s[0].w[i], ROT(0) );
  EOR_AND_ROR(o.s[0].w[i], e.s[0].w[i], d.s[0].w[i], 0, tmp);    
  }
  
  #pragma GCC unroll 2
  for (int i=0; i<2; i++){
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], c.s[1].w[i], ROT(0) );  
  EOR_AND_ROR(o.s[1].w[i], e.s[1].w[i], d.s[0].w[i], -1, tmp);
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], e.s[1].w[i], ROT(0) );  
  EOR_AND_ROR(o.s[1].w[i], e.s[1].w[i], d.s[1].w[i], 0, tmp); 
  } 
  return o;
}
if (ns == 3) {
/*
 C'0 = E0 ⊕ C2   ⊕ E0D0 ⊕ E1D0 ⊕ E2 ⊕ E2D0,
 C'1 = E0 ⊕ E0D1 ⊕ E1D1 ⊕ C0   ⊕ E2D1,
 C'2 = E0 ⊕ C1   ⊕ E0D2 ⊕ E1   ⊕ E1D2 ⊕ E2D2.
 */
  #pragma GCC unroll 2
  for (int i=0; i<2; i++){
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], e.s[0].w[i], ROT(0));  
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], c.s[2].w[i], ROT(2));     
  EOR_AND_ROR(o.s[0].w[i], e.s[0].w[i], d.s[0].w[i], 0 - 0, tmp);
  EOR_AND_ROR(o.s[0].w[i], d.s[0].w[i], e.s[1].w[i], 1 - 0, tmp);
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], e.s[2].w[i], ROT(2));
  EOR_AND_ROR(o.s[0].w[i], d.s[0].w[i], e.s[2].w[i], 2 - 0, tmp);

  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], e.s[0].w[i], ROT(-1));
  EOR_AND_ROR(o.s[1].w[i], d.s[1].w[i], e.s[0].w[i], 0 -  1, tmp);
  EOR_AND_ROR(o.s[1].w[i], d.s[1].w[i], e.s[1].w[i], 1 -  1, tmp);
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], c.s[0].w[i], ROT(-1));
  EOR_AND_ROR(o.s[1].w[i], d.s[1].w[i], e.s[2].w[i], 2 -  1, tmp);

  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], e.s[0].w[i], ROT(-2));
  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], c.s[1].w[i], ROT(-1));  
  EOR_AND_ROR(o.s[2].w[i], d.s[2].w[i], e.s[0].w[i], 0 -  2, tmp);
  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], e.s[1].w[i], ROT(-1));
  EOR_AND_ROR(o.s[2].w[i], d.s[2].w[i], e.s[1].w[i], 1 -  2, tmp);
  EOR_AND_ROR(o.s[2].w[i], d.s[2].w[i], e.s[2].w[i], 2 -  2, tmp);
  }
  return o;
}
return o;
}

forceinline word_t MXORBIC4(word_t o, word_t d, word_t e, word_t a, int ns) {
uint32_t tmp;

if (ns == 1) {
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  o.s[0].w[i] = d.s[0].w[i];
  EOR_BIC_ROR(o.s[0].w[i], a.s[0].w[i], e.s[0].w[i], 0, tmp); 
  }
  return o; 
}
if (ns == 2) {
/*
D'0 = A1E0 ⊕ D0 ⊕ A0E0,                 
D'1 = A0E1 ⊕ A0 ⊕ A1E1 ⊕ D1 ⊕ A1.
*/
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], d.s[0].w[i], ROT(0) );
  EOR_AND_ROR(o.s[0].w[i], e.s[0].w[i], a.s[1].w[i], 1, tmp);
  EOR_AND_ROR(o.s[0].w[i], a.s[0].w[i], e.s[0].w[i], 0, tmp); 
  }
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], d.s[1].w[i], ROT(0) );  
  EOR_AND_ROR(o.s[1].w[i], e.s[1].w[i], a.s[0].w[i], -1, tmp); 
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], a.s[0].w[i], ROT(-1) );  
  EOR_AND_ROR(o.s[1].w[i], a.s[1].w[i], e.s[1].w[i], 0, tmp);
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], a.s[1].w[i], ROT(0) );
  }       
  return o;  
}

if (ns == 3) {
/* 
 D'0 = A0 ⊕ D0   ⊕ A0E0 ⊕ A1 ⊕ A1E0 ⊕ A2E0,
 D'1 = A0 ⊕ A0E1 ⊕ A1E1 ⊕ A2 ⊕ D2   ⊕ A2E1,
 D'2 = A0 ⊕ A0E2 ⊕ A1E2 ⊕ D1 ⊕ A2E2,
*/
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], a.s[0].w[i], ROT(0));  
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], d.s[0].w[i], ROT(0));     
  EOR_AND_ROR(o.s[0].w[i], a.s[0].w[i], e.s[0].w[i], 0 - 0, tmp);
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], a.s[1].w[i], ROT(1));
  EOR_AND_ROR(o.s[0].w[i], e.s[0].w[i], a.s[1].w[i], 1 - 0, tmp);  
  EOR_AND_ROR(o.s[0].w[i], e.s[0].w[i], a.s[2].w[i], 2 - 0, tmp);
  
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], a.s[0].w[i], ROT(-1));
  EOR_AND_ROR(o.s[1].w[i], e.s[1].w[i], a.s[0].w[i], 0 -  1, tmp);
  EOR_AND_ROR(o.s[1].w[i], e.s[1].w[i], a.s[1].w[i], 1 -  1, tmp);
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], a.s[2].w[i], ROT(1));
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], d.s[2].w[i], ROT(1));  
  EOR_AND_ROR(o.s[1].w[i], e.s[1].w[i], a.s[2].w[i], 2 -  1, tmp);

  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], a.s[0].w[i],  ROT(-2));
  EOR_AND_ROR(o.s[2].w[i], e.s[2].w[i], a.s[0].w[i],  0 -  2, tmp);
  EOR_AND_ROR(o.s[2].w[i], e.s[2].w[i], a.s[1].w[i],  1 -  2, tmp); 
  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], d.s[1].w[i],  ROT(-1));
  EOR_AND_ROR(o.s[2].w[i], e.s[2].w[i], a.s[2].w[i],  2 -  2, tmp);
  }
  return o;
}
return o;
}

forceinline word_t MXORBIC5(word_t o, word_t e, word_t a, word_t b, int ns) {
uint32_t tmp;

if (ns == 1) {
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  o.s[0].w[i] = e.s[0].w[i];
  EOR_BIC_ROR(o.s[0].w[i], b.s[0].w[i], a.s[0].w[i], 0, tmp);  
  }
  return o;
}
if (ns == 2) {
/* 
  E'0 = B0A0 ⊕ E0 ⊕ B0 ⊕ B0A1, 
  E'1 = B1A0 ⊕ E1 ⊕ B1 ⊕ B1A1.
*/
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  EOR_AND_ROR(o.s[0].w[i], b.s[0].w[i], a.s[0].w[i], 0, tmp);
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], e.s[0].w[i], ROT(0) );  
  EOR_AND_ROR(o.s[0].w[i], b.s[0].w[i], a.s[1].w[i], 1, tmp);    
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], b.s[0].w[i], ROT(0) );
  }
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], e.s[1].w[i], ROT(0) );
  EOR_AND_ROR(o.s[1].w[i], b.s[1].w[i], a.s[0].w[i], -1, tmp);   
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], b.s[1].w[i], ROT(0) );
  EOR_AND_ROR(o.s[1].w[i], a.s[1].w[i], b.s[1].w[i], 0, tmp); 
  }  
  return o;  
}

if (ns == 3) {
/*
 E'0 = B0 ⊕ E0 ⊕ B0A0 ⊕ B1 ⊕ B1A0 ⊕ B2A0,
 E'1 = B0 ⊕ B0A1 ⊕ B1A1 ⊕ E2 ⊕ B2A1,
 E′2  = B0 ⊕ B0A2 ⊕ B1A2 ⊕ B2 ⊕ E1 ⊕ B2A2.
*/
  #pragma GCC unroll 2
  for (int i=0;i<2;i++){
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], b.s[0].w[i], ROT(0));  
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], e.s[0].w[i], ROT(0));     
  EOR_AND_ROR(o.s[0].w[i], a.s[0].w[i], b.s[0].w[i], 0 - 0, tmp);
  EOR_ROR    (o.s[0].w[i], o.s[0].w[i], b.s[1].w[i], ROT(1));
  EOR_AND_ROR(o.s[0].w[i], a.s[0].w[i], b.s[1].w[i], 1 - 0, tmp);
  EOR_AND_ROR(o.s[0].w[i], a.s[0].w[i], b.s[2].w[i], 2 - 0, tmp);
  
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], b.s[0].w[i], ROT(-1));
  EOR_AND_ROR(o.s[1].w[i], a.s[1].w[i], b.s[0].w[i], 0 -  1, tmp);
  EOR_AND_ROR(o.s[1].w[i], a.s[1].w[i], b.s[1].w[i], 1 -  1, tmp);
  EOR_ROR    (o.s[1].w[i], o.s[1].w[i], e.s[2].w[i], ROT(1));
  EOR_AND_ROR(o.s[1].w[i], a.s[1].w[i], b.s[2].w[i], 2 -  1, tmp);

  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], b.s[0].w[i], ROT(-2));
  EOR_AND_ROR(o.s[2].w[i], a.s[2].w[i], b.s[0].w[i], 0 -  2, tmp);
  EOR_AND_ROR(o.s[2].w[i], a.s[2].w[i], b.s[1].w[i], 1 -  2, tmp);
  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], b.s[2].w[i], ROT(0));
  EOR_ROR    (o.s[2].w[i], o.s[2].w[i], e.s[1].w[i], ROT(-1));
  EOR_AND_ROR(o.s[2].w[i], a.s[2].w[i], b.s[2].w[i], 2 -  2, tmp);
  }
  return o;
}
return o;
}

forceinline word_t MXORBIC(word_t c, word_t a, word_t b, int i, int ns) {
  uint32_t tmp;
  if (ns == 1) {
    EOR_BIC_ROR(c.s[0].w[i], a.s[0].w[i], b.s[0].w[i], 0, tmp);
  }
  if (ns == 2) {
    EOR_BIC_ROR(c.s[0].w[i], a.s[0].w[i], b.s[0].w[i], 0, tmp);
    EOR_BIC_ROR(c.s[1].w[i], a.s[1].w[i], b.s[0].w[i], 0 - 1, tmp);
    CLEAR();
    EOR_AND_ROR(c.s[1].w[i], a.s[1].w[i], b.s[1].w[i], 0, tmp);
    EOR_AND_ROR(c.s[0].w[i], a.s[0].w[i], b.s[1].w[i], 1 - 0, tmp);
    CLEAR();
  }
  if (ns == 3) {
    EOR_AND_ROR(c.s[0].w[i], b.s[0].w[i], a.s[1].w[i], 1 - 0, tmp);
    EOR_BIC_ROR(c.s[0].w[i], a.s[0].w[i], b.s[0].w[i], 0, tmp);
    EOR_AND_ROR(c.s[0].w[i], b.s[0].w[i], a.s[2].w[i], 2 - 0, tmp);
    EOR_AND_ROR(c.s[1].w[i], b.s[1].w[i], a.s[2].w[i], 2 - 1, tmp);
    EOR_BIC_ROR(c.s[1].w[i], a.s[1].w[i], b.s[1].w[i], 0, tmp);
    EOR_AND_ROR(c.s[1].w[i], b.s[1].w[i], a.s[0].w[i], 0 - 1, tmp);
    EOR_BIC_ROR(c.s[2].w[i], b.s[2].w[i], a.s[0].w[i], 0 - 2, tmp);
    EOR_ORR_ROR(c.s[2].w[i], a.s[2].w[i], b.s[2].w[i], 0, tmp);
    EOR_AND_ROR(c.s[2].w[i], b.s[2].w[i], a.s[1].w[i], 1 - 2, tmp);
  }
  if (ns == 4) {
    EOR_BIC_ROR(c.s[0].w[i], a.s[0].w[i], b.s[0].w[i], 0, tmp);
    EOR_BIC_ROR(c.s[1].w[i], a.s[1].w[i], b.s[0].w[i], 0 - 1, tmp);
    EOR_BIC_ROR(c.s[2].w[i], a.s[2].w[i], b.s[0].w[i], 0 - 2, tmp);
    EOR_BIC_ROR(c.s[3].w[i], a.s[3].w[i], b.s[0].w[i], 0 - 3, tmp);
    EOR_AND_ROR(c.s[1].w[i], a.s[1].w[i], b.s[1].w[i], 0, tmp);
    EOR_AND_ROR(c.s[2].w[i], a.s[2].w[i], b.s[1].w[i], 1 - 2, tmp);
    EOR_AND_ROR(c.s[3].w[i], a.s[3].w[i], b.s[1].w[i], 1 - 3, tmp);
    EOR_AND_ROR(c.s[0].w[i], a.s[0].w[i], b.s[1].w[i], 1 - 0, tmp);
    EOR_AND_ROR(c.s[2].w[i], a.s[2].w[i], b.s[2].w[i], 0, tmp);
    EOR_AND_ROR(c.s[3].w[i], a.s[3].w[i], b.s[2].w[i], 2 - 3, tmp);
    EOR_AND_ROR(c.s[0].w[i], a.s[0].w[i], b.s[2].w[i], 2 - 0, tmp);
    EOR_AND_ROR(c.s[1].w[i], a.s[1].w[i], b.s[2].w[i], 2 - 1, tmp);
    EOR_AND_ROR(c.s[3].w[i], a.s[3].w[i], b.s[3].w[i], 0, tmp);
    EOR_AND_ROR(c.s[0].w[i], a.s[0].w[i], b.s[3].w[i], 3 - 0, tmp);
    EOR_AND_ROR(c.s[1].w[i], a.s[1].w[i], b.s[3].w[i], 3 - 1, tmp);
    EOR_AND_ROR(c.s[2].w[i], a.s[2].w[i], b.s[3].w[i], 3 - 2, tmp);
  }
  return c;
}

forceinline word_t MXORAND(word_t c, word_t a, word_t b, int ns) {
  b.s[0].w[0] = ~b.s[0].w[0];
  b.s[0].w[1] = ~b.s[0].w[1];
  c = MXORBIC(c, a, b, 0, ns);
  c = MXORBIC(c, a, b, 1, ns);
  return c;
}

forceinline word_t MRND(int ns) {
  word_t w;
  if (ns >= 2) RND(w.s[1].w[0]);
  if (ns >= 2) RND(w.s[1].w[1]);
  if (ns >= 3) RND(w.s[2].w[0]);
  if (ns >= 3) RND(w.s[2].w[1]);
  if (ns >= 4) RND(w.s[3].w[0]);
  if (ns >= 4) RND(w.s[3].w[1]);
  return w;
}

forceinline word_t MZERO(int ns) {
  word_t w;
  if (ns == 1) {
    MOVI(w.s[0].w[0], 0);
    MOVI(w.s[0].w[1], 0);
  }
  if (ns >= 2) {
    RND(w.s[1].w[0]);
    RND(w.s[1].w[1]);   
    RORI(w.s[0].w[0], w.s[1].w[0], ROT(1));
    RORI(w.s[0].w[1], w.s[1].w[1], ROT(1));
    if (ns == 2) CLEAR();
  }
  if (ns >= 3) {
    RND(w.s[2].w[0]);
    RND(w.s[2].w[1]);   
    EOR_ROR(w.s[0].w[0], w.s[0].w[0], w.s[2].w[0], ROT(2));
    EOR_ROR(w.s[0].w[1], w.s[0].w[1], w.s[2].w[1], ROT(2));
  }
  if (ns >= 4) {
    RND(w.s[3].w[0]);
    RND(w.s[3].w[1]);   
    EOR_ROR(w.s[0].w[0], w.s[0].w[0], w.s[3].w[0], ROT(3));
    EOR_ROR(w.s[0].w[1], w.s[0].w[1], w.s[3].w[1], ROT(3));
  }
  return w;
}

forceinline word_t MMASK(int n, int ns) {
  uint32_t mask = 0xffffffff >> (n * 4);
  word_t m = MZERO(ns);
  m.s[0].w[0] ^= mask;
  m.s[0].w[1] ^= mask;
  return m;
}

forceinline word_t MREFRESH(word_t w, int ns) {
  word_t r = MZERO(ns);
  return MXOR(w, r, ns);
}

forceinline int MNOTZERO(word_t a, word_t b, int ns) {
  word_t c = MZERO(ns);
  /* note: OR(a,b) = ~BIC(~a,b) */
  a.s[0].w[0] = ~a.s[0].w[0];
  a.s[0].w[1] = ~a.s[0].w[1];
 /* OR first and second 64-bit word */
 c = MXORBIC(c, a, b, 0, ns);
  c = MXORBIC(c, a, b, 1, ns);
  /* OR even and odd words */
  if (ns >= 1) b.s[0].w[0] = c.s[0].w[1];
  if (ns >= 2) b.s[1].w[0] = c.s[1].w[1];
  if (ns >= 3) b.s[2].w[0] = c.s[2].w[1];
  if (ns >= 4) b.s[3].w[0] = c.s[3].w[1];
  a = MXORBIC(a, b, c, 0, ns);
  /* loop to OR 16/8/4/2/1 bit chunks */
  for (int i = 16; i > 0; i >>= 1) {
    if (ns >= 1) b.s[0].w[0] = ROR32(a.s[0].w[0], i);
    if (ns >= 2) b.s[1].w[0] = ROR32(a.s[1].w[0], i);
    if (ns >= 3) b.s[2].w[0] = ROR32(a.s[2].w[0], i);
    if (ns >= 4) b.s[3].w[0] = ROR32(a.s[3].w[0], i);
    c = MXORBIC(c, a, b, 0, ns);
    if (ns >= 1) a.s[0].w[0] = c.s[0].w[0];
    if (ns >= 2) a.s[1].w[0] = c.s[1].w[0];
    if (ns >= 3) a.s[2].w[0] = c.s[2].w[0];
    if (ns >= 4) a.s[3].w[0] = c.s[3].w[0];
  }
  /* unmask result */
  if (ns >= 2) a.s[0].w[0] ^= ROR32(a.s[1].w[0], ROT(1));
  if (ns >= 3) a.s[0].w[0] ^= ROR32(a.s[2].w[0], ROT(2));
  if (ns >= 4) a.s[0].w[0] ^= ROR32(a.s[3].w[0], ROT(3));
  return ~a.s[0].w[0];
}

forceinline share_t LOADSHARE(uint32_t* data, int ns) {
  share_t s;
  uint32_t lo, hi;
  LDR(lo, data, 0);
  LDR(hi, data, 4 * ns);
#if !ASCON_EXTERN_BI
  BD(s.w[0], s.w[1], lo, hi);
  if (ns == 2) CLEAR();
#else
  s.w[0] = lo;
  s.w[1] = hi;
#endif
  return s;
}

forceinline void STORESHARE(uint32_t* data, share_t s, int ns) {
  uint32_t lo, hi;
#if !ASCON_EXTERN_BI
  BI(lo, hi, s.w[0], s.w[1]);
  if (ns == 2) CLEAR();
#else
  lo = s.w[0];
  hi = s.w[1];
#endif
  STR(lo, data, 0);
  STR(hi, data, 4 * ns);
}

forceinline word_t MLOAD(uint32_t* data, int ns) {
  word_t w = {0};
  if (ns >= 1) w.s[0] = LOADSHARE(&(data[0]), ns);
  if (ns >= 2) w.s[1] = LOADSHARE(&(data[1]), ns);
  if (ns >= 3) w.s[2] = LOADSHARE(&(data[2]), ns);
  if (ns >= 4) w.s[3] = LOADSHARE(&(data[3]), ns);
  return w;
}

forceinline void MSTORE(uint32_t* data, word_t w, int ns) {
  if (ns >= 1) STORESHARE(&(data[0]), w.s[0], ns);
  if (ns >= 2) STORESHARE(&(data[1]), w.s[1], ns);
  if (ns >= 3) STORESHARE(&(data[2]), w.s[2], ns);
  if (ns >= 4) STORESHARE(&(data[3]), w.s[3], ns);
}

#endif /* WORD_H_ */
