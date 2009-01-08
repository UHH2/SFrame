// Dear emacs, this is -*- c++ -*-
// $Id: SOutputFile.h,v 1.1.2.1 2009-01-08 16:09:32 krasznaa Exp $
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

#ifndef SFRAME_CORE_SOutputFile_H
#define SFRAME_CORE_SOutputFile_H

// ROOT include(s):
#include <TNamed.h>
#include <TString.h>

class SOutputFile : public TNamed {

public:
   SOutputFile( const char* name, const TString& fileName );

   void SetFileName( const TString& fileName );
   const TString& GetFileName() const;

private:
   TString m_fileName; ///< Name of the temporary file holding the output ntuples

#ifndef DOXYGEN_IGNORE
   ClassDef( SOutputFile, 1 );
#endif // DOXYGEN_IGNORE

}; // class SOutputFile

#endif // SFRAME_CORE_SOutputFile_H
