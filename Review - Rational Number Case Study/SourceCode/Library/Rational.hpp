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


/******************************************************************************
** Note:  Boost.Multiprecision Rational Number Types provide a rational
**        number type that is a drop-in replacement for the native C++ number
**        types, but with unlimited precision. The pedagogical work presented
**        here highlights some of the capability of rational numbers, but
**        tailored for academic study.  Prefer Boost.Multiprecision Rational
**        Number Types in production environments.
******************************************************************************/


#ifndef RATIONAL_HPP
#define RATIONAL_HPP

#include <iostream>     // std::ios_base, others
#include <string>       // std::string, std::to_string()

#include "Utilities/Exceptions.hpp"






namespace Library
{
  class Rational
  {
    /********************************************
    * Friends
    ********************************************/
    // Friends are not member functions, so accessibility (public, private) does not apply

    friend bool operator<  (const Rational & lhs, const Rational & rhs);            // Throws OverflowException

    friend std::istream & operator>>  (std::istream & stream,       Rational & r);  // Throws Rational::IOException, IllFormedFractionException, DivideByZeroException
    friend std::ostream & operator<<  (std::ostream & stream, const Rational & r);  // Throws Rational::IOException

    public:
      /********************************************
      * Inner Types
      ********************************************/
      enum class DisplayAs {FRACTION, FLOATING_POINT};  // Controls how Rational Numbers are to be displayed

      /********************************************
      * Inner Exception Type Hierarchy Definition
      ********************************************/
      struct RationalExceptions           : Utilities::AbstractException<> {using AbstractException ::AbstractException ;};  // Class Rational exception base class
      struct   DivideByZeroException      : RationalExceptions             {using RationalExceptions::RationalExceptions;};  // Inherit base class constructors
      struct   IllFormedFractionException : RationalExceptions             {using RationalExceptions::RationalExceptions;};
      struct   OverflowException          : RationalExceptions             {using RationalExceptions::RationalExceptions;};

      struct IOException                  : Utilities::AbstractException<std::ios_base::failure> {using AbstractException::AbstractException;};


      /********************************************
      * Constructors
      ********************************************/
      // Note:  All Rational numbers are constructed in normalized form.  That is, in lowest terms and numerator signed.
      //        Normalization makes the non-default constructors too complex to be constant expressions (e.g. constexpr).


      // These single argument constructors act as implicit conversion operators.  If there is not a specific operation defined, the
      // compiler will perform one implicit conversion automatically.  In general, performing an implicit conversion means
      // constructing an unnamed temporary object (an rvalue), performing the operation, and then destroying that temporary object.
      // In some cases the performance hit for constructing and destructing the temporary object may be undesirable, in which case
      // specific functions with matching arguments have been defined as an optimization.  One goal of class Rational is to support
      // mixed mode arithmetic with the native integral and floating point types.  Therefor these conversion constructors have been
      // intentionally not defined as "explicit" constructors.

      Rational(const long double    value);                                           // Throws OverflowException
      Rational(const      double    value);                                           // Throws OverflowException  Resolves ambiguity when clients
                                                                                      //   initialize with type double constants (common)
      Rational(const std::string &  value);                                           // Throws OverflowException, DivideByZeroException, IllFormedFractionException
      Rational(const char        *  value);                                           // Throws OverflowException, DivideByZeroException, IllFormedFractionException

      Rational(const long long      numerator,  const long long denominator = 1LL);   // Throws DivideByZeroException
      Rational(const int            numerator = 0)  noexcept;                         // Resolves ambiguity between Rational(long long) and Rational(long double) when
                                                                                      //   clients initialize with integer constants (common).  Also serves as the
                                                                                      //   default constructor.

      // Rational r({a, b}, {x, y})  ==>  r({a, b} / {x, y})  ==>  r({a*y}, {b*x});
      Rational(const Rational & numerator,  const Rational & denominator);            // Throws DivideByZeroException


      // The compiler synthesized copy and move constructor and assignment functions perform exactly what is needed for this class.
      // They are explicitly presented here as default implementations for the sake of completeness.  That is, the compiler
      // synthesized versions have been considered and intentionally selected.

      //Copy constructor and copy assignment
      Rational            (const Rational &  original)   noexcept = default;
      Rational& operator= (const Rational &  rhs)        noexcept = default;  // see function set(const Rational &  rhs)) below

      // Move constructor and move assignment
      Rational            (Rational       && original)   noexcept = default;
      Rational& operator= (Rational       && rhs)        noexcept = default;  //see function set(Rational &&  rhs)) below

      // Destructor
      ~Rational()                                        noexcept = default;

      // Provide corresponding conversion (aka casting) operators to the conversion constructors.
      // Usage:
      //   static_cast<T>(r)   where:  r is of type Rational and T is one of int, long long, long double, or std::string.  The
      //                               expression yields an rvalue of type T.
      explicit operator int        () const;  // rounded towards zero
      explicit operator long long  () const;  // rounded towards zero
      explicit operator long double() const;  // accurate to within plus/minus epsilon
      explicit operator std::string() const;  // formatted in accordance with the current state of Rational::defaultFarmat



      /********************************************
      * Unary operators
      ********************************************/
      // Usage:
      //       Rational r1;
      //       r1.squared();
      //
      //       r1.opp()
      //       -r1;
      //
      //       r1.increment(0);  //post-increment - the value of the argument is irrelevant, but the type must be int
      //       r1.operator++(0);
      //       r1++;
      //
      //       r1.increment();   //pre-increment
      //       r1.operator++();
      //       ++r1;

      // non-mutating unary operators
      Rational  squared     () const;           // a * a                Throws OverflowException
      Rational  inv         () const;           // inverse a/b ==> b/a  Throws DivideByZeroException

      Rational  opp         () const noexcept;  // opposite (negation) Note: -(0/1) is still (0/1), not (-0/1)
      Rational  operator-   () const noexcept;

      Rational  abs         () const noexcept;  // absolute value
      Rational  operator+   () const noexcept;

      // mutating unary operators
      Rational& increment   ();                 // ++a  (pre-increment)
      Rational& operator++  ();

      Rational  increment   (int);              // a++  (post-increment) (argument unused but necessary for unique signature)
      Rational  operator++  (int);

      Rational& decrement   ();                 // --a  (pre-decrement)
      Rational& operator--  ();

      Rational  decrement   (int);              // a--  (post-decrement) (argument unused but necessary for unique signature)
      Rational  operator--  (int);



      /********************************************
      * Binary operators
      ********************************************/
      // Note:  Symmetrical binary operators are implemented as non-member functions and use implicit conversion constructors.
      //        Operations returning modified Rational values (set(), operator=()) are implemented as member functions.
      //        [[ Stroustrup, A Tour of C++, 2014, Advice 4.7[8] ]]
      //        [[ Stroustrup, The C++ Programming Language 4e, 2013, 18.3.1; 18.3.2; Advice 18.5[5, 6, 8*, 9] ]]
      //
      //        Unary operators require no conversion or symmetry as do binary operators so they have also been implemented as
      //        member functions
      //
      // Usage:  Rational r1, r2(1,3);
      //         r1.operator+=(r2);
      //  or     r1 += r2;
      //  or     r1.aadd(r2);             r1 =  r1   +  r2
      //                               => r1 = {0,1} + {1,3}
      //                               => r1 = {0,6} + {2,6}
      //                               => r1 = {2,6}
      //                               => r1 = {1,3}
      // more usage examples:
      //       r1.set(1);
      //       r1 = 1;
      //
      //       r1.set( {2,5} );
      //       r1 = {2,5};
      //
      //       r1.set(4.75);
      //       r1 = 4.75;
      //
      //       r1.set("75/100");
      //       r1 = "75/100";

      // mutating binary operators
      Rational& aadd        (const Rational  & rhs);                // a += b   Throws OverflowException
      Rational& operator+=  (const Rational  & rhs);

      Rational& asub        (const Rational  & rhs);                // a -= b   Throws OverflowException
      Rational& operator-=  (const Rational  & rhs);

      Rational& amult       (const Rational  & rhs);                // a *= b   Throws OverflowException
      Rational& operator*=  (const Rational  & rhs);

      Rational& adiv        (const Rational  & rhs);                // a /= b   Throws DivideByZeroException, OverflowException
      Rational& operator/=  (const Rational  & rhs);

      Rational& apow        (const unsigned    exponent);           // a ^= b   Throws OverflowException
      Rational& operator^=  (const unsigned    exponent);

      Rational& apow        (      signed      exponent);           // a ^= b for b >= 0; a = 1/(a ^ b) if b < 0
      Rational& operator^=  (      signed      exponent);           // Throws DivideByZeroException, OverflowException

      Rational& set         (const Rational  & rhs) noexcept;       //a = b
      // Note operator=(const Rational & rhs) has been defined above with constructors

      Rational& set         (Rational       && rhs) noexcept;
      // Note operator=(Rational  && rhs) has been defined above with constructors



      /********************************************
      * I/O operations
      ********************************************/
      // Reads one rational number in fraction or floating point semantics and then populates (*this)
      // Also see operator>>(std::istream & stream, Rational & r) below
      void read  (std::istream & stream = std::cin, const DisplayAs format = Rational::defaultFormat);         // Throws IOException, IllFormedFractionException, DivideByZeroException

      // Writes (*this) in current class state format (fraction or floating point) semantics
      // Also see operator<<(std::ostream & stream, const Rational & r) below
      void write (std::ostream & stream = std::cout, const DisplayAs format = Rational::defaultFormat) const;  // Throws IOException



      /********************************************
      * Class Operations
      ********************************************/
      static void            setStringFormat(const DisplayAs    format);
      static void            setEpsilon     (const long double  errorThreshold);

      static long double     getEpsilon     ();
      static DisplayAs       getStringFormat();

      // Common constants
      static const Rational & PI();    // PI
      static const Rational & E();     // Euler's number
      static const Rational & PHI();   // Golden ratio Phi

      // I/O Manipulators
      // Allows client code to explicitly control the way Rational numbers are inserted or extracted from a stream without changing
      // the default format. Usage:
      // std::cout << Rational::set_format(Rational::DisplayAs::FRACTION)       << myRational
      //           << Rational::set_format(Rational::DisplayAs::FLOATING_POINT) << myOtherRational;
      class set_format
      {
        friend std::ostream & operator<<(std::ostream & s, const set_format & manip) noexcept;
        friend std::istream & operator>>(std::istream & s, const set_format & manip) noexcept;

        public:
          set_format(DisplayAs newFormat) noexcept;
          ~set_format() noexcept;

        private:
          DisplayAs  _newFormat;
          DisplayAs  _originalFormat;
      };


  private:
      /********************************************
      * Class Attributes - these declarations are defined and initialized in the source (.cpp) file
      ********************************************/
      static long double  epsilon;                // Precision for approximations and error bound
      static DisplayAs    defaultFormat;          // Contains the default format for which rational numbers are to be converted to
                                                  // strings



      /********************************************
      * Instance Attributes
      ********************************************/
      long long int _numerator   = 0;
      long long int _denominator = 1;



      /********************************************
      * Helper Functions
      ********************************************/
      void reduce();
  };  //class Rational







  /********************************
  ** Non class operations
  ** Consider these out-of-class definitions to also be part class Rational's interface
  ********************************/

  // Symmetrical, non-mutating, arithmetic, binary operators
  //      Operands for which there is a conversion constructor above may be implicitly converted to type Rational thusly enabling
  //      mixed mode arithmetic
  // Note the lhs side is intentionally passed by non-const value
  //
  // Usage:
  //       Rational r1, r2{1,3}, r3{2,5};
  //       add(r2, r3);
  //       operator+(r2, r3);
  //       r2 + r3;
  //       5  + r3;
  //       r3 + 5;
  //       "2/3"_r + r3;   // "2/3"_r is a user defined string literal
  //       4.5 + r3;
  //       r3 + 4.5;
  //       r2 + "75/100";
  //       "75/100" + r2;
  Rational add       (Rational lhs, const Rational & rhs);             // a + b    Throws OverflowException
  Rational operator+ (Rational lhs, const Rational & rhs);

  Rational sub       (Rational lhs, const Rational & rhs);             // a - b    Throws OverflowException
  Rational operator- (Rational lhs, const Rational & rhs);

  Rational mult      (Rational lhs, const Rational & rhs);             // a * b    Throws OverflowException
  Rational operator* (Rational lhs, const Rational & rhs);

  Rational div       (Rational lhs, const Rational & rhs);             // a / b    Throws DivideByZeroException, OverflowException
  Rational operator/ (Rational lhs, const Rational & rhs);

  Rational pow       (Rational lhs,       unsigned rhs);               // a ^ b    Throws OverflowException
  Rational operator^ (Rational lhs,       unsigned rhs);               // Note rhs intentionally passed by non-const value

  Rational pow       (Rational lhs,       signed   rhs);               // a ^ b for b >= 0; 1/(a ^ b) if b < 0   Throws DivideByZeroException, OverflowException
  Rational operator^ (Rational lhs,       signed   rhs);               // Note rhs intentionally passed by non-const value


  // Symmetrical, non-mutating, logical, binary operators
  //      Operands for which there is a conversion constructor above may be implicitly converted to type Rational thusly enabling
  //      mixed mode arithmetic
  // Usage:
  //       Rational r1, r2(1,3);
  //       if( equal(r1, r2)      ) {...}
  //       if( operator==(r1, r2) ) {...}
  //       if( r1      == r2      ) {...}
  //       if( r1      == "2/3"_r ) {...}
  //       if( "2/3"_r == r1      ) {...}
  //       if( 5       == r1      ) {...}
  //       if( r1      == 5       ) {...}
  //
  //       bool result = r1 >= r2;
  //       return        r1 >= 7.25;
  bool lessThan         (const Rational & lhs, const Rational & rhs);    // Throws OverflowException
  bool operator<        (const Rational & lhs, const Rational & rhs);    // declared a friend of Rational above

  bool lessThanEqual    (const Rational & lhs, const Rational & rhs);    // Throws OverflowException
  bool operator<=       (const Rational & lhs, const Rational & rhs);

  bool equal            (const Rational & lhs, const Rational & rhs);    // Throws OverflowException
  bool operator==       (const Rational & lhs, const Rational & rhs );   // declared a friend of Rational above

  bool notEqual         (const Rational & lhs, const Rational & rhs);    // Throws OverflowException
  bool operator!=       (const Rational & lhs, const Rational & rhs);

  bool greaterThan      (const Rational & lhs, const Rational & rhs);    // Throws OverflowException
  bool operator>        (const Rational & lhs, const Rational & rhs);

  bool greaterThanEqual (const Rational & lhs, const Rational & rhs);    // Throws OverflowException
  bool operator>=       (const Rational & lhs, const Rational & rhs);



  // User Defined Literal operators
  // Use namespace to help avoid suffix clashes and allow selective use.
  //     [[ Stroustrup, The C++ Programming Language 4e, 2013, 19.2.6; Advice 19.5[8] ]]
  inline namespace Literals
  {
    inline namespace Rational_literals
    {
      // Integral user defined literal constants
      Rational operator"" _r(unsigned long long i);               // Throws OverflowException

      // NTBS user defined literal constants  Expect an "a/b" format
      Rational operator"" _r(const char c[], std::size_t size);   // Throws IOException, OverflowException, DivideByZeroException, IllFormedFractionException

      // Floating point user defined literal constants
      Rational operator"" _r(long double d);                      // Throws OverflowException
    }
  }
}  // end namespace Library





// Inline functions and template definitions must be part of the header file. To help keep the size of the header file manageable
// and to keep implementation definitions separated from interface declaration, the header file has been divided into two.
// Library/Rational.hxx is not intended to be included by clients and serves only to allow Rational and related declarations to be
// separated from implementation whist making inline function and template definitions visible to all translation units.
#include "Library/Rational.hxx"       // Rational inline function and template definitions

#endif
