#pragma once

#include <map>
#include <set>
#include <string>

#include "Book.hpp"



class Bookstore
{
  public:
    // Type Definition Aliases
    //    |Alias Name |            |  Key             |  | Value                 |
    //    +-----------+            +------------------+  +-----------------------+
    using BooksSold     = std::set<std::string /*ISBN*/  /* N/A */                >;  // A collection of ISBNs for books that have been sold
    
    using Inventory_DB  = std::map<std::string /*ISBN*/, unsigned int /*quantity*/>;  // Maintains of the quantity of books in stock identified by ISBN
    using ShoppingCart  = std::map<std::string /*ISBN*/, Book                     >;  // An individual shopping cart filled with books
    using ShoppingCarts = std::map<std::string /*name*/, ShoppingCart             >;  // A collection if shoppers, identified by
                                                                                      // name, each pushing a shopping cart.  Notice
                                                                                      // that this structure is a tree, and each
                                                                                      // element in the tree is also a tree. That
                                                                                      // is, this is a tree of trees.

    // Constructors, assignments, destructor  
    Bookstore( const std::string & persistenyInventoryDB = "BookstoreInventory.dat" );

    // Queries
    Inventory_DB & inventory();                                                       // Returns a reference to the store's one and only inventory database


    // Operations
    
    // Each customer, in turn, places their books on the checkout counter where they are scanned, paid for, and a receipt issued.
    // Returns a collection of ISBNs for books that have been sold
    BooksSold processCustomerShoppingCarts( const ShoppingCarts & shoppingCarts );
    
    // Re-orders books sold that have fallen below the re-order threshold, then clears the reorder list
    void reorderItems( BooksSold & todaysSales );

    // Initializes a bunch of customers pushing shopping carts filled with groceries
    ShoppingCarts  makeShoppingCarts();

  
  private:
    // Class attributes
    inline static constexpr unsigned int REORDER_THRESHOLD = 15;    // When the quantity on hand dips below this threshold, it's time to order more inventory
    inline static constexpr unsigned int LOT_COUNT         = 20;    // Number of items that can be ordered at one time

    // Instance attributes
    Inventory_DB       _inventoryDB;
};
