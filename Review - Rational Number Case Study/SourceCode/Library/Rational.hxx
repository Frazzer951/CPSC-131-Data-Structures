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
** Rational inline function and template definitions
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


#ifndef RATIONAL_HXX
#define RATIONAL_HXX

#include <iostream>
#include <string>       // std::string, std::to_string()
#include <utility>      // std::move()

#include "Library/Rational.hpp"


namespace Library
{
  /********************************************
  * Constructor Definitions
  ********************************************/
  inline Rational::Rational(const int numerator) noexcept
  : _numerator(numerator), _denominator(1)
  {}

  inline Rational::Rational(const double value)
  : Rational(static_cast<long double>(value)) // delegate construction
  {}

  inline Rational::Rational(const char * value)
  : Rational(std::string(value))              // delegate construction
  {}

  // Rational r({a, b}, {x, y})  ==>  r({a, b} / {x, y})  ==>  r({a*y}, {b*x});
  inline Rational::Rational(const Rational & top, const Rational & bottom)
  : Rational(top / bottom)                    // calculate the quotient and delegate construction to the move constructor
  {}




  /********************************************
  * Unary Operator Definitions
  *
  * Note that except for the pre-increment operators, all these functions return rvalues (i.e. temporary objects). The pre-increment
  * operators return lvalues.
  ********************************************/
  inline Rational  Rational::squared   () const                { return (*this) * (*this); }
  inline Rational  Rational::inv       () const                { return {_denominator, _numerator}; }

  inline Rational  Rational::opp       () const noexcept       { return operator-(); }
  inline Rational  Rational::operator- () const noexcept       { return {-_numerator, _denominator}; }

  inline Rational  Rational::abs       () const noexcept       { return operator+(); }
  inline Rational  Rational::operator+ () const noexcept       { return *this < 0  ?  -(*this)  :  *this; }

  inline Rational& Rational::increment ()                      { return operator++(); } // pre-increment
  inline Rational& Rational::operator++()                      { return *this += 1; }

  inline Rational  Rational::increment (int)                   { return operator++(0); } // post-increment
  inline Rational  Rational::operator++(int)                   { Rational temp(*this); *this += 1; return temp; }

  inline Rational& Rational::decrement ()                      { return operator--(); } // pre-decrement
  inline Rational& Rational::operator--()                      { return *this -= 1; }

  inline Rational  Rational::decrement (int)                   { return operator--(0); } // post-decrement
  inline Rational  Rational::operator--(int)                   { Rational temp(*this); *this -= 1; return temp; }



  /********************************************
  * Binary Operator Definitions
  * Note that except for the assignment operators, all these functions return rvalues (i.e. temporary objects). The assignment
  * operators return lvalues.  Assignment operators can be chained:
  *     Rational r1, r2;
  *     r1 = r2 = 0;
  ********************************************/
  // Assignment
  inline Rational& Rational::set       (const Rational  & rhs) noexcept { return operator=(rhs); }
  inline Rational& Rational::set       (      Rational && rhs) noexcept { return operator=(std::move(rhs)); }

  // Arithmetic
  // The real work is delegated to the mutating forms using the left hand side parameter, which has been passed by value
  // (i.e lhs is a local copy of the argument passed)
  inline Rational add       (Rational lhs, const Rational & rhs)             { return lhs += rhs; }
  inline Rational operator+ (Rational lhs, const Rational & rhs)             { return lhs += rhs; }

  inline Rational sub       (Rational lhs, const Rational & rhs)             { return lhs -= rhs; }
  inline Rational operator- (Rational lhs, const Rational & rhs)             { return lhs -= rhs; }

  inline Rational mult      (Rational lhs, const Rational & rhs)             { return lhs *= rhs; }
  inline Rational operator* (Rational lhs, const Rational & rhs)             { return lhs *= rhs; }

  inline Rational div       (Rational lhs, const Rational & rhs)             { return lhs /= rhs; }
  inline Rational operator/ (Rational lhs, const Rational & rhs)             { return lhs /= rhs; }

  inline Rational pow       (Rational lhs, unsigned rhs)                     { return lhs ^= rhs; }
  inline Rational operator^ (Rational lhs, unsigned rhs)                     { return lhs ^= rhs; }

  inline Rational pow       (Rational lhs, signed   rhs)                     { return lhs ^= rhs; }
  inline Rational operator^ (Rational lhs, signed   rhs)                     { return lhs ^= rhs; }



  // Logical function operator<() is a bit more complicated and is defined in the source file. All logical operators are defined in
  // terms of operator<().  All these function throws OverflowException
  inline bool equal            (const Rational & lhs, const Rational & rhs)        { return !(lhs < rhs || rhs < lhs); } // note the position of rhs and lhs here
  inline bool operator==       (const Rational & lhs, const Rational & rhs)        { return !(lhs < rhs || rhs < lhs); }

  inline bool lessThanEqual    (const Rational & lhs, const Rational & rhs)        { return !(rhs <  lhs); } // note the position of rhs and lhs here
  inline bool operator<=       (const Rational & lhs, const Rational & rhs)        { return !(rhs <  lhs); }

  inline bool notEqual         (const Rational & lhs, const Rational & rhs)        { return !(lhs == rhs); }
  inline bool operator!=       (const Rational & lhs, const Rational & rhs)        { return !(lhs == rhs); }

  inline bool greaterThan      (const Rational & lhs, const Rational & rhs)        { return  (rhs <  lhs); } // note the position of rhs and lhs here
  inline bool operator>        (const Rational & lhs, const Rational & rhs)        { return  (rhs <  lhs); }

  inline bool greaterThanEqual (const Rational & lhs, const Rational & rhs)        { return !(lhs <  rhs); }
  inline bool operator>=       (const Rational & lhs, const Rational & rhs)        { return !(lhs <  rhs); }


  /********************************************
  * Miscellaneous Operators
  ********************************************/
  inline void Rational::setStringFormat (const DisplayAs   format)                 { Rational::defaultFormat  = format; }
  inline void Rational::setEpsilon      (const long double errorThreshold)         { Rational::epsilon        = errorThreshold; }

  inline long double         Rational::getEpsilon()                                { return Rational::epsilon; }
  inline Rational::DisplayAs Rational::getStringFormat()                           { return Rational::defaultFormat; }

  inline std::istream & operator>> ( std::istream & stream,       Rational & r )   { r.read (stream);   return stream; }
  inline std::ostream & operator<< ( std::ostream & stream, const Rational & r )   { r.write(stream);   return stream; }




  /********************************************
  * I/O Manipulators definitions
  ********************************************/
  inline Rational::set_format::set_format(Rational::DisplayAs newFormat) noexcept
    : _newFormat(newFormat), _originalFormat(Rational::getStringFormat())
  {}

  inline Rational::set_format::~set_format() noexcept
  { Rational::setStringFormat(_originalFormat);}

  inline std::ostream & operator<<(std::ostream & s, const Rational::set_format & manip) noexcept
  {
    Rational::setStringFormat(manip._newFormat);
    return s;
  }

  inline std::istream & operator>>(std::istream & s, const Rational::set_format & manip) noexcept
  {
    Rational::setStringFormat(manip._newFormat);
    return s;
  }
}  // end namespace Library
#endif
