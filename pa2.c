// ----------------------------------------------------------------
// 
//   4190.308 Computer Architecture (Fall 2022)
// 
//   Project #2: SFP16 (16-bit floating point) Adder
// 
//   October 4, 2022
// 
//   Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
//   Jaehoon Shim (mattjs@snu.ac.kr)
//   IlKueon Kang (kangilkueon@snu.ac.kr)
//   Wookje Han (gksdnrwp@snu.ac.kr)
//   Jinsol Park (jinsolpark@snu.ac.kr)
//   Systems Software & Architecture Laboratory
//   Dept. of Computer Science and Engineering
//   Seoul National University
// 
// ----------------------------------------------------------------

typedef unsigned short SFP16;

/* Add two SFP16-type numbers and return the result */
#define MASK_SIGN   (SFP16)0x8000
#define MASK_ABS    (SFP16)0x7fff
#define P_INF       (SFP16)0x7f00
#define N_INF       (SFP16)0xff00
#define NAN         (SFP16)0x7f01
#define IS_NAN(x)   !!(x & 0xff)

#define SIGN(x)     (x & MASK_SIGN)
#define ABS(x)      (x & MASK_ABS)
#define FEXPN_B(x)  ((x >> 8) & 0x7f)
#define FFRAC_B(x)  ((x & 0xff) + (!!FEXPN_B(x) << 8))

SFP16 fpadd(SFP16 x, SFP16 y)
{
  /* TODO */
  SFP16 res = 0;
  SFP16 m_x, m_y;
  signed short d, e;

  //  1. (o)
  if (ABS(x) < ABS(y))
    return fpadd(y, x);

  //  special cases : +-inf, +-nan
  if (FEXPN_B(x) == 0x7f)
  {
    if (x == y) // +inf + +inf, -inf + -inf
    { // s-1
      return (x & 0xff00) | IS_NAN(x);
    }
    else if (FEXPN_B(y) == 0x7f)
    { // s-2
      return (x & 0xff00) | NAN;
    }
    else
    { // s-3
      return (x & 0xff00) | IS_NAN(x);
    }
  }

  res = x & 0x8000;

  //  2. (o)
  if (FEXPN_B(x) != 0)
  { // n + n, n + d
    d = FEXPN_B(x) - FEXPN_B(y);

    if (d > 11)
    { // lost of significance
      return x;
    }
    
    d -= !FEXPN_B(y);
  }
  else
  { // d + d
    d = 0;
  }

  e = FEXPN_B(x);
  m_x = FFRAC_B(x) << 3;
  m_y = ((FFRAC_B(y) << 3) >> d) | !!(FFRAC_B(y) & (0x01ff >> (11 - d)));

  //  3. (o)
  if (SIGN(x) == SIGN(y))
    m_x += m_y;
  else if (m_x > m_y)
    m_x -= m_y;
  else
    return res;

  //  4. (o)
  if (m_x & 0x1000) // rshift,  only occur @ n + n
    m_x = (m_x >> 1) | (m_x & 0x1), e += 1;
  else
  {
    if (e)
    { // n +- *
      if (m_x & 0x800);
      else if (d >= 2) // subtraction
        m_x <<= 1, e -= 1;
      else
      {
        if (m_x & 0x7f8);
        else
          m_x <<= 1, e -= 1;

        if (m_x & 0x780);
        else
          m_x <<= 4, e -= 4;

        if (m_x & 0x600);
        else
          m_x <<= 2, e -= 2;

        if (m_x & 0x400);
        else
          m_x <<= 1, e -= 1;

        m_x <<= 1, e -= 1;

        // (signed short) e <= 0
        if (e <= 0)
        { // n -> d
          m_x >>= (1 - e);
          e = 0;
        }
      }
    }
    else
    { // d + d, d - d
      if (m_x & 0x800)
        e += 1; // d -> n
    }
  }

  //  5. (o)
  m_x = (m_x >> 1) | (m_x & 0x1);
  m_x += (m_x & 0x2) * !!(m_x & 0x5);

  //  6. (o)
  if (m_x & 0x800)
    m_x = 0, e += 1;
  else if (!e)
    if (m_x & 0x400)
      e += 1;

  m_x >>= 2;
  
  //  7. (o)
  if (e >= 0x7f)
    res |= 0x7f00;
  else
    res |= (e << 8) | (m_x & 0xff);

  return res;
}
