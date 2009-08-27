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

// Local include(s):
#include "../include/SOutputFile.h"
#include "../include/SLogger.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SOutputFile );
#endif // DOXYGEN_IGNORE

SOutputFile::SOutputFile( const char* name, const TString& fileName )
   : TNamed( name, "Temporary SFrame file descriptor" ), m_fileName( fileName ) {

}

void SOutputFile::SetFileName( const TString& fileName ) {

   m_fileName = fileName;
   return;

}

const TString& SOutputFile::GetFileName() const {

   return m_fileName;

}
