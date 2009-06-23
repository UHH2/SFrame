// $Id$
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

// System include(s):
extern "C" {
#   include <unistd.h>
}

// STL include(s):
#include <iostream>

// Local include(s):
#include "../include/SLogWriter.h"

using namespace std;

SLogWriter* SLogWriter::m_instance = 0;

/**
 * This function implements the singleton design pattern for the
 * class. Since the constructor of the class is "protected", the
 * user can not create it manually. He/she has to access a (single)
 * instance of the object with this function.
 */
SLogWriter* SLogWriter::Instance() {

   if( ! m_instance ) {
      m_instance = new SLogWriter();
   }

   return m_instance;
}

/**
 * This is also one of the "don't do anything" destructors.
 */
SLogWriter::~SLogWriter() {

}

/**
 * This function is the heavy-lifter of the class. It writes the received
 * message to the console. The function assumes that the message has no
 * line breaks and that it has been formatted by SLogger.
 *
 * @param type The message type
 * @param line A single line of message to be displayed.
 */
void SLogWriter::Write( SMsgType type, const std::string& line ) const {

   if( type < m_minType ) return;
   map< SMsgType, std::string >::const_iterator stype;
   if( ( stype = m_typeMap.find( type ) ) == m_typeMap.end() ) return;

   // Only print a coloured output if it's going to the console:
   if( isatty( STDOUT_FILENO ) ) {
      cout << m_colorMap.find( type )->second << " (" << stype->second << ")  "
           << line << "\033[0m" << endl;
   } else {
      cout << " (" << stype->second << ")  " << line  << endl;
   }

   return;

}

/**
 * This function sets the minimum message type that should still be
 * displayed. All messages having a higher priority will be displayed
 * as well of course.
 *
 * @param type The value of the minimum type
 * @see SLogWriter::GetMinType
 */
void SLogWriter::SetMinType( SMsgType type ) {

   m_minType = type;
   return;

}

/**
 * Not much to say here.
 *
 * @see SLogWriter::SetMinType
 */
SMsgType SLogWriter::GetMinType() const {

   return m_minType;

}

/**
 * The constructor takes care of filling the two std::map-s that are
 * used for generating the nice colured output.
 */
SLogWriter::SLogWriter() {

   m_typeMap[ VERBOSE ] = "VERBOSE";
   m_typeMap[ DEBUG ]   = " DEBUG ";
   m_typeMap[ INFO ]    = " INFO  ";
   m_typeMap[ WARNING ] = "WARNING";
   m_typeMap[ ERROR ]   = " ERROR ";
   m_typeMap[ FATAL ]   = " FATAL ";
   m_typeMap[ ALWAYS ]  = "ALWAYS ";

   m_colorMap[ VERBOSE ] = "\033[1;34m";
   m_colorMap[ DEBUG ]   = "\033[34m";
   m_colorMap[ INFO ]    = "\033[32m";
   m_colorMap[ WARNING ] = "\033[35m";
   m_colorMap[ ERROR ]   = "\033[31m";
   m_colorMap[ FATAL ]   = "\033[1;31;40m";
   m_colorMap[ ALWAYS ]  = "\033[30m";

   m_minType = INFO;

}
