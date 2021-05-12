#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "Book.hpp"

int main()
{
  // Declaration of Variables
  Book                               book;
  std::vector<std::unique_ptr<Book>> books;

  // Print out user instructions
  std::cout << "Welcome to Forgotten Books, a bookstore filled with books from all nations.Place books into your shopping cart by entering each book's information.\n"
            << "  enclose string entries in quotes, separate fields with comas\n"
            << "  Enter CTL - Z( Windows ) or CTL - D( Linux ) to quit \n\n";

  // Take all books as inputs
  while( std::cout << "Enter ISBN, Title, Author, and Price\n", std::cin >> book )
  {
    books.emplace_back( std::make_unique<Book>( std::move( book ) ) );
    std::cout << "Item added to shopping cart: " << *books.back() << "\n\n";
  }

  std::cout << "Here is an itemized list of the items in your shopping cart:\n";

  // Print the books back out in reverse order
  for( int i = books.size() - 1; i >= 0; i-- )
  {
    std::cout << *books.at( i ) << std::endl;
  }

  return 0;
}
