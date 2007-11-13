// $Id: SLogger.cxx,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

// STL include(s):
#include <iomanip>
#include <iostream>

// ROOT include(s):
#include "TObject.h"

// Local include(s):
#include "../include/SLogger.h"

using namespace std;

//
// This is the hard-coded maximum length of the source names:
//
static const string::size_type MAXIMUM_SOURCE_NAME_LENGTH = 18;

SLogger::SLogger( const TObject* source )
   : m_objSource( source ), m_strSource( "" ), m_activeType( INFO ) {

   m_logWriter = SLogWriter::instance();

}

SLogger::SLogger( const string& source )
   : m_objSource( 0 ), m_strSource( source ), m_activeType( INFO ) {

   m_logWriter = SLogWriter::instance();

}

SLogger::SLogger( const SLogger& parent )
   : std::basic_ios< SLogger::char_type, SLogger::traits_type >(),
     ostringstream() {

   *this = parent;

}

SLogger::~SLogger() {

}

SLogger& SLogger::operator= ( const SLogger& parent ) {

   m_objSource = parent.m_objSource;
   m_strSource = parent.m_strSource;
   m_logWriter = SLogWriter::instance();

   return *this;

}

void SLogger::send( const SMsgType type, const string& message ) const {

   if( type < m_logWriter->minType() ) return;

   string::size_type previous_pos = 0, current_pos = 0;

   //
   // Make sure the source name is no longer than MAXIMUM_SOURCE_NAME_LENGTH:
   //
   string source_name;
   if( m_objSource ) {
      source_name = m_objSource->GetName();
   } else {
      source_name = m_strSource;
   }
   if( source_name.size() > MAXIMUM_SOURCE_NAME_LENGTH ) {
      source_name = source_name.substr( 0, MAXIMUM_SOURCE_NAME_LENGTH - 3 );
      source_name += "...";
   }

   //
   // Slice the recieved message into lines:
   //
   for( ; ; ) {

      current_pos = message.find( '\n', previous_pos );
      string line = message.substr( previous_pos, current_pos - previous_pos );

      ostringstream message_to_send;
      // I have to call the modifiers like this, otherwise g++ get's confused
      // with the operators...
      message_to_send.setf( ios::adjustfield, ios::left );
      message_to_send.width( MAXIMUM_SOURCE_NAME_LENGTH );
      message_to_send << source_name << " : " << line;
      m_logWriter->write( type, message_to_send.str() );

      if( current_pos == message.npos ) break;
      previous_pos = current_pos + 1;

   }

   return;

}

void SLogger::send() {

   //
   // Call the "other" send(...) function:
   //
   this->send( m_activeType, this->str() );

   //
   // Reset the stream buffer:
   //
   this->str( "" );

   return;

}

SLogger& SLogger::endmsg( SLogger& logger ) {

   logger.send();
   return logger;

}
