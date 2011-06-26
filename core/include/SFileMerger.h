// Dear emacs, this is -*- c++ -*-
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

#ifndef SFRAME_CORE_SFileMerger_H
#define SFRAME_CORE_SFileMerger_H

// STL include(s):
#include <vector>

// ROOT include(s):
#include <TString.h>

// Local include(s):
#include "SError.h"
#include "SLogger.h"

// Forward declaration(s):
class TFile;

/**
 *   @short Helper class for merging the TTree contents of ROOT files
 *
 *          This is a dumbed-down version of the TFileMerger class written
 *          for PROOF. Unfortunately TFileMerger has some weird behaviour,
 *          otherwise I would've just used that class.
 *
 *          The class loops over all input files specified with AddInput(),
 *          and copies the TTree-s from them into the output file.
 *
 *          Note that the output can be an existing file. In this case the
 *          TTrees from the input files are merged into the TTrees already
 *          existing in the output file.
 *
 * @version $Revision$
 */
class SFileMerger {

public:
   /// Default constructor
   SFileMerger();
   /// Destructor
   ~SFileMerger();

   /// Add an input file that should be processed
   void AddInput( const TString& fileName ) throw( SError );
   /// Specify the output of the merging
   void SetOutput( const TString& fileName ) throw( SError );

   /// Execute the merging itself
   void Merge() throw( SError );

private:
   /// Close all open files
   void CloseFiles();
   /// Merge the contents of one directory
   void MergeDirectory( TDirectory* input, TDirectory* output ) throw( SError );

   std::vector< TFile* > m_inputFiles; ///< List of all specified input files
   TFile*                m_outputFile; ///< The output file

   mutable SLogger m_logger; ///< Object for logging some messages

}; // class SFileMerger

#endif // SFRAME_CORE_SFileMerger_H
