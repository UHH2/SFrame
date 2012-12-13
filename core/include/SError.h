// Dear emacs, this is -*- c++ -*-
// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

#ifndef SFRAME_CORE_SError_H
#define SFRAME_CORE_SError_H

// STD include(s):
#include <exception>
#include <sstream>

/**
 *   @short SFrame exception class
 *
 *          Class for all exceptions used in SFrame. All the
 *          exceptions should provide a description of the occurence
 *          and an "action request" to the framework.
 *
 * @version $Revision$
 */
class SError : public std::exception,
               public std::ostringstream {

public:
   /// Severity enumeration
   /**
    * The exception can request an action from the framework. This
    * can be one of the actions described by the enumeration values.
    */
   enum Severity {
      SkipEvent = 1,     /**< The current event should be skipped from being written */
      SkipFile = 2,      /**< Processing of the current file should stop */
      SkipInputData = 3, /**< Processing of the current input data type should stop */
      SkipCycle = 4,     /**< Running of the current cycle should stop */
      StopExecution = 5  /**< SFrame should stop completely */
   };

   /// Constructor specifying only a severity
   SError( Severity severity = SkipEvent ) throw();
   /// Constructor with description and severity
   SError( const char* description, Severity severity = SkipEvent ) throw();
   /// Copy constructor
   SError( const SError& parent ) throw();

   /// Destructor
   virtual ~SError() throw();

   /// Set the description of the exception
   void SetDescription( const char* description ) throw();
   /// Set the severity of the exception
   void SetSeverity( Severity severity ) throw();

   /// Get the description of the exception
   virtual const char* what()    const throw();
   /// Get the severity of the exception
   virtual Severity    request() const throw();

   /// Function to get the std::ostream functionality
   /**
    * A little template magic is needed to provide all the << operator
    * functionalities of std::ostream to SError. This function takes
    * care of that. So in principle you should be able to use an SError
    * object as any other kind of std::ostream object.
    */
   template < class T > SError& operator<< ( T arg ) {
      ( * ( std::ostringstream* ) this ) << arg;
      return *this;
   }

private:
   /// The severity of the exception
   /**
    * This member variable describes what the framework should do when
    * it cathces this exception.
    */
   Severity m_severity;

}; // class SError

#endif // SFRAME_CORE_SError_H
