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

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;

// The objects sent over the network when running on PROOF:
#pragma link C++ class SGeneratorCut+;
#pragma link C++ class std::vector<SGeneratorCut>+;
#pragma link C++ class SDataSet+;
#pragma link C++ class std::vector<SDataSet>+;
#pragma link C++ class SFile+;
#pragma link C++ class std::vector<SFile>+;
#pragma link C++ class STree+;
#pragma link C++ class std::vector<STree>+;
#pragma link C++ class std::pair<int,vector<STree> >+; // Can't use std::vector notation here...
#pragma link C++ class std::map<int,std::vector<STree> >+;
#pragma link C++ class SInputData+;
#pragma link C++ class std::vector<SInputData>+;
#pragma link C++ class std::pair<std::string,std::string>+;
#pragma link C++ class std::vector<std::pair<std::string,std::string> >+;
#pragma link C++ class SCycleConfig+;
#pragma link C++ class SCycleOutput+;
#pragma link C++ class SCycleStatistics+;
#pragma link C++ class SOutputFile+;

// The base classes:
#pragma link C++ class ISCycleBaseConfig+;
#pragma link C++ class ISCycleBaseHist+;
#pragma link C++ class ISCycleBaseNTuple+;
#pragma link C++ class SCycleBaseBase+;
#pragma link C++ class SCycleBaseConfig+;
#pragma link C++ class SCycleBaseHist+;
#pragma link C++ class SCycleBaseNTuple+;
#pragma link C++ class SCycleBaseExec+;
#pragma link C++ class ISCycleBase+;
#pragma link C++ class SCycleBase+;

#endif // __CINT__
