//
// Created by Siegfried Depner on 7/12/18.
//

#ifndef CHESS_BIT_MATRIX_HPP
#define CHESS_BIT_MATRIX_HPP

#include <iostream>
#include <stdint.h>

// TODO: Optimize.
// TODO: Check if we should put code in .cpp file (but would break inlining, so probably not).
// TODO: Error handling? But probably not, performance...

/**
 * This class implements a very simple bit_matrix which provides basic methods for retrieving/manipulating the set bits.
 * The class supports matrices with size 8x8 only.
 *
 */

class bit_matrix {

 public:

  bit_matrix() : raw(0) {
  }

  bit_matrix(uint64_t in) {
    raw = in;
  }

  bool get_bit_at(uint8_t i) {
    const uint64_t ONE_LEFT = 9223372036854775808u; // Bit-signature is 1000...0
    return (raw << i) & ONE_LEFT;
  }

  /**
   * Kinda suboptimal solution, since we use an extra subtraction (-7) to make sure that the x-offspring
   * is in the lower left corner (instead of lower right)...
   * @param x The x coordinate to retrieve the bit at (offspring is lower left corner, goes right)
   * @param y The y coordinate to retrieve the bit at (offspring is lower left corner, goes up)
   * @return The set bit on specified position.
   */
  bool get_bit_at(uint8_t x, uint8_t y) {
    return get_bit_at((8 * y) + x);
  }

  /**
   * Sets bit at specified position to 1.
   * @param x The x coordinate to set the bit at (offspring is lower left corner, goes right)
   * @param y The y coordinate to set the bit at (offspring is lower left corner, goes up)
   */
  void set_bit_at(uint8_t x, uint8_t y) {
    set_bit_at((8 * y) + x);
  }

  /**
   * Sets bit at specified position to 0.
   * @param x The x coordinate to set the bit at (offspring is lower left corner, goes right)
   * @param y The y coordinate to set the bit at (offspring is lower left corner, goes up)
   */
  void unset_bit_at(uint8_t x, uint8_t y) {
    unset_bit_at((8 * y) + x);
  }

  /**
   * Using __builtin_popcountl (i.e. compiler-specific method) makes this non-portable. On the other hand, using this
   * method makes it blazingly fast (well, at least it should). For now, we will stay with this builtin, and porting
   * this special function to a general (slow) algorithm is very easy anyway.
   * @return Number of bits set to 1 in the bit matrix.
   */
  uint8_t get_num_bits_set() {
    return (uint8_t) __builtin_popcountl(raw);
  }

  /**
   * Method to unset all bits.
   */
  void clear() {
    raw = 0;
  }

  /**
   * Returns the internal representation of the bit matrix
   * @return Raw bit matrix
   */
  uint64_t get_raw() {
    return raw;
  }

  bit_matrix operator&(const bit_matrix &rhs) {
    return bit_matrix(raw & rhs.raw);
  }

  bit_matrix operator|(const bit_matrix &rhs) {
    return bit_matrix(raw | rhs.raw);
  }

  bool operator==(bit_matrix &rhs) {
    return raw == rhs.raw;
  }
  
  bool operator!=(bit_matrix &rhs) {
    return raw != rhs.raw;
  }

  /*
   * Helper methods
   */
  // TODO: Delete me at some point
  void print() {
    std::cout << "--------" << std::endl;
    for (uint8_t y = 8; y > 0; --y) {
      for (uint8_t x = 0; x < 8; ++x) {
        std::cout << get_bit_at(x, y - 1);
      }
      std::cout << std::endl;
    }
    std::cout << "--------" << std::endl;
  }

 private:
  uint64_t raw;

  void set_bit_at(uint8_t i) {
    const uint64_t ONE_LEFT = 9223372036854775808u; // Bit-signature is 1000...0
    raw = raw | (ONE_LEFT >> i);
  }

  void unset_bit_at(uint8_t i) {
    const uint64_t ONE_LEFT = 9223372036854775808u; // Bit-signature is 1000...0
    raw = raw & (~(ONE_LEFT >> i));
  }

};

inline bool operator==(bit_matrix lhs, const uint64_t &rhs) {
  return lhs.get_raw() == rhs;
};

inline bool operator!=(bit_matrix lhs, const uint64_t &rhs) {
  return lhs.get_raw() != rhs;
};

#endif //CHESS_BIT_MATRIX_HPP
