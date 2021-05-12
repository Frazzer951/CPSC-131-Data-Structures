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


#ifndef UTILITIES_Timer_hpp
#define UTILITIES_Timer_hpp

#include <string>
#include <iostream>
#include <chrono>

namespace Utilities
{
  /***************************************************************************************************
  ** A class of objects that keeps track of CPU time used since the object was created or last reset
  ** and reports that time in Resolution units.  It implicitly converts to Resolution units so it can
  ** be used like this:
  **
  **   Timer t;                                       // begin timing some operation, or
  **   Timer t("The consumed time is:  ")             // begin timing some operation and provide results message, or
  **   Timer t("The consumed time is:  ", std::clog)  // begin timing some operation, provide results message, and provide where to write message
  **   ...
  **   std::cout << t;                                // print out how much CPU time has elapsed in Resolution units
  **
  **  If a results message was provided at construction, that message and time duration is emitted at destruction
  **
  **  Tom Bettens
  ***************************************************************************************************/
  template< typename Resolution, const char * _units, typename Clock = std::chrono::high_resolution_clock>
  class TimerType
  {
    public:
      TimerType( std::string const & message = std::string(), std::ostream& stream = std::cout )
        : _start( Clock::now() ), _message(message), _stream(&stream)
      {}

      ~TimerType()
      { if( !_message.empty() )  (*_stream) << "Timer: " << _message << *this << " (" << _units << ")\n"; }

      // Implicit casting operator
      operator typename Resolution::rep () const
      { return std::chrono::duration_cast<Resolution>(Clock::now() - _start).count(); }

      const std::string units() const
      { return _units; }

      void reset()
      { _start = Clock::now(); }



    private:
      typename Clock::time_point  _start;
      std::string                 _message;
      std::ostream*               _stream;  // storing the stream as a pointer instead of a reference allows
                                            // the compiler to synthesize the copy and copy assignment functions.
  };




  // Let's create a couple default timer types
  namespace{ char seconds[] = "seconds",    milliseconds[] = "milliseconds",  microseconds[] = "microseconds"; }
  typedef TimerType< std::chrono::microseconds,     microseconds>  TimerUS;
  typedef TimerType< std::chrono::milliseconds,     milliseconds>  TimerMS;
  typedef TimerType< std::chrono::duration<double>, seconds>       Timer;   // 1 period : 1 second
                                                                            // std::chrono::seconds uses integral Rep representation, I wanted floating point

}  // namespace Utilities

#endif
