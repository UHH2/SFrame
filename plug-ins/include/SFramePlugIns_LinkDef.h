// Dear emacs, this is -*- c++ -*-
// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Plug-ins
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

// The plug-in classes:
#pragma link C++ class SParticle+;
#pragma link C++ class std::vector<SParticle>+;

#pragma link C++ class ProofSummedVar< Short_t >+;
#pragma link C++ class ProofSummedVar< UShort_t >+;
#pragma link C++ class ProofSummedVar< Int_t >+;
#pragma link C++ class ProofSummedVar< UInt_t >+;
#pragma link C++ class ProofSummedVar< Long_t >+;
#pragma link C++ class ProofSummedVar< ULong_t >+;
#pragma link C++ class ProofSummedVar< Long64_t >+;
#pragma link C++ class ProofSummedVar< ULong64_t >+;

#pragma link C++ class ProofSummedVar< Float_t >+;
#pragma link C++ class ProofSummedVar< Double_t >+;

#pragma link C++ class ProofSummedVar< std::vector< Short_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< UShort_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< Int_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< UInt_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< Long_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< ULong_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< Long64_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< ULong64_t > >+;

#pragma link C++ class ProofSummedVar< std::vector< Float_t > >+;
#pragma link C++ class ProofSummedVar< std::vector< Double_t > >+;

#pragma link C++ class SH1F+;
#pragma link C++ class SH1D+;
#pragma link C++ class SH1I+;

#endif // __CINT__
