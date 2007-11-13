// Dear emacs, this is -*- c++ -*-
// $Id: SLogWriter.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SLogWriter_H
#define SFRAME_CORE_SLogWriter_H

// STL include(s):
#include <string>
#include <map>

// Local include(s):
#include "SMsgType.h"

/**
 * Singleton class for actually writing the formatted messages
 * to the console.
 */
class SLogWriter {

public:
   static SLogWriter* instance();
   ~SLogWriter();

   void write( SMsgType type, const std::string& line ) const;

   void setMinType( SMsgType type );
   SMsgType minType() const;

protected:
   SLogWriter();

private:
   static SLogWriter* m_instance;

   std::map< SMsgType, std::string > m_typeMap;
   std::map< SMsgType, std::string > m_colorMap;
   SMsgType                          m_minType;

}; // class SLogWriter

#endif // SFRAME_CORE_SLogWriter_H
