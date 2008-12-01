// $Id: SLogger.cxx,v 1.2.2.1 2008-12-01 14:52:57 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
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

/// Hard-coded maximum length of the source names
static const string::size_type MAXIMUM_SOURCE_NAME_LENGTH = 18;

/**
 * Many classes in SFrame inherit from TObject. It makes it quite easy to
 * show the correct name of the class sending the message through the
 * TObject functionality.
 *
 * For instance the SLogger object defined in SCycleBaseBase will print
 * the correct name of the user analysis cycle, even though the user doesn't
 * have to set anything for this.
 *
 * @param source Pointer to the object that will print the messages (usually "this")
 */
SLogger::SLogger( const TObject* source )
   : m_objSource( source ), m_strSource( "" ), m_activeType( INFO ) {

   m_logWriter = SLogWriter::Instance();

}

/**
 * Since SLogger has to be usable by classes not inheriting from TObject
 * as well, the user can create it with specifying an std::string name
 * under which the messages should be displayed.
 *
 * @param source Name of the component sending the messages
 */
SLogger::SLogger( const string& source )
   : m_objSource( 0 ), m_strSource( source ), m_activeType( INFO ) {

   m_logWriter = SLogWriter::Instance();

}

/**
 * This constructor is necessary to be able to freely copy objects
 * using SLogger.
 *
 * @param parent The SLogger object that should be cloned
 * @see SLogger::operator=
 */
SLogger::SLogger( const SLogger& parent )
   : std::basic_ios< SLogger::char_type, SLogger::traits_type >(),
     ostringstream() {

   *this = parent;

}

/**
 * The destructor is literally not doing anything...
 */
SLogger::~SLogger() {

}

void SLogger::SetSource( const TObject* source ) {

   m_objSource = source;
   m_strSource = "";
   return;

}

void SLogger::SetSource( const std::string& source ) {

   m_objSource = 0;
   m_strSource = source;
   return;

}

/**
 * Operator for copying the configuration of one SLogger object into
 * another. It is mostly used by the copy constructor.
 *
 * @param parent The SLogger object that should be cloned
 * @see SLogger(const SLogger&)
 */
SLogger& SLogger::operator= ( const SLogger& parent ) {

   m_objSource = parent.m_objSource;
   m_strSource = parent.m_strSource;
   m_logWriter = SLogWriter::Instance();

   return *this;

}

/**
 * This function does the heavy-lifting of the message sending.
 * It doesn't use any of the std::ostringstream functionality itself.
 * It receives the type of the message and the message itself.
 * If the type is such that it should be displayed, it slices the
 * message into multiple lines and sends it line-by-line to
 * SLogWriter.
 *
 * @param type The type of the message to send
 * @param message The text of the message
 */
void SLogger::Send( const SMsgType type, const string& message ) const {

   if( type < m_logWriter->GetMinType() ) return;

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
      m_logWriter->Write( type, message_to_send.str() );

      if( current_pos == message.npos ) break;
      previous_pos = current_pos + 1;

   }

   return;

}

/**
 * This function sends the text that's been accumulated by the
 * std::ostringstream base class to the output. It actually uses the
 * SLogger::Send(const SMsgType,const string&) function to do the work.
 * After the message is sent, it clears the text buffer of the object.
 */
void SLogger::Send() {

   //
   // Call the "other" send(...) function:
   //
   this->Send( m_activeType, this->str() );

   //
   // Reset the stream buffer:
   //
   this->str( "" );

   return;

}

/**
 * This is just a convenience function to be able to just use the "<<"
 * operator for printing messages. This stream modifier acts very
 * similarly to std::endl. You can use it like:
 *
 * <code>
 *   logger << "Some text" << SLogger::endmsg;
 * </code>
 */
SLogger& SLogger::endmsg( SLogger& logger ) {

   logger.Send();
   return logger;

}
