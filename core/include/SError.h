// Dear emacs, this is -*- c++ -*-
// $Id: SError.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

#ifndef SFRAME_CORE_SError_H
#define SFRAME_CORE_SError_H

// STD include(s):
#include <exception>
#include <sstream>

/**
 * Class for all exceptions used in SFrame. All the exceptions
 * should provide a description of the occurence and an "action
 * request" to the framework.
 */
class SError : public std::exception,
               public std::ostringstream {

public:
   enum Severity { SkipEvent = 1,
                   SkipFile = 2,
                   SkipInputData = 3,
                   SkipCycle = 4,
                   StopExecution = 5 };

   //
   // Different kinds of constructors
   //
   SError( Severity severity = SkipEvent ) throw();
   SError( const char* description, Severity severity = SkipEvent ) throw();
   SError( const SError& parent ) throw();

   //
   // Destructor
   //
   virtual ~SError() throw();

   //
   // Basic "setter" methods
   //
   void setDescription( const char* description ) throw();
   void setSeverity( Severity severity ) throw();

   //
   // Basic "getter" methods:
   //
   virtual const char* what()    const throw();
   virtual Severity    request() const throw();

   //
   // Template function to get the std::ostream functionality
   //
   template < class T > SError& operator<< ( T arg ) {
      ( * ( std::ostringstream* ) this ) << arg;
      return *this;
   }

private:
   Severity m_severity;

}; // class SError

#endif // SFRAME_CORE_SError_H
