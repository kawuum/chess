#include <iostream>
#include "piece.hpp"
#include "bit_matrix.hpp"
#include <board.hpp>

int main() {


  piece p{KING, WHITE};

  if (p.is_white())
    std::cout << "Itsa white" << std::endl;

  bit_matrix b;
  board boa;
  b.print();

  // b.set_bit_at(5);
  std::cout << std::endl << std::endl;

  boa.addPiece(p, 2, 3);
  b.set_bit_at(7, 5);
  b.set_bit_at(7, 4);
  b.set_bit_at(1, 4);
  b.unset_bit_at(1, 4);
  b.print();
  std::cout << "Num bits set: " << (int) b.get_num_bits_set() << std::endl << std::endl;
  
  boa.get_bit_matrix_for_piece(p).print();
  std::cout << "Num of pieces: " << (int) boa.get_num_pieces() << std::endl;
  std::cout << "Board is " << (boa.is_empty() ? "" : "not ") << "empty" << std::endl;
  boa.clear();
  std::cout << "Board is now " << (boa.is_empty() ? "" : "still not ") << "empty" << std::endl;

  return 0;
}
