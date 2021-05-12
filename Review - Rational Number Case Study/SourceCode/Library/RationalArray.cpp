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


#include <algorithm>             // std::min(), std::max(), std::swap()
#include <cstddef>               // std::size_t
#include <iomanip>               // std::setw(), std::setfill()
#include <iostream>
#include <new>                   // std::bad_alloc
#include <sstream>
#include <string>

#include "Library/Rational.hpp"
#include "Library/RationalArray.hpp"

namespace Library
{
  /********************************************
  * Constructors, destructor
  ********************************************/
  // Fill with N elements construction
  RationalArray::RationalArray(const Rational & rational, const Size & howMany)
  {
    try
    {
      adjustCapacity(howMany);

      // increment size as we go along so cleanup can occur should construction fail
      for( _size = 0; _size != howMany; ++_size )  _container[_size] = new Rational( rational );
    }


    // Convert to construction failure, return any acquired resources up to the point of failure, and add location to the stack trace
    catch( RationalArrayExceptions & ex )
    {
      clear();
      throw ConstructionFailure( ex, "Error: RationalArray construction failure.", __LINE__, __func__, __FILE__);
    }

    catch( std::bad_alloc & ex )
    {
      clear();
      throw ConstructionFailure( ex, "Error: Memory allocation failure during RationalArray Fill construction", __LINE__, __func__, __FILE__);
    }
  }






  RationalArray::Index RationalArray::insert ( RationalArray elements, const Index & before )
  {
    // The "elements" parameter is local to this function. It's elements will be moved (shallow copied) from "elements" into "this"
    // RationalArray object. "elements" was copy (or move) constructed (or construction was elided away
    // [http://en.wikipedia.org/wiki/Copy_elision]) and as such already has a copy of the elements to be inserted. Inserting them is
    // a simple matter moving the pointers into this array. The actual deep copying was delegated to the copy constructor.
    try
    {
      adjustCapacity( _size + elements._size );
      // Cap the insertion point to the current size of the container. Any value of "before" which is greater than the size of
      // the array appends elements to the end.
      const auto insertionPoint = std::min( before, _size );

      // Traversing from right to left, make room for the additional elements by shifting elements (using a shallow copy) at or after
      // the insertion point opening up a gap big enough to hold all the additional elements. The size of the "gap" is the size of
      // the other array (e.g. "elements")
      for( Index i = _size; i-- != insertionPoint;  )
      {
        _container[i+elements._size] = _container[i];
        _container[i] = nullptr;   // not needed here, but a good defensive programming practice
      }
      _size += elements._size;

      // Move and take ownership (i.e. responsibility for deletion) of the elements in the other container placing them into this container
      while( elements._size != 0 )
      {
        --elements._size;
        _container[insertionPoint + elements._size] = elements._container[elements._size];
        elements._container[elements._size] = nullptr;  // not needed here, but a good defensive programming practice
      }

      return insertionPoint;
    }

    // Convert to insertion failure and add location to the stack trace
    catch( CapacityAdjustmentFailure & ex ) { throw InsertionFailure( ex, "Error:  insertion error",   __LINE__, __func__, __FILE__); }
    catch( std::bad_alloc & ex )            { throw InsertionFailure( ex, "Memory Allocation Failure", __LINE__, __func__, __FILE__); }
  }






  Rational & RationalArray::retrieve( const Index & position )
  {
    if( position >= _size )
    {
      std::ostringstream message;
      message << "Error:  Attempted to retrieve an element outside the current range\n"
              << "        Number of elements currently in container:  " << _size << '\n'
              << "        Index of the element requested:          :  " << position;
      throw OverIndex( message.str(), __LINE__, __func__, __FILE__ );
    }

    return * _container[position];
  }






  void RationalArray::clear()
  {
    // decrement _size as we move along so it always represents how many dynamically allocated Elements remain help by the
    // container. Keeping an accurate count facilitates exception safety and error clean up.
    while ( _size != 0 ) delete _container[ --_size ];  // delete the dynamically allocated Rational objects held by the container
    adjustCapacity( 0 );
  }






  RationalArray & RationalArray::replace( const Rational & element, const Index & position )
  {
    if( position >= _size )
    {
      std::ostringstream message;
      message << "Error:  Attempted to replace an element outside the current range\n"
              << "    Number of elements currently in container:  " << _size << '\n'
              << "    Index of the element requested:          :  " << position;
      throw OverIndex( message.str(), __LINE__, __func__, __FILE__ );
    }

    // Replace the contents of the referenced element within _container with a copy of the element passed in as a parameter.
    *_container[position] = element;
    return *this;
  }






  RationalArray & RationalArray::replace( const RationalArray & element, const Index & startingAt )
  {
    if( startingAt > _size )
    {
      std::ostringstream message;
      message << "Error:  Attempted to replace an element outside the current range\n"
              << "    Number of elements currently in container:  " << _size << '\n'
              << "    Index of the element requested:          :  " << startingAt;
      throw OverIndex( message.str(), __LINE__, __func__, __FILE__ );
    }

    // Expand the size of this array to accommodate additional elements, but of course never allow the array to get any smaller.
    adjustCapacity( std::max( startingAt + element._size, _size ) );

    {  // limit the scope of i and j as a defensive programming best practice
      Index i = startingAt, j = 0;

      // replace overlapping elements by replacing the contents of the referenced elements within
      // _container with a copy of the elements from the other's container.
      for(;  i != _size  &&  j != element._size;  ++i, ++j)   *_container[i] = *element._container[j];

      // append any remaining elements by deep copying, incrementing _size as we go along to support
      // cleanup should an error occur.
      for(;  j != element._size;  ++i, ++j, ++_size)    _container[i] = new Rational( *element._container[j] );
    }

    return *this;
  }






  RationalArray::Index RationalArray::remove( const Index & position, const Size & howMany )
  {
    if( position + howMany  > _size )
    {
      std::ostringstream message;
      message << "Error:  Attempted to remove an element outside the range of a RationalArray object.\n"
              << "   Valid Range:                      0 .. " << _size - 1 << '\n'
              << "   Number of elements in container:  " << _size << '\n'
              << "   Range of elements to be removed:  " << position + howMany - 1 << '\n'
              << "   Number of elements to be removed: " << howMany << '\n';
      throw OverIndex( message.str(), __LINE__, __func__, __FILE__ );
    }

    // Release the resources held by the elements being removed, essentially creating a gap in _container
    for( auto i = position;  i != position + howMany;  ++i)
    {
      delete _container[i];
      _container[i] = nullptr;
    }

    // Close the gap by shifting elements from right to left.
    for(Index i = position;  i != _size - howMany;  ++i)   _container[i] = _container[i + howMany];

    _size -= howMany;

    adjustCapacity( _size );
    return position;
  }






  void RationalArray::print( std::ostream & s) const
  {
    if( empty() )  s << "[] {empty}";
    else
    {
      int indexWidth = 0;      // number of base 10 digits needed to display _size (log10(_size)+1)
      for( auto i = _size; i != 0; ++indexWidth, i/=10 );

      for( Index i = 0; i != _size; ++i )
      {
        s << '[' << std::setw( indexWidth ) << i << "] "
          << std::setw(15)      // makes a weak attempt to align elements
          << *_container[i] << '\n';
      }
    }
  }






  // _size, _capacity, and _container must be initialized before calling adjustCapacity(). _capacity, and _container may be adjusted
  // after the call to adjustCapacity().
  void RationalArray::adjustCapacity( Size newCapacity )
  {
    Element * newContainer = nullptr;
    try
    {
      if( newCapacity < _size )  newCapacity = _size;                           // adjusting smaller than current number of elements held in container not allowed
      newCapacity = ((newCapacity + BLOCK_SIZE-1) / BLOCK_SIZE ) * BLOCK_SIZE;  // round up to the nearest number of whole blocks, then multiply by elements in a block

      if( newCapacity == _capacity )  return;  // no adjustment  necessary


      if( newCapacity == 0 ) // Implies _size is also zero, which in turn implies it's safe to delete the container
      {
        delete[] _container;
        _container = nullptr;
      }
      else
      {
        // value initialize all elements to nullptr
        //    [[ Lippman Lajoie Moo, C++ Primer 5e, 2013, Section 12.2, Initializing an Array of Dynamically Allocated Objects ]]
        newContainer = new Element[ newCapacity ]();

        // Perform a shallow copy resulting in both containers having pointers to the Elements
        for (Index i = 0;  i != _size;  ++i)      newContainer[i] = _container[i];

        delete [] _container;  // deletes the dynamically allocated container, but not the elements pointer to.
        _container = newContainer;
      }

      _capacity = newCapacity;  // If you make it this far, no exceptions have been thrown, so update the container's capacity.
    }

    // convert bad_alloc to capacity adjustment failure
    catch( std::bad_alloc & ex )
    {
      delete[] newContainer;  // clean up our mess before we leave

      std::ostringstream message;
      message << "Error:  RationalArray capacity adjustment failed while seeking a new capacity of " << newCapacity << " possible entries.\n"
              << "        Insufficient Heap Memory is a likely cause.";
      throw CapacityAdjustmentFailure( ex, message.str(), __LINE__, __func__, __FILE__ );
    }
  }






  void swap( RationalArray & lhs, RationalArray & rhs ) noexcept
  {
    // Note that all std::swap() overload candidates are added to the overload resolution scope. This allows user defined versions
    // of swap() to be called instead of std::swap() for object types that define their own versions of swap.
    //    [[ Lippman Lajoie Moo, C++ Primer 5e, 2013, Section 13.3, Swap ]]
    //    [[ Stack Overflow, What is the copy-and-swap idiom?, http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom ]]
    using std::swap;
    swap( lhs._size,      rhs._size      );
    swap( lhs._capacity,  rhs._capacity  );
    swap( lhs._container, rhs._container );
  }






  // Member access with error checking.  Operator arrow is recursive in nature. If operator arrow is applied to a native pointer type,
  // the pointer is dereferenced and the indicated member is fetched from the resulting object. Essentially, p->m is equivalent to
  // (*p).m for native pointers. If operator arrow is applied to an object of a class that defied operator->(), then the result of
  // p.operator->() is used to fetch the indicated member. This is repeated until a native pointer is returned followed by applying
  // operator arrow to that native pointer.
  //    [[ Lippman Lajoie Moo, C++ Primer 5e, 2013, Section 14.7, Constraints on the Return from Operator Arrow ]]
  Rational * RationalArray::iterator::operator->() const
  {
    // Iterators have been invalidated if the underlying container is moved. Underlying containers are moved, for example when they
    // are expanded to hold more elements. Does the RationalArray still use the same underlying container as when this iterator was
    // constructed? That is, is this iterator still valid?
    if( _theArray->_container != _baseContainer )
    {
      throw InvalidatedIteratorException( "Error:  Attempt to dereference an iterator that has been invalidated", __LINE__, __func__, __FILE__ );
    }

    // is this iterator null?
    if( _elementPointer == nullptr )
    {
      throw NullPointerException( "Error:  Attempt to dereference null pointer", __LINE__, __func__, __FILE__ );
    }

    // the value of this iterator points to an element within the underlying container's range?
    if( _elementPointer < _theArray->_container  || _elementPointer >= _theArray->_container + _theArray->_size )
    {
      throw OverIndex( "Error:  Attempt to dereference an iterator pointing to an element outside RationalArray's boundary", __LINE__, __func__, __FILE__ );
    }

    // the element itself is a pointer-to-Rational. Is this pointer null?
    if( *_elementPointer == nullptr )
    {
      throw NullPointerException( "Error:  Attempt to dereference null pointer", __LINE__, __func__, __FILE__ );
    }

    return *_elementPointer;
  }






  RationalArray::iterator::difference_type RationalArray::iterator::operator-(const iterator & rhs)
  {
    if( _theArray != rhs._theArray )
    {
      throw DifferingArraysException( "Error:  Iterator difference meaningful only when both iterators reference the same container",
                                      __LINE__, __func__, __FILE__ );
    }
    return _elementPointer - rhs._elementPointer;  // random access iterator difference
  }






  bool operator==( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs )
  {
    if( lhs._theArray != rhs._theArray )
    {
      throw RationalArray::iterator::DifferingArraysException
        ( "Error:  Iterator equality comparison meaningful only when both iterators reference the same container",
          __LINE__, __func__, __FILE__ );
    }
    return lhs._elementPointer == rhs._elementPointer;
  }






  bool operator< ( const RationalArray::iterator & lhs, const RationalArray::iterator & rhs )
  {
    if( lhs._theArray != rhs._theArray )
    {
      throw RationalArray::iterator::DifferingArraysException
        ( "Error:  Iterator ordering comparison meaningful only when both iterators reference the same container",
          __LINE__, __func__, __FILE__ );
    }
    return lhs._elementPointer <  rhs._elementPointer;
  }

} // namespace Library
