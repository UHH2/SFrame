// Dear emacs, this is -*- c++ -*-
// $Id: SFrameCore_LinkDef.h,v 1.3.2.1 2008-12-01 14:52:56 krasznaa Exp $
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
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;

// The interface classes:
//#pragma link C++ class ISCycleBase+;
#pragma link C++ class ISCycleBaseNTuple+;
#pragma link C++ class ISCycleBaseHist+;
#pragma link C++ class ISCycleBaseConfig+;
//#pragma link C++ class ISCycleBaseExec+;

// The objects sent over the network when running on PROOF:
#pragma link C++ class SGeneratorCut+;
#pragma link C++ class std::vector<SGeneratorCut>+;
#pragma link C++ class SFile+;
#pragma link C++ class std::vector<SFile>+;
#pragma link C++ class STree+;
#pragma link C++ class std::vector<STree>+;
#pragma link C++ class SEVTree+;
#pragma link C++ class std::vector<SEVTree>+;
#pragma link C++ class SInputData+;
#pragma link C++ class std::vector<SInputData>+;
#pragma link C++ class std::pair<std::string,std::string>+;
#pragma link C++ class std::vector<std::pair<std::string,std::string> >+;
#pragma link C++ class SCycleConfig+;
#pragma link C++ class SCycleOutput+;

// The base classes:
#pragma link C++ class SCycleBaseBase+;
#pragma link C++ class SCycleBaseConfig+;
#pragma link C++ class SCycleBaseHist+;
#pragma link C++ class SCycleBaseNTuple+;
#pragma link C++ class SCycleBaseExec+;
#pragma link C++ class SCycleBase+;
#pragma link C++ class SCycleController+;

// These are only needed for ROOT >=5.14. For some reason these
// dictionaries are not in the new ROOT versions by default...
#pragma link C++ class std::vector<int>+;
//#pragma link C++ class std::vector<double>+;

#endif // __CINT__
