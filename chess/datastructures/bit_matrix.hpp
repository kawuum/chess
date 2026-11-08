#ifndef CHESS_BIT_MATRIX_HPP
#define CHESS_BIT_MATRIX_HPP

#include <stdint.h>

#include "bits.hpp"

/**
 * An 8x8 bitboard: one bit per square, stored in a single uint64_t.
 *
 * Coordinates: x is the file (0 = a), y is the rank (0 = rank 1). Square
 * (x, y) has the index i = 8 * y + x and is stored in bit number 63 - i,
 * counted from the right. So a1 is the highest bit and h8 the lowest. This
 * order costs nothing: get_bit_at() compiles to four machine instructions
 * either way.
 *
 * Everything is inline in this header so the compiler can optimize the small
 * functions away. There are no range checks on purpose (performance):
 * coordinates must be 0..7.
 */
class bit_matrix {
 public:
  constexpr bit_matrix() : raw(0) {}

  /** A bitboard with the given raw bits (see the bit order above). */
  constexpr explicit bit_matrix(uint64_t in) : raw(in) {}

  /** True if the bit of square (x, y) is set. */
  constexpr bool get_bit_at(uint8_t x, uint8_t y) const {
    return get_bit_at((8 * y) + x);
  }

  /** Sets the bit of square (x, y) to 1. */
  void set_bit_at(uint8_t x, uint8_t y) { set_bit_at((8 * y) + x); }

  /** Sets the bit of square (x, y) to 0. */
  void unset_bit_at(uint8_t x, uint8_t y) { unset_bit_at((8 * y) + x); }

  /** Number of set bits (see bits.hpp). */
  constexpr uint8_t get_num_bits_set() const {
    return static_cast<uint8_t>(bits::popcount(raw));
  }

  /** Sets all bits to 0. */
  void clear() { raw = 0; }

  /** The internal representation (see the bit order above). */
  constexpr uint64_t get_raw() const { return raw; }

  constexpr bit_matrix operator&(const bit_matrix& rhs) const {
    return bit_matrix(raw & rhs.raw);
  }

  constexpr bit_matrix operator|(const bit_matrix& rhs) const {
    return bit_matrix(raw | rhs.raw);
  }

  constexpr bool operator==(const bit_matrix& rhs) const {
    return raw == rhs.raw;
  }

  constexpr bool operator!=(const bit_matrix& rhs) const {
    return raw != rhs.raw;
  }

 private:
  // The highest bit: 1000...0
  static constexpr uint64_t kTopBit = uint64_t{1} << 63;

  uint64_t raw;

  constexpr bool get_bit_at(uint8_t i) const { return (raw << i) & kTopBit; }

  void set_bit_at(uint8_t i) { raw |= kTopBit >> i; }

  void unset_bit_at(uint8_t i) { raw &= ~(kTopBit >> i); }
};

/** Compares the raw bits with a number, e.g. (a & b) == 0. */
constexpr bool operator==(const bit_matrix& lhs, uint64_t rhs) {
  return lhs.get_raw() == rhs;
}

constexpr bool operator!=(const bit_matrix& lhs, uint64_t rhs) {
  return lhs.get_raw() != rhs;
}

#endif  // CHESS_BIT_MATRIX_HPP
