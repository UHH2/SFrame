// $Id: SLogWriter.cxx,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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
#include <iostream>

// Local include(s):
#include "../include/SLogWriter.h"

// Uncomment following line for colored output:
#define USE_COLORED_CONSOLE

using namespace std;

SLogWriter* SLogWriter::m_instance = 0;

SLogWriter* SLogWriter::instance() {

   if( ! m_instance ) {
      m_instance = new SLogWriter();
   }

   return m_instance;
}

SLogWriter::~SLogWriter() {

}

void SLogWriter::write( SMsgType type, const std::string& line ) const {

   if( type < m_minType ) return;
   map< SMsgType, std::string >::const_iterator stype;
   if( ( stype = m_typeMap.find( type ) ) == m_typeMap.end() ) return;
#ifdef USE_COLORED_CONSOLE
   cout << m_colorMap.find( type )->second << " (" << stype->second << ")  "
        << line << "\033[0m" << endl;
#else
   cout << " (" << stype->second << ")  " << line  << endl;
#endif // USE_COLORED_CONSOLE

   return;

}

void SLogWriter::setMinType( SMsgType type ) {

   m_minType = type;
   return;

}

SMsgType SLogWriter::minType() const {

   return m_minType;

}

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
