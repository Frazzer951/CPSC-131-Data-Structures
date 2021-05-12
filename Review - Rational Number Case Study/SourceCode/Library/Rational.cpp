/******************************************************************************
** (C) Copyright 2015 by Thomas Bettens. All Rights Reserved.
**
** DISCLAIMER: The authors have used their best efforts in preparing this code. These efforts include the development, research, and
** testing of the theories and programs to determine their effectiveness. The authors make no warranty of any kind, expressed or
** implied, with regard to these programs or to the documentation contained within. The authors shall not be liable in any event for
** incidental or consequential damages in connection with, or arising out of, the furnishing, performance, or use of these libraries
** and programs.  Distribution without written consent from the authors is prohibited.
******************************************************************************/

#include <cmath>      // abs(double), floor(), round()
#include <cstdlib>    // abs(int), size_t
#include <exception>
#include <iomanip>    // manipulators with parameters
#include <ios>        // ios_base
#include <iostream>
#include <limits>     // numeric_limits
#include <numeric>    // gcd(), lcm()
#include <sstream>    // istringstream
#include <string>
#include <type_traits>    // is_integral<>, is_signed<>

#include "Library/Rational.hpp"

namespace Library
{
  /********************************************
  * Private, non-class implementation details
  ********************************************/
  namespace  // unnamed, anonymous namespace provides internal linkage
  {
    // return left + right if the sum will not overflow the range T
    template <typename T>
    T safeAdd(T left, T right)
    {
      // Compile time constraint checks
      static_assert (std::is_integral<T>::value, "Template function \"safeAdd(T, T)\" requires integral argument types T");
      static_assert (std::is_signed<T>::value,   "Template function \"safeAdd(T, T)\" requires signed argument types T");

      // Easy out:  x + 0 = x, so just return x
      if(left  == 0) return right;
      if(right == 0) return left;

      // Safe if left and right have opposite signs
      if( (left < 0  &&  right > 0)  ||  (left > 0  &&  right < 0) )   return left + right;    // opposite signs, okay

      // if both left and right are negative, then if no underflow then return the sum
      if( left < 0  &&  std::numeric_limits<T>::min() - left <= right) return left + right;    // okay, it fits

      // if both left and right are positive, then if no overflow then return the sum
      if( left > 0  &&  std::numeric_limits<T>::max() - left >= right) return left + right;    // okay, it fits


      // Adding left and right will exceed the range of T (overflow or underflow)
      std::ostringstream errorMessage;
      errorMessage << "Warning:  Underflow or Overflow detected while adding \"" << left << "\" and \"" << right <<  "\"\n"
                   << "          Range is from \"" << std::numeric_limits<T>::min() << "\" to \"" << std::numeric_limits<T>::max() << '"';
      throw Rational::OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
    }


    // return left * right if no overflow has been detected
    template <typename T>
    T safeMult(T left, T right)
    {
      static_assert (std::is_integral<T>::value, "Template function \"safeMult(T, T)\" requires integral argument types T");

      if(left == 0  ||  right == 0)  return 0;

      // Unlike the safeAdd above where we could detect the overflow before performing the operation, here we have to perform the
      // operation and save the results, then try to get back to the original operands using the saved results.

      auto result = left * right;
      if(result / right == left)    return result;


      // Multiplying left and right will exceed range (overflow or underflow)
      std::ostringstream errorMessage;
      errorMessage << "Warning:  Underflow or Overflow detected while multiplying \"" << left << "\" and \"" << right <<  "\"\n"
                   << "          Range is from \"" << std::numeric_limits<T>::min() << "\" to \"" << std::numeric_limits<T>::max() << '"';
      throw Rational::OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
    }


    // This singleton template allows Rational constants to be defined. Frequently used constants incur the cost of construction and
    // normalization only upon first usage. Usage:
    //    const Rational & constant = rationalConstant<numerator, denominator>();
    template <unsigned long long N, unsigned long long D>
    inline const Rational& rationalConstant()
    {
      static const Rational instance(N, D);
      return instance;
    }
  } // unnamed, anonymous namespace






  /********************************************
  * Class Attribute and Constants Definitions
  ********************************************/
  long double           Rational::epsilon        = 5E-9L;  // 5 times 10 to the negative 9th  (0.000000005L)
  Rational::DisplayAs   Rational::defaultFormat  = Rational::DisplayAs::FRACTION;

  // Constants expressed as a Rational number managed by a singleton
  const Rational & Rational::E()   { return rationalConstant<1084483, 398959>(); }
  const Rational & Rational::PHI() { return rationalConstant<514229,  317811>(); }
  const Rational & Rational::PI()  { return rationalConstant<103993,  33102> (); } // Other choices [n/d(accuracy)]:  2
                                                                                   //   2/7(2), 355/113(6), 103993/33102(9),
                                                                                   //   80143857/25510582(15),
                                                                                   //   2646693125139304345/842468587426513207(37)



  /********************************************
  * Constructors
  ********************************************/
  Rational::Rational(const long long numerator, const long long denominator)
  : _numerator(numerator), _denominator(denominator)
  {
    if( _denominator != 0 ) reduce();
    else
    {
      std::ostringstream errorMessage;
      errorMessage << "Error:  Attempt to construct Rational number with a zero value denominator.\n"
                   << "        Found \"" << numerator << '/' << denominator << '"';
      throw DivideByZeroException(errorMessage.str(), __LINE__, __func__, __FILE__);
    }
  }






  // This one is a little long, but its approach is pretty straight forward.  Start with a fraction (1/1) that approximates the
  // floating point value, and keep refining the approximation until it's "close enough" (within epsilon).  See John Kennedy's white
  // paper on "Converting Decimals to Fractions"
  Rational::Rational(const long double value)
  {
    // Goal is to decouple this function from the underlying types of _numerator and _denominator.  Should we want to change the
    // types of these instance attributes (a likely future enhancement) to some bigger type (for accuracy) or small type (for
    // efficiency), decoupling will minimize the ripple effect of making such a change.
    using NumType = decltype(_numerator);
    using DemType = decltype(_denominator);



    constexpr  auto MAX_NUM = std::numeric_limits<NumType>::max();
    constexpr  auto MIN_NUM = std::numeric_limits<NumType>::min();
    constexpr  auto MAX_DEM = std::numeric_limits<DemType>::max();

    // Take a quick look to see if the floating point value is close enough to an integral value, thus avoiding the more time
    // consuming algorithm below
    const auto roundedValue = std::round(value);
    if(std::abs(value - roundedValue) <= epsilon)                // close enough to an integral value, including zero
    {
      if(roundedValue <= MAX_NUM  &&  roundedValue >= MIN_NUM )  // note the mixed type implicit comparison between floating point
      {                                                          // and integral types
        _numerator   = static_cast<NumType>(roundedValue);
        _denominator = DemType(1);
      }
      else                                                       // value too big (or too small) to fit into _numerator's range
      {
        std::ostringstream errorMessage;
        errorMessage << "Warning:  Floating point value of \"" << value << "\" exceeds maximum (or minimum) Rational number value of \""
                     << Rational(MAX_NUM) << "\" (or \"" << Rational(MIN_NUM) << "\")";

        throw OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }
    }
    else
    {
      /***********************************************************************************
      ** See John Kennedy's Algorithm To Convert A Decimal To A Fraction paper (.pdf)
      ** [[ John Kennedy, Mathematics Department Santa Monica College, 01/05/2012 ]]
      ** https://sites.google.com/site/johnkennedyshome/home/downloadable-papers/dec2frac.pdf?attredirects=0
      ************************************************************************************/
      const auto  absValue             = std::abs(value);

      auto z                           = absValue;
      long double priorDenominator     = 0.0L;

      long double fractionNumerator    = 0.0L;
      long double fractionDenominator  = 1.0L;

      while(std::abs( absValue - fractionNumerator/fractionDenominator ) > epsilon  // while approximation is not close enough
            &&                                                                      // and
            z - std::floor(z) > epsilon)                                            // refinement is possible
      {
        z = 1.0L / (z - std::floor(z));
        auto temp           = fractionDenominator;
        fractionDenominator = (fractionDenominator * std::floor(z)) + priorDenominator;
        priorDenominator    = temp;
        fractionNumerator   = std::floor(absValue * fractionDenominator + 0.5L);
      }


      // Let's make sure the floating point values will fit into the numerator's and denominator's range
      if(fractionNumerator > MAX_NUM  ||  fractionDenominator > MAX_DEM)
      {
        std::ostringstream errorMessage;
        errorMessage << "Warning:  maximum range exceed during conversion from floating point value to Rational number.\n"
                     << "  Floating point value to be converted:        " << value << '\n'
                     << "  Calculated numerator and denominator values: " << fractionNumerator << ", " << fractionDenominator << '\n'
                     << "  Maximum numerator and denominator values:    " << MAX_NUM << ", " << MAX_DEM << '\n'
                     << "  Rational number range:                       " << Rational(1, MAX_DEM) << " to " << Rational(MAX_NUM) << '\n'
                     << "  Rational number precision:                   " << Rational(Rational::getEpsilon());
        throw OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      _numerator    = static_cast<NumType>((value < 0.0L)  ?  (-fractionNumerator)  :  (fractionNumerator));
      _denominator  = static_cast<DemType>(fractionDenominator);

      reduce();
    }
  }






  Rational::Rational(const std::string & value)
  {
    try
    {
      std::istringstream stream( value );       // Construct a stream from the input string and then delegating
      read(stream);                             // the work to function that knows how to parse the stream
    }
    catch(const std::ios::failure  & ex)
    {
      std::ostringstream errorMessage;
      errorMessage << "Error:  System I/O stream state failure\n";

      if(Rational::getStringFormat() == Rational::DisplayAs::FRACTION)
      {
        errorMessage << "   expected:  a fraction formatted string (i.e a/b where a and b are integers)\n";
      }
      else
      {
        errorMessage << "   expected:  a floating point formatted string (e.g. 0, 0.0, 1e6)\n";
      }

      errorMessage << "   found:     \""  << value  << '"';
      throw IllFormedFractionException(ex, errorMessage.str(), __LINE__, __func__, __FILE__);
    }
  }






  /********************************************
  * Casting Definitions
  ********************************************/
  Rational::operator long double () const
  {
    return static_cast<long double>(_numerator) / static_cast<long double>(_denominator);
  }



  Rational::operator std::string () const  // formatted in accordance with the current state of Rational::defaultFarmat
  {
    std::ostringstream result;
    result << std::setprecision(std::numeric_limits<long double>::max_digits10) << std::showpoint << std::scientific << *this;
    return result.str();
  }



  Rational::operator long long () const  // rounded towards zero
  {
    if(_numerator == 0)  return 0;                // easy out

    const auto adjustment = _denominator / 2;     // Adjustment value used for rounding and is always a positive value since the
                                                  // denominator is always positive for a normalized Rational number.

    // Round towards zero to nearest integral value by adjusting the numerator by half the denominator before performing integral
    // (i.e. truncation) arithmetic division.
    if(_numerator < 0)
    {
      constexpr auto MIN = std::numeric_limits<long long>::min();

      // Note coding as: if(_numerator - adjustment < MIN) ... may cause the underflow we're trying to avoid
      if(_numerator < MIN + adjustment)  // underflow
      {
        std::ostringstream errorMessage;
        errorMessage << "Warning:  Casting Rational value of \"" << *this
                     << "\" to type long long int exceeded minimum value of "
                     << MIN << " during the rounding process";
        throw Rational::OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      return (_numerator - adjustment) / _denominator;
    }
    else // _numerator > 0
    {
      constexpr auto MAX = std::numeric_limits<long long>::max();

      // Note coding as: if(_numerator + adjustment > MAX) ...
      //      may cause the overflow we're trying to avoid
      if(_numerator > MAX - adjustment)  // underflow
      {
        std::ostringstream errorMessage;
        errorMessage << "Warning:  Casting Rational value of \"" << *this
                     << "\" to type long long int exceeded maximum value of "
                     << MAX << " during the rounding process";
        throw Rational::OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      return (_numerator + adjustment) / _denominator;
    }
  }


  // rounded towards zero
  Rational::operator int () const  // rounded towards zero
  {
    constexpr auto MAX = std::numeric_limits<int>::max();
    constexpr auto MIN = std::numeric_limits<int>::min();

    auto result = static_cast<long long>(*this);

    if(result > MAX || result < MIN)
    {
      std::ostringstream errorMessage;
      errorMessage << "Warning:  Casting Rational value of \"" << *this
                   << "\" to type int exceeded maximum (or minimum) value of "
                   << MAX << " (or " << MIN << ')';
      throw Rational::OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
    }

    return static_cast<int>(result);
  }










  /********************************************
  * Compound assignment (accumulator) operators
  ********************************************/
  Rational& Rational::asub        (const Rational & rhs)   { return operator+=(-rhs); }  // use the unary - operation and delegate to the += function
  Rational& Rational::operator-=  (const Rational & rhs)   { return operator+=(-rhs); }

  Rational& Rational::aadd        (const Rational & rhs)   { return operator+=( rhs); }
  Rational& Rational::operator+=  (const Rational & rhs)
  {
    // This algorithm reduces, but does not eliminate, the chances for overflow.

    //   a     x         a [y/gcd(b, y)]   +   x [b/gcd(b,y)]
    //  --- + ---   =   ------------------------------------
    //   b     y                        lcm(b, y)

    try
    {
      auto & a = _numerator,
           & b = _denominator;
      auto & x = rhs._numerator,
           & y = rhs._denominator;

      // Note that subtracting same signed values and dividing two values cannot overflow or underflow the results
      const auto GCD = std::gcd( b, y );

      _numerator   = safeAdd(safeMult(a, y/GCD), safeMult(x, b/GCD));
      _denominator = std::lcm( b, y );


      // The resulting Fraction is already in lowest terms, but let's let reduce() normalize.
      reduce();
      return *this;
    }
    catch( OverflowException & ex)  // capture the overflow error, add some more information, and then throw again
    {
      std::ostringstream errorMessage;
      errorMessage << "Warning:  Underflow or Overflow detected while adding Rational Numbers \""
                   << *this << "\" and \"" << rhs << "\"";
      throw OverflowException(ex, errorMessage.str(),  __LINE__, __func__, __FILE__);
    }
  }






  Rational& Rational::adiv        (const Rational & rhs)     { return operator*=(rhs.inv()); }  // use the unary inverse operator and then delegate to the *= function
  Rational& Rational::operator/=  (const Rational & rhs)     { return operator*=(rhs.inv()); }

  Rational& Rational::amult       (const Rational & rhs)     { return operator*=(rhs); }
  Rational& Rational::operator*=  (const Rational & rhs)
  {
    // Try to limit overflow by applying GCD before multiplying. This algorithm reduces, but does not eliminate, the chances for
    // overflow.
    //   a     x       ax       a / gcd(a, y)   *   x / gcd(b, x)
    //  --- * ---  =   --  =    ---------------------------------
    //   b     y       yb       y / gcd(a, y)   *   b / gcd(b, x)
    try
    {
      auto & a = _numerator,
           & b = _denominator;
      auto & x = rhs._numerator,
           & y = rhs._denominator;


      const auto GCDay = std::gcd(a, y);
      const auto GCDbx = std::gcd(b, x);

      _numerator    = safeMult(a/GCDay, x/GCDbx);
      _denominator  = safeMult(y/GCDay, b/GCDbx);

      reduce();  // not needed?
      return *this;
    }
    catch(OverflowException & ex)  // capture the overflow error, add some more information, and then throw again
    {
      std::ostringstream errorMessage;
      errorMessage << "Warning:  Underflow or Overflow detected while multiplying Rational Numbers \""
                   << *this << "\" and \"" << rhs << '"';
      throw OverflowException(ex, errorMessage.str(), __LINE__, __func__, __FILE__);
    }
  }






  Rational& Rational::apow        (const unsigned exponent) { return operator^=(exponent); }
  Rational& Rational::operator^=  (const unsigned exponent)
  {
    // (a/b)^n  =>  a^n / b^n
    // This mathematical property thusly leads to a pretty straight forward implementation:
    //     _numerator    = std::pow(_numerator,   exponent);
    //     _denominator  = std::pow(_denominator, exponent);
    //
    // The problem, however, lies with the argument promotion of the std::pow() function.  It likes to promote arguments to floating
    // point types and return floating point types.  This in turn results in a floating point type being shoved into an integral
    // type, and most compilers rightfully warn of the potential loss of data.  Also, if Rational's _numerator and _denominator are
    // implemented with a customer integral type (an unbounded BigInteger type for example), there may be no conversion to a
    // standard floating point type.  The trade off is the time it takes to calculate the results when compared to a repeated
    // multiplication algorithm.  Of course, unless unbounded integral types for _numerator and _denominator are used, overflow and
    // underflow is a real concern.

    //  _numerator   = static_cast<decltype(_numerator)>  ( std::pow(_numerator,   exponent) );
    //  _denominator = static_cast<decltype(_denominator)>( std::pow(_denominator, exponent) );

    const auto startValue = *this;
    _numerator = _denominator = 1;

    for(auto e = exponent; e != 0; --e)  amult(startValue);   // take advantage of operator*=() ability to limit overflow

    reduce();  // not needed - amult performs reduction.  But if the algorithm is changed ...
    return *this;
  }






  Rational& Rational::apow        (signed exponent)  { return operator^=(exponent); }
  Rational& Rational::operator^=  (signed exponent)
  {
    if(exponent < 0)                                  //  a  = 1/(a ^ b)   if b < 0
    {
      set( inv() );                                   // inv() checks for, and throws exceptions on zero denominator errors
      exponent = -exponent;
    }
    return apow(static_cast<unsigned>(exponent));     //  a ^= b    if b >= 0;   // cast and delegate
  }













  /********************************************
  * misc operators
  ********************************************/

  void Rational::write(std::ostream & stream, const DisplayAs format) const
  {
    if(format == DisplayAs::FLOATING_POINT) stream << static_cast<long double>(*this);
    else
    {
      // Let's support I/O manipulator field sizes and potentially multiple threads by ensuring only a single insertion into the
      // stream occurs.
      std::ostringstream s;
      s << _numerator << '/' << _denominator;
      stream << s.str();
    }
  }



  void Rational::read(std::istream & stream, const DisplayAs format)
  {
    if(format == DisplayAs::FLOATING_POINT)
    {
      // read the floating point value from the stream, and then convert (using the conversion constructor Rational(long double))
      // that value into a rational number, and then set *this to be that rational number

      long double value = 0.0L;
      stream >> value;

      if(!stream)  // reminder, stream objects are boolean expressions (well, more accurately they provide a boolean conversion operator)
      {
        std::ostringstream errorMessage;
        errorMessage << "Error:  System I/O stream state failure\n"
                     << "        expected:  a floating point formatted string (e.g. 0, 0.0, 1e6)";
        throw IOException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      set({value});
    }
    else
    {
      char slash = '\0';
      decltype(_numerator)   num = 0; // for exception safety, let's read the values into local variables.  If
      decltype(_denominator) dem = 0; // everything works out, then move update *this

      stream >> num >> slash >> dem;  // read the integer / integer values, ignoring leading and delimiting spaces

      if(!stream)
      {
        std::ostringstream errorMessage;
        errorMessage << "Error:  System I/O stream state failure\n"
                     << "        expected:  a fraction formatted string (i.e a/b where a and b are integers)";
        throw IOException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      if(slash != '/')
      {
        std::ostringstream errorMessage;
        errorMessage << "Error:  Bad fraction form. Expecting a/b where a and b are integers\n"
                     << "        found \"" << num << ' ' << slash << ' ' << dem << '"';
        throw IllFormedFractionException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      if(dem == 0)
      {
        std::ostringstream errorMessage;
        errorMessage << "Error:  Rational number with zero denominator detected during read operation\n"
                     << "        found \"" << num << ' ' << slash << ' ' << dem << '"';
        throw DivideByZeroException(errorMessage.str(), __LINE__, __func__, __FILE__);
      }

      _numerator   = num;
      _denominator = dem;
      reduce();
    }
  }



  /********************************************
  * Logical Operators
  ********************************************/
  bool lessThan  (const Rational & lhs, const Rational & rhs) { return operator<(lhs, rhs); }
  bool operator< (const Rational & lhs, const Rational & rhs)
  {
    // This algorithm reduces, but does not eliminate, the chances for overflow.
    //   a     x        a * y/gcd(b,y)     x * b/gcd(b,y)     a * y/gcd(b,y)     x * b/gcd(b,y)
    //  --- < ---   =   --------------  <  --------------  =  --------------  <  --------------   =   a * y/gcd(b,y)  <  x * b/gcd(b,y)
    //   b     y        b * y/gcd(b,y)     y * b/gcd(b,y)     b * y/gcd(b,y)     b * y/gcd(b,y)
    try
    {
      auto & a = lhs._numerator,
           & b = lhs._denominator;
      auto & x = rhs._numerator,
           & y = rhs._denominator;

      const auto GCD = std::gcd(b, y);

      return safeMult(a, y/GCD)  <  safeMult(x, b/GCD);
    }

    catch(Rational::OverflowException & ex)  // capture the overflow error, add some more information, and then throw again
    {
      std::ostringstream errorMessage;
      errorMessage << "Warning:  Underflow or Overflow detected while comparing Rational Numbers \""
                   << lhs << "\" less than \"" << rhs << '"';
      throw Rational::OverflowException(ex, errorMessage.str(), __LINE__, __func__, __FILE__);
    }
  }




  /********************************************
  * User Defined Literal operators
  ********************************************/
  inline namespace Literals
  {
    inline namespace Rational_literals
    {
      // Integral user defined literal constants
      Rational operator"" _r(unsigned long long i)
      {
        if( i  >  static_cast<unsigned long long>(std::numeric_limits<long long>::max()) )
        {
          std::ostringstream errorMessage;
          errorMessage << "Error:  Could not create a Rational number from user defined integral literal \"" << i << "\"\n"
                       << "        Value exceeded maximum rage of \"" << std::numeric_limits<long long>::max() << '"';
          throw Rational::OverflowException(errorMessage.str(), __LINE__, __func__, __FILE__);
        }

        return static_cast<long long>(i);
      }


      // NTBS user defined literal constants  Expect a "a/b" format
      Rational operator"" _r(const char c[], std::size_t size)  // Not a NTBS, just an array of characters
      {
        try
        {
          std::istringstream s(std::string(c, size));
          Rational result;

          result.read(s, Rational::DisplayAs::FRACTION);
          return result;
        }

        catch(const std::exception & ex)
        {
          std::ostringstream errorMessage;
          errorMessage << "Error:  Could not create a Rational number from user defined string literal \"" << std::string(c, size) << '"';
          throw Rational::RationalExceptions(ex, errorMessage.str(), __LINE__, __func__, __FILE__);
        }
      }



      // Floating point user defined literal constants
      Rational operator"" _r(long double d)
      { return {d}; }
    }
  }


  /********************************************
  * Helper Functions
  ********************************************/
  void Rational::reduce()
  {
    const auto divisor = std::gcd(_numerator, _denominator);

    _numerator   /= divisor;
    _denominator /= divisor;

    if (_denominator < 0)  // normalize the fraction's sign
    {
      _numerator   = -_numerator;
      _denominator = -_denominator;
    }
  }
}
