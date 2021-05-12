/******************************************************************************
** (C) Copyright 2015 by Thomas Bettens. All Rights Reserved.
**
** DISCLAIMER: The authors have used their best efforts in preparing this code. These efforts include the development, research, and
** testing of the theories and programs to determine their effectiveness. The authors make no warranty of any kind, expressed or
** implied, with regard to these programs or to the documentation contained within. The authors shall not be liable in any event for
** incidental or consequential damages in connection with, or arising out of, the furnishing, performance, or use of these libraries
** and programs.  Distribution without written consent from the authors is prohibited.
******************************************************************************/


/******************************************************************************
** This Fundamentals of C++ Programming project are externally documented, but in general two classes are created, and a driver
** function exercises them.  The first class, named Rational, provides arithmetic functionality for rational scalars.  The second
** class, named RationalArray, provides container operations for a collection of Rational objects. Function main() stimulates the
** operations provided by Rational and RationalArray.  Errors are detected and appropriate exceptions are raised.
**
** Tom Bettens
**
** Updated - 11-AUG-2019 Tested against ISO/IEC 14882:2017(E) - Programming languages -- C++ with MSVC++ 2019 Enterprise (version
**           16.2.1), g++ (version 9.1.1), clang++ (version 7.0.0)
**
** Updated - 26-JAN-2019 Tested against ISO/IEC 14882:2017(E) - Programming languages -- C++ with MSVC++ 2017 Enterprise (version
**           15.9.6), g++ (version 8.2.1), clang++ (version 7.0.0)
**
** Updated - 18-JUN-2013 Tested against ISO/IEC 14882:2011(E) - Programming languages -- C++ with MSVC++ 2012 Ultimate + SP1, g++
**           (version 4.8.0) over cygwin (Cygwin DLL version TBD)
**
** Updated - 30-JAN-2012 Tested against ISO/IEC 14882:2003(E) - Programming languages -- C++ with MSVC++ 2008 Express + SP1, MSVC++
**           2010 Ultimate + SP1, g++ (version 4.5.3) over cygwin (Cygwin DLL version 1.7.9-1)
**
** Updated - 07-JUN-2009 Tested with: MSVC++ 2008 Express + SP1, g++ (version TBD) over cygwin (version TBD)
**
******************************************************************************/
#include <cmath>            // abs()
#include <exception>
#include <iomanip>          // setw(), setprecision()
#include <iostream>
#include <random>
#include <string>
#include <tuple>            // tuple, get()
#include <vector>

#include "Library/Rational.hpp"
#include "Library/RationalArray.hpp"



namespace // Anonymous namespace - everything in here has internal linkage
{
  template <typename T>
  using Range = std::tuple<T, T>;  // Lower and Upper (inclusive) boundaries



  // This may look like a class, and it is, but it is also a function.  More precisely, it's called a Functoid and allows instances
  // of Function Objects to be created.  I wanted to reuse the seeded random number engine for all random numbers while at the same
  // time allowing each random number usage to realize its own distribution pattern. I also wanted to refactor the common random
  // number generation used in main() and testArray() into a single location. But I also wanted to keep the function localized
  // (internal linkage) to the main() and testArray() translation unit.
  class RandomRationalFactory
  {
    public:
      RandomRationalFactory(const Range<int> & numerator, const Range<unsigned> & denominator)
      : randomNumerator  ( std::get<LOWER_BOUND>(numerator),   std::get<UPPER_BOUND>(numerator)   ),
        randomDenominator( std::get<LOWER_BOUND>(denominator), std::get<UPPER_BOUND>(denominator) )
      {}

      Library::Rational  operator() ()
      {
        // Roll the random numbers to create a random numerator and denominator
        return Library::Rational( randomNumerator(engine), randomDenominator(engine) );
      }

    private:
      // Constants
      enum : unsigned { LOWER_BOUND = 0U, UPPER_BOUND}; // indexes into Range

      // Class Attributes
      inline static std::default_random_engine engine{ std::random_device{}() }; // inline static since C++17

      // Instance Attributes
      std::uniform_int_distribution<int>      randomNumerator;
      std::uniform_int_distribution<unsigned> randomDenominator;
  };
} // Anonymous namespace












int main()
{
  using Library::Rational;
  using Library::RationalArray;

  try
  {
    RandomRationalFactory buildRandomRational( { -2000, 2000 }, { 1U, 10000U } );


    // Fill an array with random fractions
    RationalArray  myArray({0, 1}, 6);
    for(auto & fraction : myArray)  fraction = buildRandomRational();

    // Create a vector from the array and fill in with the myArray values, but in reverse order
    std::vector<Rational> myVector;
    for(auto i = myArray.size();  i > 0;  --i)  myVector.push_back(myArray.retrieve(i-1) );



    // Add up all the Fractions in the array and vector. Let's use both Rational and floating point arithmetic and compare the results
    Rational     f_arraySum  = {0, 1},
                 f_vectorSum = {0, 1};
    long double  d_arraySum  = 0.0L,
                 d_vectorSum = 0.0L;

    for(const auto & fraction : myArray)
    {
      f_arraySum += fraction;                              // Rational arithmetic
      d_arraySum += static_cast<long double>( fraction );  // floating point arithmetic
    }

    for(const auto & fraction : myVector)
    {
      f_vectorSum += fraction;                             // Rational arithmetic
      d_vectorSum += static_cast<long double>( fraction ); // floating point arithmetic
    }


    // display the contents of the containers
    std::cout  << "        MyArray         myVector\n"
               << "----------------------------------------\n";
    for(unsigned i = 0; i != myArray.size(); ++i)
    {
      std::cout << std::setw(15) << myArray[i] << " ["
                << std::setw(2)  << i << "] [" << std::setw( 2 ) << myArray.size()-i-1
                << "] " << myVector[myVector.size()-i-1] << '\n';
    }


    // Are the sums equal?
    std::cout << '\n'
              << "Sum of array  elements using Rational math is:       " << f_arraySum  << '\n'
              << "    of vector elements using Rational math is:       " << f_vectorSum << '\n'

              << std::fixed << std::setprecision( 19 )
              << "Sum of array  elements using floating point math is: " << d_arraySum  << '\n'
              << "    of vector elements using floating point math is: " << d_vectorSum << '\n'

              << std::boolalpha
              << "The array and vector sums using Rationals are equal: " << (f_arraySum == f_vectorSum) << '\n'
              << "The array and vector sums using doubles   are equal: " << (std::abs( d_arraySum - d_vectorSum ) <= 1E-18L) << "\n\n";





    // Additional session 7 requirements
    std::cout << "\n\nmyArray: Original\n" << std::string(21, '-') << '\n'
              << myArray;


    // remove items in the odd locations.   Note:  size() changes each time through the loop
    for(RationalArray::Index i = 1; i < myArray.size(); ++i)  myArray.remove(i);

    std::cout << "\n\nmyArray: items in odd locations removed\n" << std::string(39, '-') << '\n'
              << myArray;

    // fill it up so it grows
    for(auto i = myArray.capacity() * 3;  i != 0;  --i)    myArray.append( buildRandomRational() );
    std::cout << "\n\nmyArray: Grown a couple of times\n" << std::string(32, '-') << '\n'
              << myArray;


    // empty the array.  Note:  size() changes each time through the loop
    while( myArray.size() != 0 ) myArray.remove(0);
    std::cout << "\n\nmyArray: Empty\n" << std::string(21, '-') << '\n'
              << myArray;
  }

  catch( std::exception & ex )
  {
    std::cerr << typeid( ex ).name() << '\n'
              << ex.what() << '\n';
    return -__LINE__;
  }


  return 0;
}
