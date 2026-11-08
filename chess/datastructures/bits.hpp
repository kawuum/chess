#ifndef CHESS_BITS_HPP
#define CHESS_BITS_HPP

#include <stdint.h>

/**
 * Bit counting and bit scanning on 64-bit words, in one place.
 *
 * On the PC these are GCC builtins: with -mpopcnt, popcount() is the single
 * instruction POPCNT. The ARM7 of the Game Boy Advance has neither POPCNT
 * nor CLZ, and 64-bit words are two registers there; the builtins would
 * become library calls. The portable versions work on two 32-bit halves.
 * Defining CHESS_PORTABLE_BITS uses them everywhere, and
 * tests/bits_test.cpp checks that both versions agree.
 */
namespace bits {

/** Number of set bits of a 32-bit word, by adding neighbouring bit groups. */
constexpr int popcount32_portable(uint32_t x) {
  x = x - ((x >> 1) & 0x55555555u);                  // 16 sums of 2 bits
  x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);  // 8 sums of 4 bits
  x = (x + (x >> 4)) & 0x0F0F0F0Fu;                  // 4 sums of 8 bits
  return static_cast<int>((x * 0x01010101u) >> 24);  // add the 4 bytes
}

/** Number of set bits, without POPCNT. */
constexpr int popcount_portable(uint64_t x) {
  return popcount32_portable(static_cast<uint32_t>(x)) +
         popcount32_portable(static_cast<uint32_t>(x >> 32));
}

/** Zero bits above the highest set bit of a 32-bit word; x must not be 0. */
constexpr int leading_zeros32_portable(uint32_t x) {
  int n = 0;
  if ((x & 0xFFFF0000u) == 0) n += 16, x <<= 16;
  if ((x & 0xFF000000u) == 0) n += 8, x <<= 8;
  if ((x & 0xF0000000u) == 0) n += 4, x <<= 4;
  if ((x & 0xC0000000u) == 0) n += 2, x <<= 2;
  if ((x & 0x80000000u) == 0) n += 1;
  return n;
}

/** Zero bits above the highest set bit, without CLZ; x must not be 0. */
constexpr int leading_zeros_portable(uint64_t x) {
  const uint32_t high = static_cast<uint32_t>(x >> 32);
  return high != 0 ? leading_zeros32_portable(high)
                   : 32 + leading_zeros32_portable(static_cast<uint32_t>(x));
}

#ifdef CHESS_PORTABLE_BITS
constexpr int popcount(uint64_t x) { return popcount_portable(x); }
constexpr int leading_zeros(uint64_t x) { return leading_zeros_portable(x); }
#else
/** Number of set bits. */
constexpr int popcount(uint64_t x) { return __builtin_popcountll(x); }
/**
 * Zero bits above the highest set bit; x must not be 0. In the bit order of
 * bit_matrix (a1 = highest bit) this is the square index 8 * y + x of the
 * first set square.
 */
constexpr int leading_zeros(uint64_t x) { return __builtin_clzll(x); }
#endif

}  // namespace bits

#endif
