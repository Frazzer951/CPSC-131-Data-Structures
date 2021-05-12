#include <cmath>      // abs()
#include <iomanip>    // quoted()
#include <iostream>
#include <string>
#include <string_view>
#include <utility>    // move()

#include "Book.hpp"

const double EPSILON = 0.0001;

// Constructor
Book::Book( std::string_view title, std::string_view author, std::string_view isbn, double price ) : _isbn( isbn ), _title( title ), _author( author ), _price( price ) {}

// Queries
std::string Book::isbn() const { return _isbn; }
std::string Book::title() const { return _title; }
std::string Book::author() const { return _author; }
double      Book::price() const { return _price; }

// Mutators
void Book::isbn( std::string_view newIsbn ) { _isbn = newIsbn; }
void Book::title( std::string_view newTitle ) { _title = newTitle; }
void Book::author( std::string_view newAuthor ) { _author = newAuthor; }
void Book::price( double newPrice ) { _price = newPrice; }

// Insertion and Extraction Operators
std::ostream & operator<<( std::ostream & stream, const Book & book )
{
  const std::string delimiter = ", ";

  stream << std::quoted( book.isbn() ) << delimiter
         << std::quoted( book.title() ) << delimiter
         << std::quoted( book.author() ) << delimiter
         << book.price();

  return stream;
}

std::istream & operator>>( std::istream & stream, Book & book )
{
  Book temp;
  char delimiter = ',';

  stream >> std::quoted( temp._isbn ) >> delimiter
      >> std::quoted( temp._title ) >> delimiter
      >> std::quoted( temp._author ) >> delimiter
      >> temp._price;

  if( stream ) book = std::move( temp );
  return stream;
}

// Relational Operators
bool operator==( const Book & lhs, const Book & rhs )
{
  return ( lhs._isbn == rhs._isbn ) && ( lhs._title == rhs._title ) && ( lhs._author == rhs._author ) && ( std::abs( lhs._price - rhs._price ) < EPSILON );
}

bool operator<( const Book & lhs, const Book & rhs )
{
  if( auto result = lhs._isbn.compare( rhs._isbn ); result != 0 ) return result < 0;
  if( auto result = lhs._author.compare( rhs._author ); result != 0 ) return result < 0;
  if( auto result = lhs._title.compare( rhs._title ); result != 0 ) return result < 0;

  if( std::abs( lhs._price - rhs._price ) >= EPSILON ) return lhs._price < rhs._price;

  return false;
}

bool operator!=( const Book & lhs, const Book & rhs ) { return !( lhs == rhs ); }

bool operator<=( const Book & lhs, const Book & rhs ) { return !( rhs < lhs ); }

bool operator>( const Book & lhs, const Book & rhs ) { return ( rhs < lhs ); }

bool operator>=( const Book & lhs, const Book & rhs ) { return !( lhs < rhs ); }
