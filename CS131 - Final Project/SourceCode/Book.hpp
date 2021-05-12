#pragma once    // include guard

#include <iostream>
#include <string>
#include <string_view>




class Book
{
  // Insertion and Extraction Operators
  friend std::ostream & operator<<( std::ostream & stream, const Book & book );
  friend std::istream & operator>>( std::istream & stream,       Book & book );

  // Relational Operators
  friend bool operator==( const Book & lhs, const Book & rhs );
  friend bool operator< ( const Book & lhs, const Book & rhs );

  public:
    // Constructors
    Book( std::string_view title  = {},
          std::string_view author = {},
          std::string_view isbn   = {},
          double           price  = 0.0 );

    // Queries
    std::string isbn  () const;
    std::string title () const;
    std::string author() const;
    double      price () const;

    // Mutators
    void isbn  ( std::string_view newIsbn   );
    void title ( std::string_view newTitle  );
    void author( std::string_view newAuthor );
    void price ( double           newPrice  );

  private:
    std::string _isbn;
    std::string _title;
    std::string _author;
    double      _price = 0.0;
};

// Relational Operators
bool operator==( const Book & lhs, const Book & rhs );
bool operator!=( const Book & lhs, const Book & rhs );

bool operator< ( const Book & lhs, const Book & rhs );
bool operator<=( const Book & lhs, const Book & rhs );
bool operator> ( const Book & lhs, const Book & rhs );
bool operator>=( const Book & lhs, const Book & rhs );
