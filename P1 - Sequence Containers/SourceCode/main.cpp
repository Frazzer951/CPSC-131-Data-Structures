#include <exception>
#include <iostream>
#include <typeinfo>

#include "Book.hpp"
#include "BookList.hpp"




namespace
{
  void basicScenario()
  {
    // Let's start a book list
    BookList booksToRead = {{"Inviable Man"           },
                            {"Moby Dick"              },
                            {"Les Mis"                },
                            {"A Tale of Two Cities"   }};

    // Changed my mind, I want to make sure I read Les Mis first so I'm going to move that to the top of my list
    booksToRead.moveToTop( {"Les Mis"} );

    // Let's see what's on the list so far
    std::cout << "My Book List" << booksToRead << "\n\n";

    // Yay, no.  Need to add a few more
    booksToRead += {{"Ulysses"          },
                    {"The Color Purple" }};


    // My roommate also has a book list
    BookList roommatesList = {{"Hunger Games", "S. Collins"               },
                              {"Hunger Games", "Suzanne Collins"          },
                              {"Les Mis"                                  },
                              {"Secret Garden", "Frances Hodgson Burnett" },
                              {"Charlie & Choc Factory"                   }};

    std::cout << "My roommate's Book List" << roommatesList << "\n\n";

    // Let's combine the lists (if different) and go shopping
    if( booksToRead != roommatesList ) booksToRead += roommatesList;


    // Oops, forgot one of my favorites.  Let's add that right before '2 cities.
    booksToRead.insert( {"Oliver Twist", "Charles Dickens"}, booksToRead.find( {"A Tale of Two Cities"} ) );

    // Time is short, let me take one off the list!
    booksToRead.remove( {"Secret Garden", "Frances Hodgson Burnett"} );


    // Now let's see what's on the list
    std::cout << "Combined Book Lists" << booksToRead << "\n\n";



    // Did you get it right?
    BookList expectedResults = {{"Les Mis"                         },
                                {"Inviable Man"                    },
                                {"Moby Dick"                       },
                                {"Oliver Twist", "Charles Dickens" },
                                {"A Tale of Two Cities"            },
                                {"Ulysses"                         },
                                {"The Color Purple"                },
                                {"Hunger Games", "S. Collins"      },
                                {"Hunger Games", "Suzanne Collins" },
                                {"Charlie & Choc Factory"          }};

    std::cout << "\nExpected results:" << expectedResults                                        << "\n\n"
              << "\nActual results:"   << booksToRead                                            << "\n\n"
              << "\nTest results:  "   << ( booksToRead == expectedResults  ? "PASS"  :  "FAIL") << '\n';
  }
}




int main()
{
  try
  {
    basicScenario();

    
    ///////////////////////// TO-DO (1) //////////////////////////////
     /// Create, manipulate, and display your own BookList object here.  Not looking for anything specific but expect you to
     /// demonstrate you can use the BookList class to create, update, and display the contents of a book list.

    BookList lukesBooks = {{ "Book 1"       },
                           { "Book 21"      },
                           { "Another book" },
                           { "Book 2"       }};

    std::cout << "My Book List" << lukesBooks << "\n\n";

    lukesBooks.moveToTop( { "Book 2" } );

    std::cout << "My Book List with 'Book 2' at the top" << lukesBooks << "\n\n";

    lukesBooks += {{ "Book 10"      },
                   { "Harry Potter" }};

    std::cout << "My Book List with 'Book 10' and 'Harry Potter' added" << lukesBooks << "\n\n";

    lukesBooks.remove( { "Book 1" } );
    lukesBooks.insert( { "Cooking 101" }, lukesBooks.find( { "Another book" } ) );

    std::cout << "My Book List with 'book 1' removed and 'Cooking 101' before 'Another book'" << lukesBooks << "\n\n";

    /////////////////////// END-TO-DO (1) ////////////////////////////
  }

  catch( const std::exception & ex )
  {
    std::cerr << "Fatal Error:  unhanded exception \"" << typeid( ex ).name() << "\"\n"
              << ex.what() << '\n';
  }
}
