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


/*************************************************************************
** RationalArray inline function and template definitions
**
** Function definitions in this section represent good inline candidates. These definitions have been moved from the source file
** into this header file making the definitions visible to the compiler at the point of call. Each function has been defined as
** "inline" which causes it to have internal linkage, which in turn makes the linker happy when this header file is included in more
** than one translation unit (e.g. no duplicate symbol error messages)
**
** Many of these one line functions could have been coded in the class definition but I choose to do it here to facilitate
** separating implementations from interface, thus making it easier to modify the implementation later. Template definitions usually
** are coded in the header file as well.
**************************************************************************/
#ifndef RATIONAL_ARRAY_HXX
#define RATIONAL_ARRAY_HXX

#include <initializer_list>
#include <iterator>                 // std::begin, std::end, std::distance
#include <new>                      // std::bad_alloc
#include <utility>                  // std::move

#include "Library/Rational.hpp"
#include "Library/RationalArray.hpp"



namespace Library
{
  /********************************************
  * Friends
  ********************************************/
  inline std::ostream & operator<<  (std::ostream & stream, const RationalArray & r)
  {
    r.print( stream );
    return stream;
  }



  /********************************************
  * Constructors, destructor
  ********************************************/
  inline RationalArray::RationalArray() noexcept = default;
  inline RationalArray::RationalArray( const RationalArray                   & otherArray ) : RationalArray( otherArray.begin(), otherArray.end() ) {}
  inline RationalArray::RationalArray( const std::initializer_list<Rational> & theList    ) : RationalArray( theList.begin(),    theList.end()    ) {}

  inline RationalArray::RationalArray( RationalArray && otherArray )  noexcept
  { swap( *this, otherArray ); }

  inline RationalArray::RationalArray( const RationalArray & otherArray, const Index & startingAt, const Size & howMany )
    : RationalArray( otherArray.begin() += startingAt,   otherArray.begin() += startingAt + howMany )
  {}

  template <typename Container>
  inline RationalArray::RationalArray( const Container  & otherContainer ) : RationalArray( std::begin(otherContainer), std::end(otherContainer) ) {}

  template <typename InputIterator, typename>
  RationalArray::RationalArray( InputIterator first, InputIterator last )
  {
    try
    {
      adjustCapacity( Size(std::distance(first, last)) );  // calculate size from (not necessarily random) iterators

      // Perform a deep copy.
      // increment as we go along so cleanup can occur should construction fail
      for( auto i = first; i != last; ++i )  _container[_size++] = new Rational( *i );
    }


    // Convert to construction failure, return any acquired resources up to the point of failure, and add location to the stack trace
    catch( RationalArrayExceptions & ex )
    {
      clear();
      throw ConstructionFailure( ex, "Error: RationalArray Iterators (first .. last] copy construction failed.", __LINE__, __func__, __FILE__ );
    }

    catch( std::bad_alloc & ex )
    {
      clear();
      throw ConstructionFailure( ex, "Error: Memory allocation failure during RationalArray Iterators (first .. last] copy construction", __LINE__, __func__, __FILE__ );
    }

    catch( ... )  // I don't know what this is, but I definitely want to clean up my partially constructed array
    {
      clear();
      throw;  // re-throw active exception object
    }
  }

  inline RationalArray::~RationalArray()
  { clear(); }



  /********************************************
  * RationalArray iterators
  ********************************************/
  inline RationalArray::iterator       RationalArray::begin()        noexcept { return *this;   }
  inline RationalArray::const_iterator RationalArray::begin()  const noexcept { return *this;   }
  inline RationalArray::const_iterator RationalArray::cbegin() const noexcept { return begin(); }

  inline RationalArray::iterator       RationalArray::end()          noexcept { return begin() += _size; }
  inline RationalArray::const_iterator RationalArray::end()    const noexcept { return begin() += _size; }
  inline RationalArray::const_iterator RationalArray::cend()   const noexcept { return begin() += _size; }

  /********************************************
  * Capacity queries
  ********************************************/
  inline bool                RationalArray::empty   () const noexcept  { return _size == 0; }
  inline RationalArray::Size RationalArray::size    () const noexcept  { return _size; }
  inline RationalArray::Size RationalArray::capacity() const noexcept  { return _capacity; }


  /********************************************
  * Element access
  ********************************************/
  inline const Rational & RationalArray::retrieve  ( const Index & position ) const
  { return const_cast<Library::RationalArray *>(this)->retrieve( position ); }

  inline RationalArray RationalArray::retrieve ( const Index & startingAt, const Size & howMany ) const
  { return {*this, startingAt, howMany}; }

  inline Rational & RationalArray::operator[]( const Index & position )
  { return retrieve( position ); }

  inline const Rational & RationalArray::operator[]( const Index & position ) const
  { return retrieve( position ); }


 /********************************************
  * Modifiers
  ********************************************/
  inline RationalArray::Index RationalArray::prepend( const Rational       & element  )   { return insert(element,  0); }
  inline RationalArray::Index RationalArray::prepend(       RationalArray    elements )   { return insert(std::move(elements), 0); }

  inline RationalArray::Index RationalArray::append ( const Rational       & element  )   { return insert(element,  _size); }
  inline RationalArray::Index RationalArray::append (       RationalArray    elements )   { return insert(std::move(elements), _size); }

  inline RationalArray::Index RationalArray::insert( const Rational & element, const Index & before )
  { return insert( RationalArray(element), before ); }


  inline RationalArray & RationalArray::operator=( RationalArray otherArray )    // Copy assignment
  {
    // implement the copy assignment operator using the copy/swap pattern. This pattern provides for added exception safety and
    // protects against self assignment.
    //    [[ Lippman Lajoie Moo, C++ Primer 5e, 2013, Section 13.3, Using swap in Assignment Operators ]]

    swap( *this, otherArray );
    return *this;
  }







  /*************************************************************************
  ** iterator and const_iterator class inline definitions
  *************************************************************************/

  /********************************************
  ** Construction and destruction
  *********************************************/
  inline RationalArray::iterator::iterator( const RationalArray & theArray )
    : _elementPointer( theArray._container ), _theArray( &theArray ), _baseContainer( theArray._container )
  {}


  /********************************************
  * Modifiers
  ********************************************/
  inline RationalArray::iterator & RationalArray::iterator::operator++()    // pre -increment
  {
    if(_elementPointer != nullptr) ++_elementPointer;
    return *this;
  }

  inline RationalArray::iterator & RationalArray::iterator::operator--()    // pre -decrement
  {
    if(_elementPointer != nullptr) --_elementPointer;
    return *this;
  }

  inline RationalArray::iterator   RationalArray::iterator::operator++(int) // post-increment
  {
    iterator tmp(*this);
    operator++();
    return tmp;
  }

  inline RationalArray::iterator   RationalArray::iterator::operator--(int) // post-decrement
  {
    iterator tmp(*this);
    operator--();
    return tmp;
  }

  // Compound assignment
  inline RationalArray::iterator & RationalArray::iterator::operator+=( RationalArray::Size offset )
  {
    if( _elementPointer != nullptr ) _elementPointer += offset;
    return *this;
  }

  inline RationalArray::iterator & RationalArray::iterator::operator-=( RationalArray::Size offset )
  {
    if( _elementPointer != nullptr ) _elementPointer -= offset;
    return *this;
  }


  /********************************************
  * Element access
  ********************************************/
  inline Rational & RationalArray::iterator::operator*() const
  { return *(operator->()); }

  // Define direct access via the indexing operator in terms of the dereferencing operator to reuse error checking. Create a new
  // iterator at the desired location, then deference that. Note it[x] is equivalent to *(it + x)
  inline Rational & RationalArray::iterator::operator[]( const Size & at ) const
  { return *(*this + at); }


  /********************************************
  ** Relational and Equality
  ********************************************/
  inline bool operator<=( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs ) { return !(rhs <  lhs); } // note the position of rhs and lhs here
  inline bool operator!=( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs ) { return !(lhs == rhs); }
  inline bool operator> ( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs ) { return  (rhs <  lhs); } // note the position of rhs and lhs here
  inline bool operator>=( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs ) { return !(lhs <  rhs); }


  /********************************************
  ** Symmetric Pointer Arithmetic
  ********************************************/
  inline RationalArray::iterator operator+ ( RationalArray::iterator it, RationalArray::Size offset ) { return it += offset; }
  inline RationalArray::iterator operator- ( RationalArray::iterator it, RationalArray::Size offset ) { return it -= offset; }

  inline RationalArray::iterator operator+ ( RationalArray::Size offset, RationalArray::iterator it ) { return it += offset; }
  inline RationalArray::iterator operator- ( RationalArray::Size offset, RationalArray::iterator it ) { return it -= offset; }





  /*************************************************************************
  ** const_iterator class definitions
  *************************************************************************/

  /********************************************
  ** Construction and destruction
  *********************************************/
  inline RationalArray::const_iterator::const_iterator( const RationalArray::iterator & it ) : iterator(it) {}


  /********************************************
  * Element access
  ********************************************/
  inline const Rational & RationalArray::const_iterator::operator* () const
  { return iterator::operator*(); }

  inline const Rational * RationalArray::const_iterator::operator->() const
  { return iterator::operator->(); }

  // Define direct access via the indexing operator in terms of the dereferencing operator to reuse error checking. Create a new
  // iterator at the desired location, then deference that. Note it[x] is equivalent to *(it + x)
  inline const Rational & RationalArray::const_iterator::operator[]( const Size & at ) const
  { return *(*this + at); }


  /********************************************
  ** Symmetric Pointer Arithmetic
  ********************************************/
  inline RationalArray::const_iterator operator+ ( RationalArray::const_iterator it, RationalArray::Size offset ) { return it += offset; }
  inline RationalArray::const_iterator operator- ( RationalArray::const_iterator it, RationalArray::Size offset ) { return it -= offset; }

  inline RationalArray::const_iterator operator+ ( RationalArray::Size offset, RationalArray::const_iterator it ) { return it += offset; }
  inline RationalArray::const_iterator operator- ( RationalArray::Size offset, RationalArray::const_iterator it ) { return it -= offset; }

} // namespace Library
#endif
