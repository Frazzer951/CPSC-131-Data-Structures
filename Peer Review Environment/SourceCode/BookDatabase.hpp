#pragma once

#include <cstddef>   // size_t
#include <string>
#include <map>

#include "Book.hpp"



// Singleton Design Pattern
class BookDatabase
{
  public:
    // Get a reference to the one and only instance of the database
    static BookDatabase & instance();

    // Locate and return a reference to a particular record
    Book * find( const std::string & isbn );                                    // Returns a pointer to the item in the database if 
                                                                                // found, nullptr otherwise
    // Queries
    std::size_t size() const;                                                   // Returns the number of items in the database

  private:
    BookDatabase            ( const std::string  & filename );
    BookDatabase            ( const BookDatabase &          ) = delete;         // intentionally prohibit making copies
    BookDatabase & operator=( const BookDatabase &          ) = delete;         // intentionally prohibit copy assignments
    
    // Private implementation details
    std::map<std::string /*ISBN*/, Book> _data;
};
