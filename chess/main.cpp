#include <iostream>
#include "piece.hpp"
#include "bit_matrix.hpp"

int main() {


  chess_piece p{KING, WHITE};

  if (p.is_white())
    std::cout << "Itsa white" << std::endl;

  bit_matrix b;
  b.print();

  // b.set_bit_at(5);
  std::cout << std::endl << std::endl;

  b.set_bit_at(7, 5);
  b.set_bit_at(7, 4);
  b.set_bit_at(1, 4);
  b.unset_bit_at(1, 4);
  b.print();
  std::cout << "Num bits set: " << (int) b.get_num_bits_set() << std::endl;

  return 0;
}