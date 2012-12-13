// Dear emacs, this is -*- c++ -*-
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

#ifndef SFRAME_CORE_SOutputFile_H
#define SFRAME_CORE_SOutputFile_H

// ROOT include(s):
#include <TNamed.h>
#include <TString.h>

/**
 *   @short Class describing temporary ntuple files in local running mode
 *
 *          This class is used similarly to TProofOutputFile when running
 *          ntuple writing jobs in "LOCAL" mode. In this mode the output
 *          ntuples are written to a temporary file in the running directory,
 *          and finally the file's contents are merged into the output file
 *          also holding all the other outputs of the cycle.
 *
 * @version $Revision$
 */
class SOutputFile : public TNamed {

public:
   /// Constructor with object- and file name
   SOutputFile( const char* name, const TString& fileName );

   /// Set the name of the temporary ntuple file
   void SetFileName( const TString& fileName );
   /// Get the name of the temporary ntuple file
   const TString& GetFileName() const;

private:
   /// Name of the temporary file holding the output ntuples
   TString m_fileName;

#ifndef DOXYGEN_IGNORE
   ClassDef( SOutputFile, 1 )
#endif // DOXYGEN_IGNORE

}; // class SOutputFile

#endif // SFRAME_CORE_SOutputFile_H
