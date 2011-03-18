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

#ifndef SFRAME_CORE_SConstants_H
#define SFRAME_CORE_SConstants_H

namespace SFrame {

   static const char* CycleConfigName      = "CycleConfig";
   static const char* CurrentInputDataName = "CurrentInputData";
   static const char* RunStatisticsName    = "RunStatistics";
   static const char* ProofOutputName      = "PROOF_OUTPUTFILE";
   static const char* ProofOutputDirName   = "jobTempOutput_XXXXXX";
   static const char* ProofOutputFileName  = "SFramePROOFTempOutput.root";

} // namespace SFrame

#endif // SFRAME_CORE_SConstants_H
