// Dear emacs, this is -*- c++ -*-
// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: User
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

#ifndef SFRAME_USER_FirstCycle_H
#define SFRAME_USER_FirstCycle_H

// STL include(s):
#include <vector>
#include <string>

// Local include(s):
#include "core/include/SCycleBase.h"
#include "plug-ins/include/SParticle.h"
#include "plug-ins/include/SSummedVar.h"

/**
 * Example cycle reading a SusyView ntuple and writing out an
 * ntuple of its own.
 */
class FirstCycle : public SCycleBase {

public:
   FirstCycle();

   virtual void BeginCycle() throw( SError );
   virtual void EndCycle() throw( SError );

   virtual void BeginInputData( const SInputData& ) throw( SError );
   virtual void EndInputData  ( const SInputData& ) throw( SError );

   virtual void BeginMasterInputData( const SInputData& ) throw( SError );
   virtual void EndMasterInputData  ( const SInputData& ) throw( SError );

   virtual void BeginInputFile( const SInputData& ) throw( SError );

   virtual void ExecuteEvent( const SInputData&, Double_t weight ) throw( SError );

private:
   //
   // Names of the input/output trees:
   //
   std::string m_recoTreeName;
   std::string m_metaTreeName;

   enum ValHistsType { GENERAL, ELECTRON, MUON, JETS };
   void FillValidationHists( ValHistsType, const TString& status );

   //
   // Example properties
   //
   std::string m_stringVariable;
   int         m_intVariable;
   double      m_doubleVariable;
   bool        m_boolVariable;
   std::vector< int >         m_intVecVariable;
   std::vector< double >      m_doubleVecVariable;
   std::vector< std::string > m_stringVecVariable;
   std::vector< bool >        m_boolVecVariable;

   //
   // The input variables
   //
   Int_t                  m_El_N;
   std::vector< double >* m_El_p_T;
   std::vector< double >* m_El_eta;
   std::vector< double >* m_El_phi;
   std::vector< double >* m_El_E;

   //
   // The output variables
   //
   int                      m_o_example_variable;
   std::vector< double >    m_o_El_p_T;
   std::vector< SParticle > m_o_El;

   //
   // Metadata tree with separate entries for each electron:
   //
   TTree* m_electronTree;
   Double_t m_meta_El_p_T;
   Double_t m_meta_El_eta;
   Double_t m_meta_El_phi;
   Double_t m_meta_El_E;

   //
   // Some counters:
   //
   SSummedVar< Int_t > m_allEvents; //!
   SSummedVar< Int_t > m_passedEvents; //!
   SSummedVar< std::vector< Int_t > > m_test; //!

   ClassDef( FirstCycle , 0 );

}; // class FirstCycle

#endif // SFRAME_USER_FirstCycle_H
