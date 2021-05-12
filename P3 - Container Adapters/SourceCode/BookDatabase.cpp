///////////////////////// TO-DO (1) //////////////////////////////
  /// Include necessary header files
  /// Hint:  Include what you use, use what you include
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>

#include "Book.hpp"
#include "BookDatabase.hpp"
/////////////////////// END-TO-DO (1) ////////////////////////////



// Return a reference to the one and only instance of the database
BookDatabase & BookDatabase::instance()
{
  std::string filename;
  // Don't forget to #include <filesystem> to get visibility to the exists() function
  //
  // Look for a prioritized list of database files in the current working directory to use
  if     ( std::filesystem::exists( "Open Library Database-Full.dat"   ) )filename = "Open Library Database-Full.dat";  
  else if( std::filesystem::exists( "Open Library Database-Large.dat"  ) )filename = "Open Library Database-Large.dat"; 
  else if( std::filesystem::exists( "Open Library Database-Medium.dat" ) )filename = "Open Library Database-Medium.dat";
  else if( std::filesystem::exists( "Open Library Database-Small.dat"  ) )filename = "Open Library Database-Small.dat";
  else if( std::filesystem::exists( "Sample_Book_Database.dat"         ) )filename = "Sample_Book_Database.dat";


  static BookDatabase theInstance( filename );
  return theInstance;
}




// Construction
BookDatabase::BookDatabase( const std::string & filename )
{
  std::ifstream fin( filename, std::ios::binary );


  ///////////////////////// TO-DO (2) //////////////////////////////
    /// The file contains Books separated by whitespace.  A Book has 4 pieces of data delimited with a comma.  (This exactly matches
    /// the previous assignment as to how Books are read) 
    ///
    ///       Field            Type            Notes
    ///  1.   Book's ISBN      String          Unique identifier (primary key), always enclosed in double quotes
    ///  2.   Book's Title     String          May contain spaces, always enclosed in double quotes
    ///  3.   Book's Author    String          May contain spaces, always enclosed in double quotes
    ///  4.   Book's Price     Floating Point  In dollars
    ///
    ///  Example:
    ///    "0001062417",  "Early aircraft",                 "Maurice F. Allward", 65.65
    ///    "0000255406",  "Shadow maker \"1st edition)\"",  "Rosemary Sullivan",   8.08
    ///    "0000385264",  "Der Karawanenkardinal",          "Heinz Gstrein",      35.18
    ///
    ///  Note: double quotes within the string are escaped with the backslash character
    ///
  
  Book book;
  while( fin >> book )
  {
    _book_database.push_back( book );
  }

  /////////////////////// END-TO-DO (2) ////////////////////////////

  // Note:  The file is intentionally not explicitly closed.  The file is closed when fin goes out of scope - for whatever
  //        reason.  More precisely, the object named "fin" is destroyed when it goes out of scope and the file is closed in the
  //        destructor. See RAII
}


///////////////////////// TO-DO (3) //////////////////////////////
  /// Implement the rest of the interface, including functions find (recursively) and size
  ///
  /// Programming note:  An O(n) operation, like searching an unsorted vector, would not generally be implemented recursively.  The
  ///                    depth of recursion may be greater than the program's function call stack size.  But for this programming
  ///                    exercise, getting familiar with recursion is a goal.

Book * BookDatabase::find( const std::string & isbn )
{
  return find( isbn, _book_database.begin() );
}

Book * BookDatabase::find( const std::string & isbn, const std::vector<Book>::iterator & cur_pos )
{
  if( cur_pos == _book_database.end() ) return nullptr;

  if( isbn == cur_pos->isbn() )
  {
    Book * book_ptr = new Book( *cur_pos );
    return book_ptr;
  }

  return find( isbn, cur_pos + 1 );
}

std::size_t BookDatabase::size() const
{
  return _book_database.size();
}

/////////////////////// END-TO-DO (3) ////////////////////////////
