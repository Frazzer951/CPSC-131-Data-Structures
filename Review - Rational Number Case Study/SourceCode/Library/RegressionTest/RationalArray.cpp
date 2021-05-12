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


/***********************
** If you are having trouble linking this with Visual Studio, see:
** http://stackoverflow.com/questions/3729515/visual-studio-2010-2008-cant-handle-source-files-with-identical-names-in-diff
**
** Right click on the project in the solution explorer, choose C/C++ -> "Output Files" and type the following into the
** "Object File Name" box:
**
**   $(IntDir)/%(RelativeDir)/
**
** Note that I also selected (All Configurations, All Platforms) from the drop downs. This will compile every file in a
** directory hierarchy which mirrors the source tree. VS2010 will begin the build by creating these directories if they
** don't exist. Further, for those who hate white space in their directory names, this macro does remove all spaces, so there
** is no need to play around with double quotes when using it.
**************************/

#include <array>
#include <exception>
#include <forward_list>
#include <iostream>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <vector>

#include "Library/Rational.hpp"
#include "Library/RationalArray.hpp"
#include "Utilities/Exceptions.hpp"
#include "Utilities/Timer.hpp"






namespace Library::RegressionTest::RationalArray
{
  // Failed test exception types
  struct RationalArrayFailedTest : Library::RationalArray::RationalArrayExceptions  {using RationalArrayExceptions::RationalArrayExceptions;};
  struct   SizeError  : RationalArrayFailedTest                                     {using RationalArrayFailedTest::RationalArrayFailedTest;};
  struct   ValueError : RationalArrayFailedTest                                     {using RationalArrayFailedTest::RationalArrayFailedTest;};


  /****************************************************************************************
  ** Define a random Rational generator
  ****************************************************************************************/
  class Random // Calls to instances of this Functoid (a statefull function) generate random Fractions within the provided ranges
  {
    public:
      Random(int NumeratorLowerLimit, int NumeratorUpperLimit, unsigned DenominatorLowerLimit, unsigned DenominatorUpperLimit)
        : numeratorDistribution  (NumeratorLowerLimit,   NumeratorUpperLimit  ),
        denominatorDistribution(DenominatorLowerLimit, DenominatorUpperLimit)
      {}

      Rational  operator() ()
      {
        return Rational( numeratorDistribution(randomNumberGenerationEngine), denominatorDistribution(randomNumberGenerationEngine) );
      }

    private:
      // Class Attributes
      static std::default_random_engine randomNumberGenerationEngine;

      // Instance Attributes
      std::uniform_int_distribution<int>      numeratorDistribution;
      std::uniform_int_distribution<unsigned> denominatorDistribution;
  };
  std::default_random_engine  Random::randomNumberGenerationEngine(std::random_device{}()); // Seed with a value from a random number generator device



  // Helper function to test RationalArray's ability to construct an array of Rational objects when sufficient heap memory is
  // unavailable. Approach: Ask for blocks of ridiculously large amounts of sequential memory until there aren't any left. Then do
  // the same with a smaller block, and so on. The goal is to consume all blocks of heap memory large enough to store a Rational
  // object, which is actually very small. Then we'll try to construct our RationalArray object which, if all goes as expected, will
  // fail. A key tenet to this approach is that all that consumed memory gets released as quickly as possible on all paths of
  // execution, including exceptions. To implement this approach the Resource Acquisition Is Initialization (RAII) pattern together
  // with a smart pointer as a local stack object in a recursive function is used. The smart pointer is destroyed when it goes out
  // of scope, and the smart pointer's destructor releases the associated dynamically allocated memory. When all the memory is
  // recursively consumed and RationalArray's constructor has completed execution (either naturally or via a thrown exception), the stack is unwound
  // destroying the smart pointers.
  //
  // WARNING - this function indeed consumes most, if not all, available heap memory and that in turn may cause the program or even
  // your system to crash. Be careful when running this particular test.
  void gobble( std::size_t blobSize =  0x4000'0000U )
  {
    std::unique_ptr<char[]> p( new(std::nothrow) char[blobSize]() );

    if( p )                                   gobble( blobSize );
    else if( blobSize > sizeof( Rational ) )  gobble( blobSize >> 1 );
    else                                      Library::RationalArray myArray( Rational(), 25 );
  }





  void test01 () // Construction, Copy, Destruction, Append, Capacity
  {
    using Library::RationalArray;

    RationalArray temp;
    if( temp.size() != 0 ) throw SizeError("Unexpected array size after initialization", __LINE__, __func__, __FILE__);

    {
      RationalArray copy( temp );
      if( copy.size() != 0 ) throw SizeError("Unexpected array size after copy", __LINE__, __func__, __FILE__);
    }

    Random randomNumber(-100, 100, 1, 100);

    temp.append( randomNumber() );
    for( RationalArray::Size i = 0;  i != temp.capacity() / 3;  ++i)  temp.append( randomNumber() );
    if( temp.size() != 1 + temp.capacity() / 3 ) throw SizeError("Unexpected array size after append", __LINE__, __func__, __FILE__);

    {
      RationalArray copy( temp );
      if( copy.size() != temp.size() )  throw SizeError("Unexpected array size after copy", __LINE__, __func__, __FILE__);
      for( RationalArray::Size i = 0;  i != temp.size();  ++i)
      {
        if( !(temp.retrieve(i) == copy.retrieve(i)) ) throw ValueError("Values did not match after copy" , __LINE__, __func__, __FILE__);
      }
    }

    for( RationalArray::Size i = temp.size();  i != temp.capacity();  ++i)  temp.append( randomNumber() );
    if( temp.size() != temp.capacity() )  throw SizeError("Unexpected array size after append", __LINE__, __func__, __FILE__);

    {
      RationalArray myList{5, 4};
      RationalArray myContainer1( 5, 4 );
      RationalArray myContainer2( 5.3, 4 );
      RationalArray myContainer3 = {{2, 3}, {3, 5}, {9, -12}};
      RationalArray myContainer4( myContainer1, 1, 2 );
    }

    {
      const std::vector<Rational> myContainer = {3, 5, {2,3}, 5.3};
      RationalArray theOtherContainer( myContainer );
    }
    {
      std::vector<Rational> myContainer = {3, 5, {2,3}, 5.3};
      RationalArray theOtherContainer( myContainer );
    }
    {
      RationalArray myContainer1( std::vector<Rational>{3, 5, {2,3}, 5.3} );
      RationalArray myContainer2( std::forward_list<Rational>{3, 5, {2,3}, 5.3} );

      Rational myRationalCollection[] ={3, 5,{2,3}, 5.3};
      RationalArray myContainer3( myRationalCollection );
    }
  } //test01 ()



  void test02()  // Multiple entries at construction
  {
    using Library::RationalArray;
    auto & PI = Rational::PI();

    RationalArray temp( PI, 4 );

    if( temp.size() != 4 ) throw SizeError("Unexpected array size after initialization", __LINE__, __func__, __FILE__);

    for( RationalArray::Size i = 0;  i != temp.size();  ++i)
    {
      if( !(temp.retrieve(i) == PI) ) throw ValueError("Values did not match after initialization" , __LINE__, __func__, __FILE__);
    }
  } // test02 ()



  void test03() // add to the front and the end
  {
    using Library::RationalArray;
    auto & PI  = Rational::PI(),
         & E   = Rational::E();

    RationalArray temp;
    temp.prepend(PI);
    temp.append(E);
    // for(RationalArray::Size i=0;  i != temp.capacity()/2;  ++i)
    for( RationalArray::Size i=0;  i != 4;  ++i)
    {
      temp.prepend(PI);
      temp.append(E);
    }
    if( temp.size() != (temp.capacity()/2) * 2 )  throw SizeError("Unexpected array size after appending and prepending values", __LINE__, __func__, __FILE__);
    for( RationalArray::Size i = 0;  i != temp.capacity()/2;  ++i)
    {
      if ( !(temp.retrieve(i) == PI) )  throw ValueError("Values did not match after appending and prepending" , __LINE__, __func__, __FILE__);
    }

    for( RationalArray::Size i = temp.capacity()/2;  i != temp.capacity()/2*2;  ++i)
    {
      if( !(temp.retrieve(i) == E) )  throw ValueError("Values did not match after appending and prepending" , __LINE__, __func__, __FILE__);
    }
  } // test03 ()



  void test04()  // Prepending, appending, inserting and slicing Rationals and RationalArrays tests
  {
    using Library::RationalArray;
    auto & PI  = Rational::PI(),
         & E   = Rational::E();

    RationalArray        temp ( PI, 5 );
    const RationalArray  slice(  E, 3 );

    temp.insert( slice, 2 );

    RationalArray control(E, 3);
    control.prepend( RationalArray(PI, 2) );
    control.append( RationalArray( PI, 3 ) );

    if( temp.size() != control.size() )  throw SizeError("Unexpected array size after inserting and slicing values", __LINE__, __func__, __FILE__);
    for( RationalArray::Size i = 0;  i != temp.size();  ++i)
    {
      if( !(temp.retrieve(i) == control.retrieve(i)) )  throw ValueError("Unexpected array value after inserting and slicing values", __LINE__, __func__, __FILE__);
    }

    temp.clear();
    if( temp.size() != 0 )  throw SizeError("Unexpected array size after clearing array", __LINE__, __func__, __FILE__);

    temp = control;
    if( temp.size() != control.size() )  throw SizeError("Unexpected array size after assigning one array to another", __LINE__, __func__, __FILE__);

    for( RationalArray::Size i = 0;  i != temp.size();  ++i)
    {
      if( !(temp.retrieve(i) == control.retrieve(i)) )  throw ValueError("Unexpected array value after assigning one array to another", __LINE__, __func__, __FILE__);
    }

    {
      temp.append( {5, 6, {7,8}} );
      temp.prepend( {{2,3}, 7, 4.75} );
    }

    temp = control.retrieve(0, control.size() / 2);     // R-value assignment
    temp = { PI, E, Rational::PHI() };                    // braced initialization assignment
  } // test04 ()



  void test05()  // Item replacement
  {
    using Library::RationalArray;
    auto & PI  = Rational::PI(),
         & E   = Rational::E();

    RationalArray temp(PI, 5);
    temp.replace(E, 3);

    RationalArray control( PI, 3 );
    control.append(E);
    control.append(PI);

    if( temp.size() != control.size() )  throw SizeError("Unexpected array size after inserting and appending values", __LINE__, __func__, __FILE__);
    for( RationalArray::Size i = 0;  i != temp.size();  ++i)
    {
      if( !(temp.retrieve(i) == control.retrieve(i)) )  throw ValueError("Unexpected array value after constructing control test array", __LINE__, __func__, __FILE__);
    }
  } // test05 ()



  void test06()   // Replace and Remove tests
  {
    using Library::RationalArray;
    auto & PI  = Rational::PI(),
         & E   = Rational::E();

    RationalArray temp(PI, 5);
    temp.replace( RationalArray(E,5), 3);

    RationalArray control( PI, 3 );
    control.append( RationalArray(E,5) );

    if( temp.size() != 8 )               throw SizeError("Unexpected array size after inserting and appending values", __LINE__, __func__, __FILE__);
    if( temp.size() != control.size() )  throw SizeError("Unexpected array size after inserting and appending values", __LINE__, __func__, __FILE__);

    for( RationalArray::Size i = 0;  i != temp.size();  ++i)  if( !(temp.retrieve(i) == control.retrieve(i)) )
    {
      throw ValueError("Unexpected array value after constructing control test array", __LINE__, __func__, __FILE__);
    }

    RationalArray slice = temp.retrieve( 1, 5 );
    temp.remove( 1, 5 );
    if( temp.size() != 3 )               throw SizeError("Unexpected array size after removing values", __LINE__, __func__, __FILE__);
    if( slice.size() != 5)               throw SizeError("Unexpected array size returned from removing values", __LINE__, __func__, __FILE__);
    for( RationalArray::Size i = 0;  i != slice.size();  ++i) if( !(slice.retrieve(i) == control.retrieve(i+1)) )
    {
        throw ValueError("Unexpected array value returned from removing array elements", __LINE__, __func__, __FILE__);
    }
  } // test06 ()



  void test07() // error handling
  {
    using Library::RationalArray;

    // WARNING - function gobble() indeed consumes most, if not all, available heap memory and that in turn may cause the program or even
    // your system to crash. Be careful when running this particular test.
    #if 0
      try // dynamic memory allocation errors
      {
        try
        {
          gobble();      // Consumes heap memory to the point of exhaustion, and the calls RationalArrays's constructor
          throw RationalArrayFailedTest( "Construction error due to insufficient heap memory not detected", __LINE__, __func__, __FILE__ );
        }

        // Because we intentionally consumed all available memory, the ConstructionFailure object may itself be unconstructible. Let's
        // intercept and help the test along. At this point, memory has been restored for our use again.
        catch( std::bad_alloc & ex )
        {
          throw RationalArray::ConstructionFailure( ex, "Bad memory allocation detected", __LINE__, __func__, __FILE__ );
        }
      }
      catch( RationalArray::ConstructionFailure & )   // Ignore the expected exception
      {}
    #endif



    try  // retrieve a fraction that doesn't exist
    {
      RationalArray myArray( Rational(), 10 );
      myArray.retrieve( 15 );
      throw RationalArrayFailedTest( "Retrieval error (over index) not detected", __LINE__, __func__, __FILE__);
    }

    catch( RationalArray::OverIndex & ) {} // Ignore the expected exception


    try // replace something that doesn't exit
    {
      RationalArray myArray( Rational(), 10 );
      myArray.replace( Rational(2, 3),  15 );
      throw RationalArrayFailedTest( "Replacement error (over index) not detected", __LINE__, __func__, __FILE__);
    }

    catch( RationalArray::OverIndex & ) {} // Ignore the expected exception


    try // remove something that doesn't exit
    {
      RationalArray myArray( Rational(), 10 );
      myArray.remove( 15 );
      throw RationalArrayFailedTest( "Replacement error (over index) not detected", __LINE__, __func__, __FILE__);
    }

    catch( RationalArray::OverIndex & ) {} // Ignore the expected exception


    try
    {
      RationalArray myContainer( {1, 2, 3, 4}, 1, 4 ); // over indexes when creating the sub-array
      throw RationalArrayFailedTest( "over index upon sub-array copy construction not detected", __LINE__, __func__, __FILE__);
    }

    catch(RationalArray::ConstructionFailure &) {}  // ignore the expected exception

  } // test07 ()





  void testRationalArray(std::ostream &)
  {
    try
    {
      test01();
      test02();
      test03();
      test04();
      test05();
      test06();
      test07();
    }


    catch( SizeError & ex )
    {
      std:: ostringstream  message;
      message << "Failed:  RationalArray Size Error: \n"  << ex.what() << '\n';
      throw RationalArrayFailedTest( message.str(), __LINE__, __func__, __FILE__ );
    }

    catch( ValueError & ex )
    {
      std:: ostringstream  message;
      message << "Failed:  RationalArray Value Error: \n"  << ex.what() << '\n';
      throw RationalArrayFailedTest( message.str(), __LINE__, __func__, __FILE__ );
    }

     catch(RationalArrayFailedTest & ex )
    {
      std:: ostringstream  message;
      message << "Failed:  RationalArray Undetermined reason: \n"  << ex.what() << '\n';
      throw RationalArrayFailedTest( message.str(), __LINE__, __func__, __FILE__ );
    }

    catch( std::exception & ex )
    {
      std:: ostringstream  message;
      message << "Failed:  Rational Unexpected system error: \n"  << ex.what() << '\n';
      throw RationalArrayFailedTest( message.str(), __LINE__, __func__, __FILE__ );
    }
  }






  /**************************************************************************
  ** Execute the regression tests.  An object of type ExecuteTest is defined at namespace Library::RegressionTest::RationalArray
  ** scope. Such objects are constructed prior to executing main().  The intent here is to cause class RationalArray to be
  ** regression tested without modifying function main(), or any other function, to invoke the regression test.  Simply compiling
  ** and linking this source file into the program will cause the regression test to be performed.  To remove the regression test
  ** from deliverable code, simply remove this source file from the files compiled and linked into the program.
  ***************************************************************************/
  namespace  // unnamed, anonymous namespace provides internal linkage
  {
    //3.b:  Call a function you write that proves to an observer the workings of
    //      class RationalArray  (not looking for anything specific here, just
    //      want to see and give credit for any work you do to test class
    //      RationalArray)
    struct ExecuteTest
    {
      ExecuteTest()
      {
        try
        {
          Utilities::TimerMS timer;
          std::clog << "INFO:  Regression Test for \"class Library::RationalArray\" has started.\n";
          testRationalArray( std::clog );
          std::clog << "INFO:  Regression Test for \"class Library::RationalArray\" has completed successfully in " << timer << " milliseconds.\n\n";
        }

        catch( const std::exception & ex )
        {
          std::cerr << "FAILURE:  Regression test for \"class Library::RationalArray\" failed.\nProgram terminated\n"
                    << ex.what() << std::endl;

          // Reminder:  Objects with automatic storage are not destroyed by calling std::exit().  In this case, this is okay.
          std::exit(- __LINE__);
        }
      }
    } run;  // namespace scoped object runs before main() is invoked.
  }  // anonymous namespace
}  // namespace Library::RegressionTest::RationalArray
