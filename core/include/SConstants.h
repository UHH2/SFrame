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

#ifndef SFRAME_CORE_SConstants_H
#define SFRAME_CORE_SConstants_H

/**
 * @short Namespace for some simple constants used internally in the code
 *
 *        The code uses some hardcoded constants in a few places. To make life a
 *        little easier, those constants are used by including them from this
 *        file.
 *
 * @version $Revision$
 */
namespace SFrame {

   /// Name of the SCycleConfig object when sending it to the workers on PROOF
   static const char* CycleConfigName      = "CycleConfig";
   /// Name of the SInputData object when sending it to the workers on PROOF
   static const char* CurrentInputDataName = "CurrentInputData";
   /// Name of the SCycleStatistics when sending it back from the PROOF workers
   static const char* RunStatisticsName    = "RunStatistics";
   /// Name of the TNamed object given to the cycle to get the output file name
   static const char* ProofOutputName      = "PROOF_OUTPUTFILE";
   /// Directory pattern for creating a temporary local directory
   static const char* ProofOutputDirName   = "jobTempOutput_XXXXXX";
   /// Name of the temporary local file created in LOCAL mode for output ntuples
   static const char* ProofOutputFileName  = "SFramePROOFTempOutput.root";

} // namespace SFrame

#endif // SFRAME_CORE_SConstants_H
