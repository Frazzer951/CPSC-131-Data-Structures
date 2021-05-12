///////////////////////// TO-DO (1) //////////////////////////////
  /// Include necessary header files
  /// Hint:  Include what you use, use what you include
#include <fstream>
#include <iomanip>

#include "BookDatabase.hpp"
#include "Bookstore.hpp"
/////////////////////// END-TO-DO (1) ////////////////////////////





Bookstore::Bookstore( const std::string & persistenyInventoryDB )
{
  std::ifstream fin( persistenyInventoryDB );                     // Creates the stream object, and then opens the file if it can
                                                                  // The file is closed as fin goes out of scope

  ///////////////////////// TO-DO (2) //////////////////////////////
    /// The file's inventory contents consists of a quoted ISBN string followed by a quantity on hand unsigned integer separated by
    /// whitespace, like this:
    ///     "9861311467"	8
    ///     "9794201189"    46
    ///     "9971691361"    35
    ///     "9991137319"    9
    ///
    /// While no errors (eof, open failure, data type mismatch, etc,) detected, read the inventory record from the input stream and
    /// then add that information to the memory resident inventory database.
    ///
    /// Hint: Extract the quoted string and the quantity

  std::string  isbn;
  unsigned int quantity;

  while( fin >> std::quoted( isbn ) >> quantity )
  {
    _inventoryDB[isbn] = quantity;
  }

  /////////////////////// END-TO-DO (2) ////////////////////////////
}                                                                 // File is closed as fin goes out of scope







Bookstore::Inventory_DB & Bookstore::inventory()
{
  return _inventoryDB;
}







Bookstore::BooksSold Bookstore::processCustomerShoppingCarts( const ShoppingCarts & shoppingCarts )
{
  auto & worldWideBookDatabase = BookDatabase::instance();        // Get a reference to the database of all books in the world. The
                                                                  // database will contains a full description of the item and the
                                                                  // item's price.

  BooksSold todaysSales;                                          // a collection of unique ISBNs of books sold


  ///////////////////////// TO-DO (3) //////////////////////////////
    /// For each customer, print out a receipt containing a full description of books (obtained from the database of all books in
    /// the world) in their shopping cart along with the total amount due. As items are being scanned, decrement the quantity on
    /// hand for that book in the store's inventory.
    ///
    /// Hint:  Here's some pseudocode to get you started.  Create two loops, one nested inside the other, like this:
    ///        1.     For each shopping cart
    ///        1.1        Initialize the amount due to zero
    ///        1.2        For each book in the customer's shopping cart
    ///        1.2.1          If the book is not found in the database of all books in the world, indicate on the receipt it's free of charge
    ///        1.2.2          Otherwise
    ///        1.2.2.1            Print the full description on the receipt
    ///        1.2.2.2            Add the book's price to the amount due
    ///        1.2.2.3            If the book is something the store sells (the item is in the store's inventory)
    ///        1.2.2.3.1              Decrease the number of books on hand for the book sold
    ///        1.2.2.3.2              Add the book's isbn to the list of books sold today
    ///        1.3         Print the total amount due on the receipt

  for( auto & [name, cart] : shoppingCarts )
  {
    std::cout << name << "'s shopping cart contains:\n";
    double amountDue = 0;

    for( auto & [isbn, book] : cart )
    {
      Book * book_ptr = worldWideBookDatabase.find( isbn );

      if( book_ptr == nullptr )
      {
        std::cout << '\t' << isbn << "\", (" << book.title() << ") not found, book is free! \n";
      }
      else
      {
        std::cout << '\t' << *book_ptr << '\n';
        amountDue += book_ptr->price();

        if( auto it = _inventoryDB.find( book_ptr->isbn() ); it != _inventoryDB.end() )
        {
          it->second -= 1;
          todaysSales.insert( book_ptr->isbn() );
        }
      }
    }
    std::cout << std::fixed << std::setprecision( 2 ) << std::showpoint
              << '\t' << std::string( 25, '-' ) << '\n'
              << "\t Total  $" << amountDue << "\n\n\n";
  }

  /////////////////////// END-TO-DO (3) ////////////////////////////

  return todaysSales;
} // processCustomerShoppingCarts







void Bookstore::reorderItems( BooksSold & todaysSales )
{
  auto & worldWideBookDatabase = BookDatabase::instance();        // Get a reference to the database of all books in the world. The
                                                                  // database will contains a full description of the item and the
                                                                  // item's price.

  ///////////////////////// TO-DO (4) //////////////////////////////
    /// For each product that has fallen below the reorder threshold, assume an order has been placed and now the shipment has
    /// arrived. Update the store's inventory to reflect the additional items on hand.
    ///
    /// Hint:  Here's some pseudocode to get you started.
    ///        1       For each book sold today
    ///        1.1         If the book is not in the store's inventory or if the number of books on hand has fallen below the re-order threshold
    ///        1.1.1           If the book is not in the database of all books in the world,
    ///        1.1.1.1             display just the ISBN
    ///        1.1.2           Otherwise,
    ///        1.1.2.1             display the book's full description
    ///        1.1.3           If the book is not in the store's inventory
    ///        1.1.3.1             display a notice indicating the book no longer sold in this store and will not be re-ordered
    ///        1.1.4           Otherwise,
    ///        1.1.4.1             Display the current quantity on hand and the quantity re-ordered
    ///        1.1.4.2             Increase the quantity on hand by the number of items ordered and received (LOT_COUNT)
    ///        2       Reset the list of book sold today so the list can be reused again later

  std::cout << "Re-ordering books the store is running low on.\n\n";
  int i = 1;
  for( std::string isbn : todaysSales )
  {
    auto it = _inventoryDB.find( isbn );
    if( it == _inventoryDB.end() || it->second < REORDER_THRESHOLD )
    {
      auto book = worldWideBookDatabase.find( isbn );
      if( book == nullptr )
      {
        std::cout << ' ' << i << ":  {" << isbn << "}\n\n";
      }
      else
      {
        std::cout << ' ' << i << ":  {" << *book << "}\n\n";
      }
      i++;
      if( it == _inventoryDB.end() )
      { std::cout << "        *** no longer sold in this store and will not be re-ordered\n"; }
      else
      {
        std::cout << "        only " << it->second << " remain in stock which is " << ( REORDER_THRESHOLD - it->second ) << " unit(s) below reorder threshold (" << REORDER_THRESHOLD << "), re-ordering " << LOT_COUNT << " more\n";
        it->second += LOT_COUNT;
      }
    }
  }

  todaysSales.clear();

  /////////////////////// END-TO-DO (4) ////////////////////////////
}







Bookstore::ShoppingCarts Bookstore::makeShoppingCarts()
{
  // Our store has many customers, and each (identified by name) is pushing a shopping cart. Shopping carts are structured as
  // trees of trees of books.
  ShoppingCarts carts =
  {
    // first shopping cart
    { "Red Baron", { {"9789999995641", {"Mgt styles"                 }},
                     {"9991137319"   , {"Grasses of U of L"          }},
                     {"9789999513104", {"Computer Impact on Children"}},
                     {"54782169785"  , {"131 Answer Key"             }},
                     {"9792430091"   , {"Fiqh & manajemen"           }},
                     {"9991130306"   , {"Birds Atlas"                }}
                   }
    },

    // second shopping cart
    { "Peppermint Patty", { {"9829062015"   , {"Fishing in Melekeok"         }},
                            {"9789999710718", {"You Can't See Round Corners" }},
                            {"9810233094"   , {"Fuzzy logic"                 }},
                            {"9798184242729", {"Crossroads logic"            }},
                            {"981012399X"   , {"Walking balloon"             }},
                            {"9789999706124", {"Prof Bear's Math World"      }},
                            {"9990395128"   , {"Thorns & roses"              }},
                            {"9789998470804", {"Help or Hindrance"           }},
                            {"9789998379794", {"Freedom & Beyond"            }},
                            {"998761776X"   , {"Health management assessment"}},
                            {"9810209746"   , {"Waves & distributions"       }},
                            {"9799577623217", {"Have You Seen My Cat?"       }}
                          }
    },

    // third shopping cart
    { "Woodstock", { {"9810234910"   , {"Algebraic K-theory"     }},
                     {"9789999338493", {"Car Care"               }},
                     {"9789998609976", {"Money & Wealth"         }},
                     {"9789999984270", {"Our 2 Minds"            }},
                     {"9983993279"   , {"Call me"                }},
                     {"9789999987653", {"Who's Who"              }},
                     {"9789999141079", {"Folk Songs"             }},
                     {"9789998830035", {"Org Dev"                }},
                     {"9789999902199", {"Be Outstanding"         }},
                     {"9802161748"   , {"Wild mammals"           }},
                     {"9812040463"   , {"Dirty dozen"            }},
                     {"9789999670272", {"Breaking Promises"      }},
                     {"9988584865"   , {"Ghost names"            }},
                     {"9789999706124", {"Prof Bear's Math World" }},
                     {"9976910584"   , {"Cooking with vegetables"}},
                     {"9789999712651", {"Advertising Secrets"    }}
                   }
    },

    // forth shopping cart
    { "Schroeder", { {"9999977219"   , {"Sound of Her Laugh"    }},
                     {"9789999746892", {"Children & TV"         }},
                     {"9820000858"   , {"Fish Manual"           }},
                     {"9799317207"   , {"Upland agriculture"    }},
                     {"9810227639"   , {"Advances in robotics"  }},
                     {"9964988028"   , {"Beyond the rivers"     }},
                     {"9964782578"   , {"Delivered through rain"}},
                     {"9987617891"   , {"Women's participation"}},
                     {"9964953453"   , {"Ile of No Return"      }},
                     {"9810500246"   , {"Secret goldfish"       }}
                   }
    },

    // fifth shopping cart
    { "Lucy van Pelt", { {"9964781385"   , {"Violence"                     }},
                         {"9810458290"   , {"xKnowledge capital investments"}},
                         {"9798059840"   , {"Economic assessment"          }},
                         {"9839629549"   , {"Hawkers delight"              }},
                         {"9789999689182", {"Sacred Deposit"               }},
                         {"9789999638722", {"12 Crimes of Christmas"       }},
                         {"9856194261"   , {"Scales and consequences"      }},
                         {"9810083262"   , {"2nd chance"                   }},
                         {"9812387285"   , {"Energy and geometry"          }},
                         {"9966960619"   , {"A better bonfire"             }},
                         {"9844122961"   , {"Looking back"                 }},
                         {"9789999706124", {"Prof Bear's Math World"       }},
                         {"9789999275842", {"Billboard Music Week"         }}
                       }
    },

    // sixth shopping cart
    { "Charlie Brown", { {"9789998852051", {"From the Coal Mines"}}
                       }
    }
  };

  return carts;
}  // makeShoppingCarts
