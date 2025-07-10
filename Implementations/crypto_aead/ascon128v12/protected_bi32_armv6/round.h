#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "constants.h"
#include "printstate.h"

forceinline state_t AFFINE1(state_t s, int d) {
  s.x[2].s[d].w[0] ^= s.x[1].s[d].w[0];
  s.x[0].s[d].w[0] ^= s.x[4].s[d].w[0];
  s.x[4].s[d].w[0] ^= s.x[3].s[d].w[0];
  s.x[2].s[d].w[1] ^= s.x[1].s[d].w[1];
  s.x[0].s[d].w[1] ^= s.x[4].s[d].w[1];
  s.x[4].s[d].w[1] ^= s.x[3].s[d].w[1];
  return s;
}

forceinline state_t AFFINE2(state_t s, int d) {
  s.x[1].s[d].w[0] ^= s.x[0].s[d].w[0];
  s.x[3].s[d].w[0] ^= s.x[2].s[d].w[0];
  s.x[0].s[d].w[0] ^= s.x[4].s[d].w[0];
  s.x[1].s[d].w[1] ^= s.x[0].s[d].w[1];
  s.x[3].s[d].w[1] ^= s.x[2].s[d].w[1];
  s.x[0].s[d].w[1] ^= s.x[4].s[d].w[1];
  
  return s;
}

forceinline state_t SBOX(state_t s, int ns) {
  state_t s_temp;
  
  /* affine layer 1 */
  if (ns >= 1) s = AFFINE1(s, 0);
  if (ns >= 2) s = AFFINE1(s, 1);
  if (ns >= 3) s = AFFINE1(s, 2);
  if (ns >= 4) s = AFFINE1(s, 3);
 
  #pragma GCC unroll 40
  for (int j = 0; j < 5; j++)
  for (int jj = 0; jj < ns; jj++){
    s_temp.x[j].s[jj].w[0] = s.x[j].s[jj].w[0];
    s.x[j].s[jj].w[0] = 0;   
  }   
  #pragma GCC unroll 40
  for (int j = 0; j < 5; j++)
  for (int jj = 0; jj < ns; jj++){   
    s_temp.x[j].s[jj].w[1] = s.x[j].s[jj].w[1];
    s.x[j].s[jj].w[1] = 0;
  }   
  
 if(ns<4){ 
 s.x[0] = MXORBIC1(s.x[0], s_temp.x[0], s_temp.x[1], s_temp.x[2], ns);   
 s.x[2] = MXORBIC3(s.x[2], s_temp.x[2], s_temp.x[3], s_temp.x[4], ns); 
 s.x[1] = MXORBIC2(s.x[1], s_temp.x[1], s_temp.x[2], s_temp.x[3], ns);    
 s.x[3] = MXORBIC4(s.x[3], s_temp.x[3], s_temp.x[4], s_temp.x[0], ns);  
 s.x[4] = MXORBIC5(s.x[4], s_temp.x[4], s_temp.x[0], s_temp.x[1], ns);  
 }
 else{
 s.x[0] = M4XORBIC(s.x[0], s_temp.x[0], s_temp.x[1], s_temp.x[2], 0);   
 s.x[1] = M4XORBIC(s.x[1], s_temp.x[1], s_temp.x[2], s_temp.x[3], 0); 
 s.x[2] = M4XORBIC(s.x[2], s_temp.x[2], s_temp.x[3], s_temp.x[4], 0);    
 s.x[3] = M4XORBIC(s.x[3], s_temp.x[3], s_temp.x[4], s_temp.x[0], 0);  
 s.x[4] = M4XORBIC(s.x[4], s_temp.x[4], s_temp.x[0], s_temp.x[1], 1);  
 }

  /* affine layer 2 */
  if (ns >= 1) s = AFFINE2(s, 0);
  s.x[2].s[0].w[0] = ~s.x[2].s[0].w[0];
  s.x[2].s[0].w[1] = ~s.x[2].s[0].w[1];
  if (ns >= 2) s = AFFINE2(s, 1);
  if (ns >= 3) s = AFFINE2(s, 2);
  if (ns >= 4) s = AFFINE2(s, 3);
  
  return s;
}

forceinline state_t LINEAR(state_t s, int d) {
  state_t t;
  t.x[0].s[d].w[0] = s.x[0].s[d].w[0] ^ ROR32(s.x[0].s[d].w[1], 4);
  t.x[0].s[d].w[1] = s.x[0].s[d].w[1] ^ ROR32(s.x[0].s[d].w[0], 5);
  t.x[1].s[d].w[0] = s.x[1].s[d].w[0] ^ ROR32(s.x[1].s[d].w[0], 11);
  t.x[1].s[d].w[1] = s.x[1].s[d].w[1] ^ ROR32(s.x[1].s[d].w[1], 11);
  t.x[2].s[d].w[0] = s.x[2].s[d].w[0] ^ ROR32(s.x[2].s[d].w[1], 2);
  t.x[2].s[d].w[1] = s.x[2].s[d].w[1] ^ ROR32(s.x[2].s[d].w[0], 3);
  t.x[3].s[d].w[0] = s.x[3].s[d].w[0] ^ ROR32(s.x[3].s[d].w[1], 3);
  t.x[3].s[d].w[1] = s.x[3].s[d].w[1] ^ ROR32(s.x[3].s[d].w[0], 4);
  t.x[4].s[d].w[0] = s.x[4].s[d].w[0] ^ ROR32(s.x[4].s[d].w[0], 17);
  t.x[4].s[d].w[1] = s.x[4].s[d].w[1] ^ ROR32(s.x[4].s[d].w[1], 17);
  s.x[0].s[d].w[0] ^= ROR32(t.x[0].s[d].w[1], 9);
  s.x[0].s[d].w[1] ^= ROR32(t.x[0].s[d].w[0], 10);
  s.x[1].s[d].w[0] ^= ROR32(t.x[1].s[d].w[1], 19);
  s.x[1].s[d].w[1] ^= ROR32(t.x[1].s[d].w[0], 20);
  s.x[2].s[d].w[0] ^= ROR32(t.x[2].s[d].w[1], 0);
  s.x[2].s[d].w[1] ^= ROR32(t.x[2].s[d].w[0], 1);
  s.x[3].s[d].w[0] ^= ROR32(t.x[3].s[d].w[0], 5);
  s.x[3].s[d].w[1] ^= ROR32(t.x[3].s[d].w[1], 5);
  s.x[4].s[d].w[0] ^= ROR32(t.x[4].s[d].w[1], 3);
  s.x[4].s[d].w[1] ^= ROR32(t.x[4].s[d].w[0], 4);
  return s;
}

forceinline void ROUND_(state_t* p, uint8_t C_o, uint8_t C_e, int ns) {
  state_t s = *p;
  /* constant and sbox layer*/
  s.x[2].s[0].w[0] ^= C_e;  
  s.x[2].s[0].w[1] ^= C_o;
  s = SBOX(s, ns);
  /* linear layer*/
  if (ns >= 4) s = LINEAR(s, 3);
  if (ns >= 3) s = LINEAR(s, 2);
  if (ns >= 2) s = LINEAR(s, 1);
  if (ns >= 1) s = LINEAR(s, 0);
  *p = s;
  printstate(" round output", &s, ns);
}

forceinline void ROUND(state_t* p, uint64_t C, int ns) {
  ROUND_(p, C >> 32, C, ns);
}

forceinline void PROUNDS(state_t* s, int nr, int ns) {
  int i = START(nr);
  do {
    ROUND_(s, RC(i), ns);
    i += INC;
  } while (i != END);
}

#endif /* ROUND_H_ */

