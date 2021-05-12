/******************************************************************************
** (C) Copyright 2015 by Thomas Bettens. All Rights Reserved.
**
** DISCLAIMER: The authors have used their best efforts in preparing this
** code. These efforts include the development, research, and testing of the
** theories and programs to determine their effectiveness. The authors make no
** warranty of any kind, expressed or implied, with regard to these programs or
** to the documentation contained within. The authors shall not be liable in
** any event for incidental or consequential damages in connection with, or
** arising out of, the furnishing, performance, or use of these libraries and
** programs.  Distribution without written consent from the authors is
** prohibited.
******************************************************************************/


#ifndef RATIONAL_ARRAY_HPP
#define RATIONAL_ARRAY_HPP

#include <cstddef>                // std::size_t
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <type_traits>            // enable_if, is_arithmetic

#include "Library/Rational.hpp"
#include "Utilities/Exceptions.hpp"




namespace Library
{
  class RationalArray
  {
    /********************************************
    * Friends
    ********************************************/
    // Friends are not member functions, so accessibility (public, private) does not apply
    friend void swap( RationalArray & lhs, RationalArray & rhs ) noexcept;

    friend std::ostream & operator<<  (std::ostream & stream, const RationalArray & r);      // Throws everything operator<<(std::istream & stream, Rational) does


    public:
      /********************************************
      * Inner Types
      ********************************************/
      using Size     = std::size_t;                // Similar to std::vector's size_type
      using Index    = std::size_t;
      using Element  = Rational *;                 // Similar to std::vector's value_type that represents the data type stored in the container


      // Exception Type Hierarchy Definition
      struct RationalArrayExceptions     : Utilities::AbstractException<>   {using AbstractException::AbstractException ;};             // Class RationalArray exception base class
      struct   InsertionFailure          : RationalArrayExceptions          {using RationalArrayExceptions::RationalArrayExceptions;};  // Inherit base class constructors
      struct   ConstructionFailure       : RationalArrayExceptions          {using RationalArrayExceptions::RationalArrayExceptions;};
      struct   CapacityAdjustmentFailure : RationalArrayExceptions          {using RationalArrayExceptions::RationalArrayExceptions;};
      struct   OverIndex                 : RationalArrayExceptions          {using RationalArrayExceptions::RationalArrayExceptions;};
      struct   RemovalFailure            : RationalArrayExceptions          {using RationalArrayExceptions::RationalArrayExceptions;};
      struct   ReplacementFailure        : RationalArrayExceptions          {using RationalArrayExceptions::RationalArrayExceptions;};



      /********************************************
      * Constructors, destructor
      ********************************************/
      RationalArray()                                     noexcept;        // default array construction
      RationalArray( const RationalArray  & otherArray );                  // copy array construction
      RationalArray(       RationalArray && otherArray )  noexcept;        // move array construction


      // Construct an array filled with "howMany" copies of the provided Rational number. Also acts as a conversion constructor
      // converting a Rational number into an array of one Rational number.
      explicit RationalArray( const Rational & rational, const Size  & howMany = 1 );


      // Construct an array filled with copies of the Rational numbers within the other array from [startingAt .. startingAt +
      // howMany). If the array is shorter than "howMany", as many elements as possible are used. Similar to std::string::substr()
      RationalArray( const RationalArray  & otherArray, const Index & startingAt, const Size & howMany );


      // Construct an array from an initializer list of Rational numbers. During constructor overload resolution, braced
      // initializers are matched to this function if at all possible, even if other constructors offer seemingly better matches.
      //    [[ Meyers, Effective Modern C++, 2014, Item 7 ]]
      // User should be aware that
      //    RationalArray A1{5, 4} produces an array of two elements whose contents are {5,1} and {4, 1} RationalArray A2(5, 4)
      //    produces an array of four elements, each having a value of {5, 1}
      RationalArray( const std::initializer_list<Rational> & theList );


      // Construct a RationalArray filled with copies of Rational numbers pointed to by the iterators [first .. last). Instantiate
      // for iterators and pointers, but not for the arithmetic types (there is another constructor for a pair of arithmetic types)
      //    [[ Meyers, Effective Modern C++, 2014, Item 27 ]]
      template <typename InputIterator, typename = std::enable_if_t< !std::is_arithmetic<InputIterator>::value >>
      RationalArray( InputIterator first, InputIterator last );


      // Generalized copy constructor from any container type (other than RationalArray, of course) holding Rational numbers
      template <typename Container>
      explicit RationalArray( const Container & otherContainer );

      ~RationalArray();



      /********************************************
      * RationalArray iterators
      ********************************************/
      // Native pointers, also called raw pointers, are bi-directional, random access iterators when used to access elements of a
      // native array. A simple approach is to define RationalArray iterators as raw pointers, and keeping things simple is quite
      // attractive in many cases. Raw pointers are very quick and easy to use, but they are also easy to use wrong. As class
      // designers we want to make our classes easy to use and hard to use wrong. By making our iterators a little smarter, we can
      // protect against null pointer accesses, exceeding accessing elements beyond the container, and so on. Start out by
      // implementing your iterators as raw pointers, then as time permits, explore some additional C++ features by implementing
      // your iterators as classes with overloaded operators. See below.
      //
      // Note that STL iterators are typically smart, but not brilliant. That is, they protect against dereferencing null
      // iterators, but do not usually protect against accessing elements beyond the container. That said, most compiler vendors
      // make STL iterators smart in "release" configurations, and brilliant in "debug" configurations.
      class iterator;                  // Similar to std::vector's iterator to element
      class const_iterator;            // Similar to std::vector's iterator to a constant element


      iterator       begin()        noexcept;    // Similar to std::vector's begin()
      const_iterator begin()  const noexcept;
      const_iterator cbegin() const noexcept;

      iterator       end()          noexcept;   // Similar to std::vector's end()
      const_iterator end()    const noexcept;
      const_iterator cend()   const noexcept;



      /********************************************
      * Capacity queries
      ********************************************/
      bool empty()    const noexcept;   // Test for an empty container, similar to std::vector's empty()
      Size size()     const noexcept;   // Return the number of elements in the container, similar to std::vector's size()
      Size capacity() const noexcept;   // Returns the size of the storage space currently allocated for the array,
                               // expressed in terms of elements, similar to std::vector's capacity()



      /********************************************
      * Element access
      ********************************************/
      Rational          & retrieve  ( const Index & position );         // L-value post-fix form, similar to std::vector's at()
      const Rational    & retrieve  ( const Index & position ) const;   // R-value post-fix form, similar to std::vector's at()
      RationalArray       retrieve  ( const Index & startingAt, const Size & howMany ) const;  // similar to std::string's substr()

      Rational          & operator[]( const Index & position );         // L-value in-fix form,   similar to std::vector's operator[]()
      const Rational    & operator[]( const Index & position ) const;   // R-value in-fix form,   similar to std::vector's operator[]()


      void print( std::ostream & s = std::cout ) const;



      /********************************************
      * Modifiers
      ********************************************/
      // Insert element(s) at the beginning, or at the end, or before the indicated element and return the index of the (first)
      // newly inserted element. These operations invalidate all indexes and iterators held by the client. Note that RationalArray
      // objects are intentionally passed by value (See Meyers) leveraging both the copy and move constructors. The index returned
      // is similar to the iterator returned by std::vector::insert(). Note that begin()+index yields an iterator as is returned by
      // std::vector::insert().
      //    [[ Meyers, Effective Modern C++, 2014, Item 41 ]]
      //
      // Note also that initializer lists can also be prepended, appended, or inserted, thanks to RationalArray's
      // initializer_list implicit constructor.  For example:
      //   myArray.append( { {2,3}, 7, 4.75 } );

      Index prepend( const Rational       & element  );                        // similar to the std::deque's push_front()
      Index prepend(       RationalArray    elements );
      Index append ( const Rational       & element  );                        // similar to the std::vector's push_back()
      Index append (       RationalArray    elements );
      Index insert ( const Rational       & element,  const Index & before );  // similar to the std::vector's insert()
      Index insert (       RationalArray    elements, const Index & before );  // If the array is shorter than "beforeItem", elements are appended to the end.


      // Assignment. Similar to the compiler synthesized copy constructor, the compiler synthesized copy assignment operator will
      // cause RationalArray to leak memory. Good rule of thumb, implement the copy constructor and copy assignment operator in
      // pairs. But unlike the copy constructor which must pass its argument by reference to avoid infinite recursion, the copy
      // assignment operator passes its argument by non-constant value. This argument binds to both l-values and r-values using copy
      // and move constructors, respectively. Initialization lists can also be assigned because they are implicitly converted to
      // RationalArrays by the initialization_list constructor. In testing, r-value objects were elided into the argument instead of
      // moved, which is highly desirable [http://en.wikipedia.org/wiki/Copy_elision].
      //    [[ Meyers, Effective Modern C++, 2014, Item 41 ]]
      RationalArray & operator=( RationalArray otherArray );     // copy assignment


      // Remove all elements making the container be empty
      void clear();  // similar to std::vector's clear()


      // Replace the element(s) at the specified index with the provided element(s), extending the size if necessary. Similar to
      // std: :basic_string::replace(), only std::basic_string::replace() throws an exception instead of extending the size. Returns *this.
      RationalArray & replace( const Rational      & element,  const Index & position );
      RationalArray & replace( const RationalArray & elements, const Index & startingAt );


      // Removes from RationalArray a range of elements ([position, position + howMany)). Returns the index of the new location of the
      // element that followed the last element removed. This is the container's new size if the operation removed the last element
      // in the sequence. Invalidates all indexes held by the client. Similar to the std::vector's erase().
      Index remove( const Index & position, const Size & howMany = 1 );



    private:
      // Class attributes
      static const Size  BLOCK_SIZE = 10;  // Blocks of memory are dynamically acquired and released in multiples of BLOCS_SIZE

      // Instance attributes
      Size          _size      = 0;
      Size          _capacity  = 0;
      Element*      _container = nullptr;  // Dynamically allocated array of Elements

      // Helper member functions
      void adjustCapacity( Size newCapacity );
  };





  /*************************************************************************
  ** RationalArray's iterator classes declarations
  **************************************************************************/
  // When implementing user defined iterators, overloading operators is a necessity, and understanding the subtle, recursive
  // properties of operator->() is not always apparent at first glance. User defined iterators are a good example of how one can
  // write their own data types that are as natural to use as the built in types. Clients use these iterators as they would a native
  // pointer - both are full Random Access Iterators. Providing iterators for our container is just another way to make our classes
  // easy to use, and hard to use wrong. For example, providing both iterator and const_iterator such that non-constant
  // RationalArrays can be modified through the iterator, but constant RationalArray objects cannot - and the compiler will help
  // enforce it so there are no run-time surprises later. In contrast to native pointers, however, these iterators will perform
  // error checking further protecting users from using the iterators wrong.
  class RationalArray::iterator /*: public std::iterator<std::random_access_iterator_tag, Rational>*/ // std::iterator deprecated in c++17
  {
    friend bool operator==( const iterator & lhs, const iterator & rhs );
    friend bool operator< ( const iterator & lhs, const iterator & rhs );

    friend RationalArray::iterator  RationalArray::begin() noexcept;


    public:
      // Exception Type Hierarchy Definition
      struct NullPointerException         : RationalArrayExceptions { using RationalArrayExceptions::RationalArrayExceptions; };
      struct InvalidatedIteratorException : RationalArrayExceptions { using RationalArrayExceptions::RationalArrayExceptions; };
      struct DifferingArraysException     : RationalArrayExceptions { using RationalArrayExceptions::RationalArrayExceptions; };

      // Types (explicitly define these types in the wake of std::iterator deprecation
      using iterator_category = std::random_access_iterator_tag;
      using value_type        = Rational;
      using difference_type   = std::ptrdiff_t;
      using pointer           = value_type * ;
      using reference         = value_type & ;

      /********************************************
      ** Construction and destruction
      *********************************************/
      // Synthesized copy constructor and copy assignment operator are fine.
      iterator            ()                       = default;
      iterator            ( const iterator  & )    = default;
      iterator            (       iterator && )    = default;
      iterator & operator=( const iterator  & )    = default;
      iterator & operator=(       iterator && )    = default;
      virtual ~iterator   ()                       = default;

    protected:
      // To make the iterator class hard to use wrong, don't allow users of this class to create initialized iterators. Force users
      // to use RationalArray's begin() and end() functions.
      iterator( const RationalArray & theArray );

    public:
      /********************************************
      * Modifiers
      ********************************************/
      // Increment and decrement
      iterator & operator++();      // pre -increment
      iterator & operator--();      // pre -decrement

      iterator   operator++( int ); // post-increment
      iterator   operator--( int ); // post-decrement

      // Compound assignment
      iterator & operator+=( RationalArray::Size offset );
      iterator & operator-=( RationalArray::Size offset );

      difference_type operator-( const iterator & rhs );


      /********************************************
      * Element access
      ********************************************/
      // Iterator dereference with error checking
      Rational & operator* () const;
      Rational * operator->() const;

      // Indexing the array through the iterator.  The iterator can be constant or non-constant, but the iterator, by definition,
      // points to a non-constant RationalArray.
      Rational & operator[]( const Size & at ) const;

    protected:
            Element*         _elementPointer = nullptr;       // current value of iterator
      const RationalArray*   _theArray       = nullptr;       // the RationalArray container being iterated over
            Element*         _baseContainer  = nullptr;       // the underlying array holding RationalArray's elements
  };

  // Relational and Equality
  bool operator<=( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs );
  bool operator!=( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs );
  bool operator> ( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs );
  bool operator>=( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs );

  // Symmetric Pointer Arithmetic
  RationalArray::iterator operator+ ( RationalArray::iterator it, RationalArray::Size offset );
  RationalArray::iterator operator- ( RationalArray::iterator it, RationalArray::Size offset );

  RationalArray::iterator operator+ ( RationalArray::Size offset, RationalArray::iterator it );
  RationalArray::iterator operator- ( RationalArray::Size offset, RationalArray::iterator it );



  class RationalArray::const_iterator : public RationalArray::iterator
  {
    friend RationalArray::const_iterator RationalArray::begin() const noexcept;

    public:
      using RationalArray::iterator::iterator;  // inherit base class constructors.

      // implicit conversion from iterator to const_iterator
      const_iterator( const RationalArray::iterator & it );

      /********************************************
      * Element access
      ********************************************/
      // Note that these functions are not virtual functions, and therefore hide, not override, the base class functions and will
      // not be called polymorphically. A function that overrides a base class virtual function requires the return type to
      // be the same as, or a covariant of the return type of the function being overridden. The return types of these two functions
      // intentionally do not met that criteria.
      //    [[ Lippman Lajoie Moo, C++ Primer 5e, 2013, Section 15.3, Virtual Functions in a Derived Class ]]
      //    [[ Stroustrup, The C++ Programming Language 4e, 2013, 20.3.6 ]]
      const Rational & operator* () const;
      const Rational * operator->() const;

      // Indexing the array through the iterator.  The iterator can be constant or non-constant, but the const_iterator, by definition,
      // points to a constant RationalArray.
      const Rational & operator[]( const Size & at ) const;
  };

  // Symmetric Pointer Arithmetic
  RationalArray::const_iterator operator+ ( RationalArray::const_iterator it, RationalArray::Size offset );
  RationalArray::const_iterator operator- ( RationalArray::const_iterator it, RationalArray::Size offset );

  RationalArray::const_iterator operator+ ( RationalArray::Size offset, RationalArray::const_iterator it );
  RationalArray::const_iterator operator- ( RationalArray::Size offset, RationalArray::const_iterator it );

} // namespace Library



// Inline functions and template definitions must be part of the header file. To help keep the size of the header file manageable
// and to keep implementation definitions separated from interface declaration, the header file has been divided into two.
// Library/RationalArray.hxx is not intended to be included by clients and serves only to allow RationalArray and related
// declarations to be separated from implementation whist making inline function and template definitions visible to all translation
// units. This technique is primary used as a teaching tool. I don't see this in real-world applications often, but it's out there.
// It allows me to distribute the declarations without the definitions. Normally, the contents of "Library/RationalArray.hxx" would
// be physically entered below instead of telling the pre-processor to include the contents.
#include "Library/RationalArray.hxx"       // RationalArray inline function and template definitions

#endif  // #ifndef RATIONAL_ARRAY_HPP
