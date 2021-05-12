#include <cmath>       // abs(), ceil(), log10()
#include <exception>
#include <iomanip>     // setprecision()
#include <iostream>    // boolalpha(), showpoint(), fixed()
#include <sstream>

#include "Book.hpp"
#include "CheckResults.hpp"




namespace  // anonymous
{
  constexpr auto EPSILON = 1E-4;

  class BookRegressionTest
  {
    public:
      BookRegressionTest();

    private:
      void construction();
      void io          ();
      void comparison  ();

      Regression::CheckResults affirm;
  } run_book_tests;




  void BookRegressionTest::construction()
  {
    Book b1,
         b2( "book's title"                                          ),
         b3( "book's title",  "book's author"                        ),
         b4( "book's title",  "book's author", "book's ISBN"         ),
         b5( "book's title",  "book's author", "book's ISBN", 123.79 );

    affirm.is_equal( "Default price", b1.price(), 0.0 );

    b1.title( "1-3-5" );
    affirm.is_equal( "Symmetrical Title mutator/query", "1-3-5", b1.title() );

    b1.author( "21-23-25" );
    affirm.is_equal( "Symmetrical Author mutator/query", "21-23-25", b1.author() );

    b1.isbn( "11-13-15" );
    affirm.is_equal( "Symmetrical ISBN code mutator/query", "11-13-15", b1.isbn() );

    b1.price( 31.11 );
    affirm.is_equal( "Symmetrical Price mutator/query", 31.11, b1.price() );

    affirm.is_true
    ( "Book construction with arguments",
           b2.title() == "book's title"
        && b3.title() == "book's title" && b3.author() == "book's author"
        && b4.title() == "book's title" && b4.author() == "book's author" && b4.isbn() == "book's ISBN"
        && b5.title() == "book's title" && b5.author() == "book's author" && b5.isbn() == "book's ISBN" && std::abs(b5.price() - 123.79) < EPSILON
     );

    Book b6( b5 );
    affirm.is_true
    ("Copy construction",
          b6.title()    ==  b5.title()
       && b6.author()   ==  b5.author()
       && b6.isbn()     ==  b5.isbn()
       && std::abs(b6.price() - b5.price()) < EPSILON
    );

    b6 = b4;
    affirm.is_true
    ("Copy construction",
          b6.title()    ==  b4.title()
       && b6.author()   ==  b4.author()
       && b6.isbn()     ==  b4.isbn()
       && std::abs(b6.price() - b4.price()) < EPSILON
    );
  }



  void BookRegressionTest::io()
  {
    {  // Input parsing
      std::istringstream stream( R"~~( "0000171921","Linux commands & resources directory - 1st ed","Howard L. Goldberg",56.69

                                        "0000370096", "Where eagles dare"             , 
                                        "Norah Woollard"       , 
                                        118.07

                                        "0001005340"    , 
                                        "Little Grey Rabbit's Birthday", 
                                        "Alison \"Ally\" Uttley", 
                                        31.57 "0001062417", 
                                        "Early aircraft", "Allward, Maurice F.",
                                          65.65

                                        "0001051776" , "The  poems of Rudyard Kipling", "Rudyard Kipling", 26.45

                                        "00000000000000", "incomplete / invalid book"
                                 )~~" );

      Book t1, t2, t3, t4, t5, t6;
      stream >> t1 >> t2 >> t3 >> t4 >> t5 >> t6;

      affirm.is_equal( "Book input parsing 1", Book { "Linux commands & resources directory - 1st ed", "Howard L. Goldberg",      "0000171921", 56.69  }, t1 );
      affirm.is_equal( "Book input parsing 2", Book { "Where eagles dare",                             "Norah Woollard",          "0000370096", 118.07 }, t2 );
      affirm.is_equal( "Book input parsing 3", Book { "Little Grey Rabbit's Birthday",                 "Alison \"Ally\" Uttley",  "0001005340", 31.57  }, t3 );
      affirm.is_equal( "Book input parsing 4", Book { "Early aircraft",                                "Allward, Maurice F.",     "0001062417", 65.65  }, t4 );
      affirm.is_equal( "Book input parsing 5", Book { "The  poems of Rudyard Kipling",                 "Rudyard Kipling",         "0001051776", 26.45  }, t5 );

      affirm.is_equal( "Book input parsing 6", Book(), t6 );
    }
    {  // read what you write
      Book b1,
           b2( "book's title" ),
           b3( "book's title", "book's author" ),
           b4( "book's title", "book's author", "book's ISBN" ),
           b5( "book's title", "book's author", "book's ISBN", 123.79 );

      std::stringstream stream;

      stream << b1 << '\n'
             << b2 << '\n'
             << b3 << '\n'
             << b4 << '\n'
             << b5;

      Book t1, t2, t3, t4, t5;
      stream >> t1 >> t2 >> t3 >> t4 >> t5;

      affirm.is_equal( "Symmetrical Book insertion and extraction 1", b1, t1 );
      affirm.is_equal( "Symmetrical Book insertion and extraction 2", b2, t2 );
      affirm.is_equal( "Symmetrical Book insertion and extraction 3", b3, t3 );
      affirm.is_equal( "Symmetrical Book insertion and extraction 4", b4, t4 );
      affirm.is_equal( "Symmetrical Book insertion and extraction 5", b5, t5 );
    }
  }



  void BookRegressionTest::comparison()
  {
    Book less( "a1", "a1", "a1", 10.0 ), more(less);

    // Be careful - using affirm.xxx() may hide the class-under-test overloaded operators.  But affirm.is_true() doesn't provide as
    // much information when the test fails.
    affirm.is_equal    ( "Equality test - is equal                   ", less, more );
    affirm.is_equal    ( "Equality test - floating point lower limit ", less, Book {"a1", "a1", "a1", less.price() - EPSILON} );
    affirm.is_equal    ( "Equality test - floating point upper limit ", less, Book {"a1", "a1", "a1", less.price() + EPSILON} );

    affirm.is_not_equal( "Inequality Title test                      ", less, Book {"b1", "a1", "a1", 10.0} );
    affirm.is_not_equal( "Inequality Author test                     ", less, Book {"a1", "b1", "a1", 10.0} );
    affirm.is_not_equal( "Inequality ISBN test                       ", less, Book {"a1", "a1", "b1", 10.0} );
    affirm.is_not_equal( "Inequality Price test - lower limit        ", less, Book {"a1", "a1", "a1", less.price() - ( 2 * EPSILON )} );
    affirm.is_not_equal( "Inequality Price test - upper limit        ", less, Book {"a1", "a1", "a1", less.price() + ( 2 * EPSILON )} );


    auto check = [&]()
    {
      // Exercise all relational operators for the class-under-test.
      return less <  more
          && less <= more
          && more >  less
          && more >= less;
    };

    more = {"a1", "a1", "a1", 11.0};
    affirm.is_true( "Relational Price test                      ", check() );

    more = {"a2", "a1", "a1", 9.0};
    affirm.is_true( "Relational Title test                      ", check() );

    more = {"a0", "a2", "a1", 9.0};
    affirm.is_true( "Relational Author test                     ", check() );

    more = {"a0", "a0", "a2", 9.0};
    affirm.is_true( "Relational ISBN test                       ", check() );
  }



  BookRegressionTest::BookRegressionTest()
  {
    std::clog << std::boolalpha << std::showpoint << std::fixed << std::setprecision( 2 );


    try
    {
      std::clog << "\nBook Regression Test:  Construction\n";
      construction();

      std::clog << "\nBook Regression Test:  Relational comparisons\n";
      auto previousPrecision = std::clog.precision( static_cast<std::streamsize>( std::ceil( -std::log10( EPSILON ) ) ) );
      comparison();
      std::clog.precision( previousPrecision );

      std::clog << "\nBook Regression Test:  Input/Output\n";
      io ();

      std::clog << affirm << '\n';
    }
    catch( const std::exception & ex )
    {
      std::clog << "FAILURE:  Regression test for \"class Book\" failed with an unhandled exception. \n\n\n"
                << ex.what() << std::endl;
    }
  }
} // namespace
