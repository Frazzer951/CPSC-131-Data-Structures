#pragma once    // include guard

#include <array>
#include <cstddef>                                                                            // size_t
#include <forward_list>
#include <iostream>
#include <list>
#include <stdexcept>                                                                          // domain_error, length_error, logic_error
#include <vector>
#include <initializer_list>

#include "Book.hpp"


class BookList
{
  // Insertion and Extraction Operators
  friend std::ostream & operator<<( std::ostream & stream, const BookList & bookList );
  friend std::istream & operator>>( std::istream & stream,       BookList & bookList );

  // Relational Operators
  friend bool operator==( const BookList & lhs, const BookList & rhs );
  friend bool operator< ( const BookList & lhs, const BookList & rhs );

  public:
    // Types and Exceptions
    enum class Position {TOP, BOTTOM};

    struct InvalidInternalState_Ex : std::domain_error { using domain_error::domain_error; }; // Thrown if internal data structures become inconsistent with each other
    struct CapacityExceeded_Ex     : std::length_error { using length_error::length_error; }; // Thrown if more books are inserted than will fit
    struct InvalidOffset_Ex        : std::logic_error  { using logic_error ::logic_error;  }; // Thrown of inserting beyond current size


    // Constructors, destructor, and assignment operators
    BookList();                                                                               // construct an empty book list
                                                                                              
    BookList( const BookList  & other );                                                      // construct a book list as a copy of another book list
    BookList(       BookList && other );                                                      // construct a book list by taking the contents of another book list
                                                                                              
    BookList & operator=( BookList    rhs );                                                  // intentionally passed by value and not const ref
    BookList & operator=( BookList && rhs );                                                  
                                                                                              
    BookList             ( const std::initializer_list<Book> & initList );                    // constructs a book list from a braced list of books
    BookList & operator+=( const std::initializer_list<Book> & rhs      );                    // concatenates a braced list of books to this list
    BookList & operator+=( const BookList                    & rhs      );                    // concatenates the rhs list to the end of this list

   ~BookList();


    // Queries
    std::size_t size()                    const;
    std::size_t find( const Book & book ) const;                                              // returns the (zero-based) offset from top of list
                                                                                              // returns the (zero-based) position of the book, size() if book not found


    // Mutators
    void insert( const Book & book, Position    position = Position::TOP );                   // add the book to the top (beginning) of the book list
    void insert( const Book & book, std::size_t offsetFromTop            );                   // inserts before the existing book currently at that offset
                                                                                             
    void remove( const Book & book          );                                                // no change occurs if book not found
    void remove( std::size_t  offsetFromTop );                                                // no change occurs if (zero-based) offsetFromTop >= size()

    void moveToTop( const Book & book );

    void swap( BookList & rhs ) noexcept;                                                     // exchange one book list with another

  private:
    // Helper functions
    bool        containersAreConsistant() const;
    std::size_t books_sl_list_size()  const;                                                  // std::forward_list doesn't maintain size, so calculate it on demand

    // Instance Attributes
    std::size_t _books_array_size   = 0;                                                      // std::array's size is constant so manage that attributes ourself

    std::array       <Book, 11>  _books_array;
    std::vector      <Book    >  _books_vector;
    std::list        <Book    >  _books_dl_list;
    std::forward_list<Book    >  _books_sl_list;
};

// Relational Operators
bool operator==( const BookList & lhs, const BookList & rhs );
bool operator!=( const BookList & lhs, const BookList & rhs );

bool operator< ( const BookList & lhs, const BookList & rhs );
bool operator<=( const BookList & lhs, const BookList & rhs );
bool operator> ( const BookList & lhs, const BookList & rhs );
bool operator>=( const BookList & lhs, const BookList & rhs );
