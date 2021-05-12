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


#include <iostream>
#include <exception>
#include <sstream>
#include <cstdlib>        // abs(int)
#include <cmath>          // abs(double), floor(), round()

#include "Library/Rational.hpp"
#include "Utilities/Exceptions.hpp"
#include "Utilities/Timer.hpp"


namespace Library::RegressionTest::Rational
{

  struct FailedTest               : Library::Rational::RationalExceptions  {using RationalExceptions::RationalExceptions;};
  struct   BadFractionArithmetic  : FailedTest                             {using FailedTest::FailedTest;};
  struct   FractionReductionError : FailedTest                             {using FailedTest::FailedTest;};



  void testRational(std::ostream &)
  {
    using Library::Rational;
    using namespace Library::Rational_literals;
    using namespace std::string_literals;

    try
    {
      // Test constructors
      if( notEqual(Rational(),  Rational("0/1")) )              throw FailedTest( "Default construction error detected",             __LINE__, __func__, __FILE__);
      if( Rational{} != "0/1"_r )                               throw FailedTest( "Default construction error detected",             __LINE__, __func__, __FILE__);
      if( Rational{} != 0_r )                                   throw FailedTest( "Default construction error detected",             __LINE__, __func__, __FILE__);
      if( 0 != Rational{} )                                     throw FailedTest( "Default construction error detected",             __LINE__, __func__, __FILE__);
      if( Rational(5) != 5 )                                    throw FailedTest( "Integer construction error detected",             __LINE__, __func__, __FILE__);
      if( Rational(-5LL) != -5 )                                throw FailedTest( "Long Long Integer construction error detected",   __LINE__, __func__, __FILE__);
      if( Rational{3, 5} != 3.0 / 5.0 )                         throw FailedTest( "Integer pair construction error detected",        __LINE__, __func__, __FILE__);
      if( Rational( "7/11"s ) != Rational(7, 11) )              throw FailedTest( "Standard string construction error detected",     __LINE__, __func__, __FILE__);
      if( Rational( "7/11"  ) != Rational(7, 11) )              throw FailedTest( "NTBS construction error detected",                __LINE__, __func__, __FILE__);

      Rational(0);
      Rational(0LL);
      Rational('0');
      Rational("0/1");
      Rational("0/1"s);
      Rational(0.0);
      Rational(0.0L);
      Rational(0.0F);

      -3_r;
      -3.5_r;
      9.9999999999999999999_r;
      "-3/2"_r;

      if( +(3.1415926535897932384626433832795_r - Rational::PI()) > Rational::getEpsilon() )
      {
              throw FailedTest( "Type double construction error detected",     __LINE__, __func__, __FILE__);
      }

      // Reduction tests
      if( !(Rational( 4,   8) == Rational(1, 2)) ) throw FractionReductionError("Error: 4/8 != 1/2",     __LINE__, __func__, __FILE__);
      if(   Rational(-8,   4) != -2 )              throw FractionReductionError("Error: -8/4 != -2",     __LINE__, __func__, __FILE__);
      if(   Rational( 18, -3) != -6 )              throw FractionReductionError("Error: 18/-3 != -6",    __LINE__, __func__, __FILE__);
      if(   Rational(-45,-10) !=  "9/2"_r )        throw FractionReductionError("Error: -45/-10 != 9/2", __LINE__, __func__, __FILE__);

      // Assignment tests
      {
        Rational temp;
        temp = {22, 7};
        if( temp != Rational(44, 14) )             throw BadFractionArithmetic("Error: T = 22/7  !=  44/14",    __LINE__, __func__, __FILE__);
        if( static_cast<int>(temp) != 3 )          throw BadFractionArithmetic("Error: 22/7 not rounded to 3",  __LINE__, __func__, __FILE__);
        if( static_cast<int>(temp + "1/2"_r) != 4 )throw BadFractionArithmetic("Error: 51/14 not rounded to 4", __LINE__, __func__, __FILE__);
      }

      // Casting tests
      {
        auto originalFormat = Rational::getStringFormat();
        Rational::setStringFormat(Rational::DisplayAs::FRACTION);
        if( static_cast<std::string>(Rational(17, 3)) != "17/3"s )     throw FailedTest("Error: 17/3 not casted to the string: \"17/3\"", __LINE__, __func__, __FILE__);

        Rational::setStringFormat(Rational::DisplayAs::FLOATING_POINT);
        if( Rational(static_cast<std::string>(Rational(17, 3)))   !=   Rational(17,3) )
        {
          throw FailedTest("Error: 17/3 not casted to suitable string to get back to where it started.\n"s
                          +"       expected: \"" + std::to_string(17.0L/3.0L) + "\"\n"
                          +"       found:    \"" + static_cast<std::string>(Rational(17, 3)) + '"'
                           , __LINE__, __func__, __FILE__);
        }

        Rational::setStringFormat(originalFormat);

        if( std::abs(static_cast<long double>(Rational(17, 3)) - (17.0L/3.0L)) > Rational::getEpsilon())
        {
          throw FailedTest("Error: Type double casting error detected ", __LINE__, __func__, __FILE__);
        }
      }

      // ({2,1} + {7,1} + {-3,5})/1
      if( (Rational(2) + 7 + Rational(-3, 5)).inv() != "5/42"_r) throw BadFractionArithmetic("Error: cascading addition doesn't add up", __LINE__, __func__, __FILE__);


      /////////////////////////////////////////////////////////////////////////
      // Expected (forced) errors
      /////////////////////////////////////////////////////////////////////////
      // Divide by zero error at construction
      try
      {
        Rational wrong(1, 0);  // Expect a divide by zero exception
        throw FailedTest( "Divide by zero error not detected", __LINE__, __func__, __FILE__);
      }

      catch( Rational::DivideByZeroException & ) {}  // ignore the expected exception

      // Divide by zero error on inversion
      try
      {
        Rational temp(0, 1);
        temp.inv();
        throw FailedTest( "Divide by zero error not detected", __LINE__, __func__, __FILE__);
      }

      catch( Rational::DivideByZeroException & ) {}  // ignore the expected exception





      // Test binary math operations
      {
        Rational r1(10);
        r1.set( "22/7");
        r1 = "22/7";
        r1 += 5;
        r1 += Rational::PI();
        ++r1;
        r1++;
        r1 = add("22/7", Rational::PI());
      }

      // Test compound assignment operations
      {
        Rational number1, number2, number3;
        number1.set(5);         // assign an integer to the rational object (semantically identical to =)
        number1 = 5;            // Also assigns an integer to the rational object
        number1.set(number2);
        number1 = number2;

        number1 += 5;
        number1 -= 5;
        number1 *= 5;
        number1 /= 5;

        number1++;
        ++number1;
      }
    }




    /////////////////////////////////////////////////////////////////////////
    // Unexpected error
    /////////////////////////////////////////////////////////////////////////
    catch( Utilities::AbstractException<> & ex )
    {
      std::ostringstream errorMessage;
      errorMessage << "Failed:  Class Rational regression testing has failed with unhanded Library exception \""
                   << typeid(ex).name() << '"';
      throw FailedTest(ex, errorMessage.str(), __LINE__, __func__, __FILE__ );
    }

    catch( std::exception & ex )
    {
      std::ostringstream errorMessage;
      errorMessage << "Failed:  Class Rational regression testing has failed with unhanded system exception \""
                   << typeid(ex).name() << '"';
      throw FailedTest(ex, errorMessage.str(), __LINE__, __func__, __FILE__);
    }
  } // testRational()





  /**************************************************************************
  ** Execute the regression tests.  An object of type ExecuteTest is defined at namespace Library::RegressionTest::Rational scope.
  ** Such objects are constructed prior to executing main().  The intent here is to cause class Rational to be regression tested
  ** without modifying function main(), or any other function, to invoke the regression test.  Simply compiling and linking this
  ** source file into the program will cause the regression test to be performed.  To remove the regression test from deliverable
  ** code, simply remove this source file from the files compiled and linked into the program.
  ***************************************************************************/

  namespace  // unnamed, anonymous namespace provides internal linkage
  {
    //3.a:  Call a function you write that proves to an observer the workings of
    //      class Rational  (not looking for anything specific here, just want
    //      to see and give credit for any work you do to test class Rational)
    struct ExecuteTest
    {
      ExecuteTest()
      {
        try
        {
          Utilities::TimerMS timer;
          std::clog << "INFO:  Regression Test for \"class Library::Rational\" has started.\n";
          testRational( std::clog );
          std::clog << "INFO:  Regression Test for \"class Library::Rational\" has completed successfully in " << timer << " milliseconds.\n\n";
        }

        catch( const std::exception & ex )
        {
          std::cerr << "FAILURE:  Regression test for \"class Library::Rational\" failed.\nProgram terminated\n\n\n"
                    << ex.what() << std::endl;

          // Reminder:  Objects with automatic storage are not destroyed by calling std::exit().  In this case, this is okay.
          std::exit(- __LINE__);
        }
      }
    } run;  // namespace scoped object runs before main() is invoked.
  }  // anonymous namespace
}  // Namespace Library::RegressionTest::Rational
