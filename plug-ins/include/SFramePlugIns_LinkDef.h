// Dear emacs, this is -*- c++ -*-
// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Plug-ins
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

// The plug-in classes:
#pragma link C++ class SParticle+;
#pragma link C++ class vector<SParticle>+;

#pragma link C++ class ProofSummedVar<Short_t>+;
#pragma link C++ class ProofSummedVar<UShort_t>+;
#pragma link C++ class ProofSummedVar<Int_t>+;
#pragma link C++ class ProofSummedVar<UInt_t>+;
#pragma link C++ class ProofSummedVar<Long_t>+;
#pragma link C++ class ProofSummedVar<ULong_t>+;
#pragma link C++ class ProofSummedVar<Long64_t>+;
#pragma link C++ class ProofSummedVar<ULong64_t>+;

#pragma link C++ class ProofSummedVar< Float_t >+;
#pragma link C++ class ProofSummedVar< Double_t >+;

#pragma link C++ class ProofSummedVar<vector<short> >+;
#pragma link C++ class ProofSummedVar<vector<unsigned short> >+;
#pragma link C++ class ProofSummedVar<vector<int> >+;
#pragma link C++ class ProofSummedVar<vector<unsigned int> >+;
#pragma link C++ class ProofSummedVar<vector<long> >+;
#pragma link C++ class ProofSummedVar<vector<unsigned long> >+;
#pragma link C++ class ProofSummedVar<vector<long long> >+;
#pragma link C++ class ProofSummedVar<vector<unsigned long long> >+;

#pragma link C++ class ProofSummedVar<vector<float> >+;
#pragma link C++ class ProofSummedVar<vector<double> >+;

// These dictionaries are not created by ROOT automatically:
#pragma link C++ class pair<string,unsigned int>+;
#pragma link C++ class map<string,unsigned int>+;

#pragma link C++ class ProofSummedVar<map<string,int> >+;
#pragma link C++ class ProofSummedVar<map<string,unsigned int> >+;
#pragma link C++ class ProofSummedVar<map<string,float> >+;
#pragma link C++ class ProofSummedVar<map<string,double> >+;

#pragma link C++ class SH1F+;
#pragma link C++ class SH1D+;
#pragma link C++ class SH1I+;

#endif // __CINT__
