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

// Local include(s):
#include "../include/SError.h"

/**
 * This constructor is used most often in the code. You just specify
 * a severity to the event, then fill the description using the
 * std::ostream functionality of the object, finally throw the
 * exception object.
 *
 * @param severity The action request of the exception
 */
SError::SError( Severity severity ) throw()
   : std::exception(), std::ostringstream(), m_severity( severity ) {

}

/**
 * This constructor is useful for throwing exceptions with a simple
 * explanation. For such exceptions you don't have to create an
 * exception object, but can throw the exception like this:
 *
 * <code>
 *   throw SError( "Skip this event", SError::SkipEvent );
 * </code>
 *
 * @param description Explanation for the occurance
 * @param severity    The action request of the exception
 */
SError::SError( const char* description, Severity severity ) throw()
   : std::exception(), std::ostringstream(), m_severity( severity ) {

   this->str( description );
}

/**
 * I think this copy constructor is necessary for the correct
 * handling of the exception objects. It clones the original
 * object completely.
 *
 * @param parent The object to clone
 */
SError::SError( const SError& parent ) throw()
   : std::basic_ios< SError::char_type, SError::traits_type >(),
     std::exception(), std::ostringstream(), m_severity( parent.m_severity ) {

   this->str( parent.str() );
}

/**
 * Even though the destructor doesn't do anything, it is still mandatory to
 * define it explicitly. Otherwise the compiler would make a destructor that
 * doesn't declare that it may throw (an) exception(s), and this is an
 * error on most compilers.
 */
SError::~SError() throw() {

}

/**
 * @param description The description that should be associated with this
 *                    exception
 */
void SError::SetDescription( const char* description ) throw() {

   this->str( description );
   return;
}

/**
 * @param severity The action that should be taken as the effect of this
 *                 exception
 */
void SError::SetSeverity( Severity severity ) throw() {

   m_severity = severity;
   return;
}

/**
 * This function is re-implemented from std::exception, to give an explanation
 * of the reason why this exception was thrown.
 *
 * @returns An explanation about the exception
 */
const char* SError::what() const throw() {

   return this->str().c_str();
}

/**
 * @returns The action that should be taken because of this exception
 */
SError::Severity SError::request() const throw() {

   return m_severity;
}
