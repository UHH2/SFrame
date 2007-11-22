// Dear emacs, this is -*- c++ -*-
// $Id: SFrameCore_LinkDef.h,v 1.2 2007-11-22 18:19:25 krasznaa Exp $
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

// The base classes:
#pragma link C++ class SCycleBaseBase+;
#pragma link C++ class SCycleBaseConfig+;
#pragma link C++ class SCycleBaseHist+;
#pragma link C++ class SCycleBaseNTuple+;
#pragma link C++ class SCycleBase+;
#pragma link C++ class SCycleController+;

// These are only needed for ROOT >=5.14. For some reason these
// dictionaries are not in the new ROOT versions by default...
#pragma link C++ class std::vector<int>;
#pragma link C++ class std::vector<double>;

#endif // __CINT__
