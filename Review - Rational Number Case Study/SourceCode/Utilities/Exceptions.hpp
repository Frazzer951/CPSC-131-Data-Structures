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

/**************************************************
** Intermediate C++ Mail System Project Possible Solution
**
** Thomas Bettens
** Last modified:  26-April-2015
** Last Verified:  12-June-2015
** Verified with:  VC++2015 RC, GCC 5.1,  Clang 3.5
***************************************************/


#ifndef UTILITIES_Abstract_Exception_hpp
#define UTILITIES_Abstract_Exception_hpp

#include <exception>
#include <stdexcept>
#include <string>
#include <type_traits>  // is_base_of()
#include <typeinfo>     // type_info returned from typeid()


namespace Utilities
{
  /*************************************************************************************
  ** As an error handling strategy, all exceptions generated from the Library package are derived from this Abstract Base Class
  ** (ABC), which by default extends the standard exception hierarchy.  A common extension to this pattern is for classes within the
  ** Library to define their own base class exception, derived from this one (AbstractException), and concrete exception classes for
  ** specific errors or anomalies.
  **
  **   Concepts:
  **     StandardException must provide:
  **     a)  a compatible-with--template-argument-Description constructor (e.g.  StandardException(const std::string &),
  **         StandardException(const char *)) Note that class std::exception does not meet this concept.
  *************************************************************************************/
  template <typename StandardException = std::runtime_error>
  class AbstractException : public StandardException  // Abstract Base Class
  {
    // Enforce derivation from standard exception hierarchy
    static_assert(std::is_base_of<std::exception, StandardException>::value, "AbstractException does not derive from std::exception");

    public:
      AbstractException            ()                          = delete;
      AbstractException            (const AbstractException&)  = default;
      AbstractException& operator= (const AbstractException&)  = default;
      AbstractException            (      AbstractException&&) = default;
      AbstractException& operator= (      AbstractException&&) = default;


      // These constructors are intended to help create and propagate exceptions.  A common pattern is to throw some exception object
      // in a block of code, then catch that exception, add more information, and then throw some other (could be the same type)
      // exception object containing the information message from the one caught plus the new information.  For example:
      // Usage:
      //         try
      //         {
      //           // do some work
      //           throw SomeException      (    "Some informative message", __LINE_, __func__, __FILE__); // Constructor (1)
      //         }
      //         catch (const SomeException & ex)
      //         {
      //           // do some work
      //           throw SomeOtherException (ex, "Some informative message", __LINE_, __func__, __FILE__); // Constructor (2)
      //         }

      // (1)
      AbstractException(const std::string &   description,
                        const int             lineNumber,
                        const std::string &   functionName,
                        const std::string &   fileName)

      : StandardException{description + "\n**** thrown at line " + std::to_string(lineNumber)
                          + " in function \"" + functionName + "\" in file \"" + fileName + "\"\n"}
      {}



      // (2)
      AbstractException(const std::exception &   exception,
                        const std::string    &   description,
                        const int                lineNumber,
                        const std::string    &   functionName,
                        const std::string    &   fileName
                       )
      : AbstractException{description, lineNumber, functionName, fileName}  // delegate construction
      {
        _description = exception.what();
      }

      // Returns the information contained in the exception object with some header information.  Note that the header includes the
      // name of the exception class as provided by operator typeid.  It's important that typeid not be executed in a base class
      // constructor as that would return the name of the base class and not the name of the most derived class.  In this case, the
      // name of the exception class is obtained when the client requests to see the contents of the message (i.e. function what()),
      // not when the exception objects is created.
      const char * what() const noexcept override
      {
        std::string header{std::string{"Exception:  "} + typeid(*this).name() + '\n'};
        header += std::string(header.size(), '-') + '\n';      // underline the header with just enough characters to match the header's size

        _description += header + StandardException::what() + "\n\n\n";
        return _description.c_str();
      }

      // Pure virtual function prevents creation of objects, making this an abstract class
      ~AbstractException()  override = 0;


    private:
      mutable std::string   _description;  // used to hold the return value of function what(), which could be invoked on constant
                                           // objects, hence the mutable qualifier
  };

  // Class member definitions
  template <typename T>
  inline AbstractException<T>::~AbstractException()  // Pure virtual destructor definition
  {}
} // namespace Utilities

#endif
