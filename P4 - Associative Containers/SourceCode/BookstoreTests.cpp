#include <cmath>      // abs()
#include <cstdlib>    // exit()
#include <exception>
#include <iomanip>     // setprecision()
#include <iostream>    // boolalpha(), showpoint(), fixed(), unitbuf
#include <sstream>

#include "Bookstore.hpp"
#include "CheckResults.hpp"





namespace  // anonymous
{
  class Redirect
  {
    public:
      Redirect( std::ostream & from, std::ostream & to ) : originalStream( from )
      {
        from.flush();
        from.rdbuf( to.rdbuf() );
      }

      ~Redirect()
      {
        // restore the original buffer
        originalStream.flush();
        originalStream.rdbuf( originalBuffer );
      }

    private:
      std::ostream &         originalStream;
      std::streambuf * const originalBuffer = originalStream.rdbuf();
  };






  class BookstoreRegressionTest
  {
    public:
      BookstoreRegressionTest();

    private:
      void test_1( const Bookstore::Inventory_DB & inventory );
      void test_2( const Bookstore::Inventory_DB & inventory );
      void test_3( const Bookstore::Inventory_DB & inventory );
      void test_4( const Bookstore::BooksSold    & soldBooks, const Bookstore::Inventory_DB & inventory );

      void validate( const Bookstore::Inventory_DB & inventory, const Bookstore::Inventory_DB & pairs );

      Regression::CheckResults affirm;

      Bookstore::Inventory_DB expectedValues =
      {
        // "Red Baron"
        {"9789999995641", 50}, {"9991137319"   ,  9}, {"9789999513104", 20}, //{"54782169785"  , 27}, 
        {"9792430091"   , 18}, {"9991130306"   , 31},

        // Peppermint Patty
        {"9829062015"   , 30}, {"9789999710718", 23}, {"9810233094"   , 43}, {"9798184242729",  7},
        {"981012399X"   , 25}, {"9789999706124", 17}, {"9990395128"   , 41}, {"9789998470804", 27},
        {"9789998379794", 40}, {"998761776X"   , 50}, {"9810209746"   , 49}, {"9799577623217", 19},

        // Woodstock
        {"9810234910"   , 40}, {"9789999338493", 28}, {"9789998609976", 39}, {"9789999984270", 19},
        {"9983993279"   , 26}, {"9789999987653", 23}, {"9789999141079", 34}, {"9789998830035", 38},
        {"9789999902199", 40}, {"9802161748"   , 31}, {"9812040463"   , 51}, {"9789999670272", 37},
        {"9988584865"   , 13}, {"9789999706124", 17}, {"9976910584"   , 13}, {"9789999712651", 52},

        // Schroeder
        {"9999977219"   , 34}, {"9789999746892", 14}, {"9820000858"   , 35}, {"9799317207"   , 20},
        {"9810227639"   , 48}, {"9964988028"   , 26}, {"9964782578"   , 17}, {"9987617891"   , 37},
        {"9964953453"   , 10}, {"9810500246"   , 30},

        // Lucy van Pelt
        {"9964781385"   , 32}, {"9810458290"   , 23}, {"9798059840"   , 36}, {"9839629549"   , 37},
        {"9789999689182", 20}, {"9789999638722", 48}, {"9856194261"   , 47}, {"9810083262"   , 39}, 
        {"9812387285"   , 52}, {"9966960619"   , 31}, {"9844122961"   , 27}, {"9789999706124", 17}, 
        {"9789999275842", 38},

        // Charlie Brown
        {"9789998852051", 22}
      };
  } run_Bookstore_tests;






  BookstoreRegressionTest::BookstoreRegressionTest()
  {
    std::clog << std::boolalpha << std::showpoint << std::fixed << std::setprecision( 2 );

    try
    {
      // It is anticipated the implementation of the functions under test will write data to standard output. Capture and (for now)
      // ignore standard output from called functions only for the duration of these tests.  We may wish to inspect the contents of
      // this output at a future date.  These tests look and validate the function's returned parameters and side affects, but not
      // what they write to standard output.
      std::ostringstream buffer;
      Redirect           null( std::cout, buffer );    // null is an RAII object and the redirection gets reset when object is destroyed


      std::clog << "\n\n\nInventory: Associative Containers Homework Regression Test:\n";

      // Create a store and make some carts
      Bookstore theStore;
      auto      shoppingCarts = theStore.makeShoppingCarts();

      // Get a handle to this store's inventory and verify its content
      auto & inventory = theStore.inventory();
      test_1( inventory );

      // Service all the customers and verify new inventory and items sold that dipped below re-order threshold
      auto booksSold = theStore.processCustomerShoppingCarts( shoppingCarts );

      test_2( inventory );
      test_4( booksSold, inventory );

      // Change what the store carries before recording, reorder, then validate inventory content again
      inventory     .erase( "9802161748" );
      expectedValues.erase( "9802161748" );

      theStore.reorderItems( booksSold );
      test_3( inventory );

      std::clog << affirm << '\n';
    }

    catch( const std::exception & ex )
    {
      std::clog << "FAILURE:  Regression test for \"class Bookstore\" failed with an unhandled exception. \n\n\n"
                << ex.what() << std::endl;

      // Reminder:  Objects with automatic storage are not destroyed by calling std::exit().  In this case, this is okay.
      // std::exit( -__LINE__ ); // uncomment to terminate program with test failures
    }
  }






  void BookstoreRegressionTest::validate( const Bookstore::Inventory_DB & actualInventory, const Bookstore::Inventory_DB & expectedInventory )
  {
    bool allPassed = true;
    for( const auto & [expectedIsbn, expectedQuantity] : expectedInventory )   if( actualInventory.at( expectedIsbn ) != expectedQuantity )
    {
      affirm.is_equal( "Inventory item \"" + expectedIsbn + "\" quantity", expectedQuantity, actualInventory.at( expectedIsbn ) );
      allPassed = false;
    }

    if( allPassed ) affirm.is_true( "Inventory item verification", true );
  }






  void BookstoreRegressionTest::test_1( const Bookstore::Inventory_DB & inventory )
  {
    affirm.is_equal( "Inventory Item Check - Opening inventory database size", 104'002ULL, inventory.size() );

    validate( inventory, expectedValues);
  }






  void BookstoreRegressionTest::test_2( const Bookstore::Inventory_DB & inventory )
  {
    affirm.is_equal( "Inventory Item Check - Closing inventory database size", 104'002ULL, inventory.size() );

    for( auto & [isbn, quantity] : expectedValues ) --quantity;
    expectedValues["9789999706124"] -= 2;
    validate( inventory, expectedValues );
  }






  void BookstoreRegressionTest::test_3( const Bookstore::Inventory_DB & inventory )
  {
    affirm.is_equal( "Inventory Item Check - Reorder inventory database size", 104'001ULL, inventory.size() );

    for (auto & isbn : {"9789999706124", "9789999746892", "9798184242729", "9964953453", "9976910584", "9988584865", "9991137319"})
    {
      expectedValues.at( isbn ) += 20;
    }
    validate( inventory, expectedValues );
  }






  void BookstoreRegressionTest::test_4( const Bookstore::BooksSold & soldBooks, const Bookstore::Inventory_DB & inventory )
  {

    Bookstore theStore;
    auto      shoppingCarts = theStore.makeShoppingCarts();

    Bookstore::BooksSold expectedList;
    for( auto & [name, cart] : shoppingCarts ) for( auto & [isbn, book] : cart) 
    {
      if( inventory.find(isbn) != inventory.end() )  expectedList.insert( isbn );
    }

    affirm.is_equal( "Items sold today - size",    expectedList.size(), soldBooks.size() );
    affirm.is_true( "Items sold today - content", expectedList == soldBooks );


    Bookstore::BooksSold expectedBooksToReorder = { "9789999706124",
                                                    "9789999746892",
                                                    "9798184242729",
                                                    "9964953453",
                                                    "9976910584",
                                                    "9988584865",
                                                    "9991137319"
                                                  };
    Bookstore::BooksSold booksToReorder;
    for( auto & isbn : expectedList )  if( inventory.at(isbn) < 15) booksToReorder.insert(isbn);
    affirm.is_equal( "Items to reorder - count", expectedBooksToReorder.size(), booksToReorder.size() );

    affirm.is_true( "Items to reorder - content", expectedBooksToReorder == booksToReorder );
  }

} // namespace
