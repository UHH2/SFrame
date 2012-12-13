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
#include "../include/SOutputFile.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SOutputFile )
#endif // DOXYGEN_IGNORE

/**
 * @param name Name of this object in memory
 * @param fileName Name of the output file created by a cycle
 */
SOutputFile::SOutputFile( const char* name, const TString& fileName )
   : TNamed( name, "Temporary SFrame file descriptor" ),
     m_fileName( fileName ) {

}

/**
 * @param fileName Name of the output file created by a cycle
 */
void SOutputFile::SetFileName( const TString& fileName ) {

   m_fileName = fileName;
   return;
}

/**
 * @returns The name of the output file created by a cycle
 */
const TString& SOutputFile::GetFileName() const {

   return m_fileName;
}
