// $Id: SError.cxx,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

// Local include(s):
#include "../include/SError.h"

using namespace std;

SError::SError( Severity severity ) throw()
   : exception(), ostringstream(), m_severity( severity ) {

}

SError::SError( const char* description, Severity severity ) throw()
   : exception(), ostringstream(), m_severity( severity ) {

   this->str( description );

}

SError::SError( const SError& parent ) throw()
   : std::basic_ios< SError::char_type, SError::traits_type >(),
     exception(),
     ostringstream(), m_severity( parent.m_severity ) {

   this->str( parent.str() );

}

SError::~SError() throw() {

}

void SError::setDescription( const char* description ) throw() {

   this->str( description );
   return;

}

void SError::setSeverity( Severity severity ) throw() {

   m_severity = severity;
   return;

}

const char* SError::what() const throw() {

   return this->str().c_str();

}

SError::Severity SError::request() const throw() {

   return m_severity;

}
