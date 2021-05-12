#include <exception>
#include <iomanip>     // setprecision()
#include <iostream>    // boolalpha(), showpoint(), fixed()
#include <string>      // to_string()

#include "CheckResults.hpp"
#include "BookList.hpp"

namespace    // anonymous
{
  class BookListRegressionTest
  {
    public:
      BookListRegressionTest();

    private:
      void test();

      Regression::CheckResults affirm;
  } run_booklist_tests;




  void BookListRegressionTest::test()
  {
    const Book book_1( "book_1" ),
               book_2( "book_2" ),
               book_3( "book_3" ),
               book_4( "book_4" ),
               book_5( "book_5" ),
               book_6( "book_6" );

    {
      BookList list;
      affirm.is_equal( "Default construction:  Size", 0U, list.size() );
    }

    {
      BookList list;
      list.insert( book_3                             );
      list.insert( book_4, BookList::Position::BOTTOM );
      list.insert( book_1, 1                          );
      list.insert( book_2                             );

      BookList expected = {book_2, book_3, book_1, book_4};

      affirm.is_equal( "Initializer list constructor:  Size",    4U,       list.size() );
      affirm.is_equal( "Initializer list constructor:  content", expected, list        );
    }

    {
      BookList list1 = {book_2, book_3, book_1, book_4};
      list1 += {book_3, book_1, book_2, book_5};

      BookList list2 = {book_2, book_6, book_1, book_4};
      list1 += list2;

      BookList expected = {book_2, book_3, book_1, book_4, book_5, book_6};

      affirm.is_equal( "Initializer list concatenation:  Size",    6U,       list1.size() );
      affirm.is_equal( "Initializer list concatenation:  content", expected, list1        );

      affirm.is_equal( "Search - book_2",    0U, list1.find( book_2        ) );
      affirm.is_equal( "Search - book_3",    1U, list1.find( book_3        ) );
      affirm.is_equal( "Search - book_1",    2U, list1.find( book_1        ) );
      affirm.is_equal( "Search - book_4",    3U, list1.find( book_4        ) );
      affirm.is_equal( "Search - book_5",    4U, list1.find( book_5        ) );
      affirm.is_equal( "Search - book_6",    5U, list1.find( book_6        ) );
      affirm.is_equal( "Search - not there", 6U, list1.find( {"not there"} ) );
    }

    {
      BookList list = {book_2, book_1, book_4, book_5, book_6};

      list.remove( book_1 );
      affirm.is_equal( "Remove by Book - middle", BookList {book_2, book_4, book_5, book_6}, list );

      list.remove( book_6 );
      affirm.is_equal( "Remove by Book - bottom", BookList {book_2, book_4, book_5}, list );

      list.remove( book_2 );
      affirm.is_equal( "Remove by Book - top", BookList {book_4, book_5}, list );

      list.remove( {"not there"} );
      affirm.is_equal( "Remove by Book - not there", BookList {book_4, book_5}, list );
    }

    {
      BookList list = {book_2, book_1, book_4, book_5, book_6};

      list.remove( 1 );
      affirm.is_equal( "Remove by position - middle", BookList {book_2, book_4, book_5, book_6}, list );

      list.remove( 3 );
      affirm.is_equal( "Remove by position - bottom", BookList {book_2, book_4, book_5}, list );

      list.remove( 0 );
      affirm.is_equal( "Remove by position - top", BookList {book_4, book_5}, list );

      list.remove( 10 );
      affirm.is_equal( "Remove by position - bad index", BookList {book_4, book_5}, list );
    }

    {
      BookList list = {book_2, book_1, book_4, book_5, book_6};

      list.moveToTop( book_5        );
      list.moveToTop( book_6        );
      list.moveToTop( book_6        );
      list.moveToTop( book_5        );
      list.moveToTop( book_4        );
      list.moveToTop( {"not there"} );

      BookList expected = {book_4, book_5, book_6, book_2, book_1};
      affirm.is_equal( "Move to top", expected, list );
    }

    {
      BookList list;

      try
      {
        for( unsigned i = 0; i < 100; ++i ) list.insert( Book{ "Book-" + std::to_string( i ) } );
        affirm.is_true( "Fixed size array capacity check", false );
      }
      catch ( const BookList::CapacityExceeded_Ex & )  // expected
      {
        affirm.is_true( "Fixed size array capacity check", true );
      }
    }
  }




  BookListRegressionTest::BookListRegressionTest()
  {
    std::clog << std::boolalpha << std::showpoint << std::fixed << std::setprecision( 2 );


    try
    {
      std::clog << "\nBook List Regression Tests:\n";
      test();

      std::clog << affirm << '\n';
    }
    catch( const std::exception & ex )
    {
      std::clog << "FAILURE:  Regression test for \"class BookList\" failed with an unhandled exception. \n\n\n"
                << ex.what() << std::endl;
    }
  }
}    // namespace
