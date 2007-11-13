// Dear emacs, this is -*- c++ -*-
// $Id: FirstCycle.h,v 1.1.1.1 2007-11-13 12:41:09 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: User
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
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

/**
 * Example cycle reading a SusyView ntuple and writing out an
 * ntuple of its own.
 */
class FirstCycle : public SCycleBase {

public:
   FirstCycle();
   ~FirstCycle();

   virtual void BeginCycle() throw( SError );
   virtual void EndCycle() throw( SError );

   virtual void BeginInputData( const SInputData& ) throw( SError );
   virtual void EndInputData  ( const SInputData& ) throw( SError );

   virtual void BeginInputFile( const SInputData& ) throw( SError );

   virtual void ExecuteEvent( const SInputData&, Double_t weight ) throw( SError );

private:
   std::string m_RecoTreeName;

   enum ValHistsType { GENERAL, ELECTRON, MUON, JETS };
   void FillValidationHists( ValHistsType, const TString& status );

   //
   // Example properties
   //
   std::string m_stringVariable;
   int         m_intVariable;
   double      m_doubleVariable;
   std::vector< int >         m_intVecVariable;
   std::vector< double >      m_doubleVecVariable;
   std::vector< std::string > m_stringVecVariable;

   //
   // The input variables
   //
   Int_t                m_El_N;
   std::vector<double>* m_El_p_T;
   std::vector<double>* m_El_eta;
   std::vector<double>* m_El_phi;
   std::vector<double>* m_El_E;

   //
   // The output variables
   //
   int                      m_o_example_variable;
   std::vector< double >    m_o_El_p_T;
   std::vector< SParticle > m_o_El;

   ClassDef( FirstCycle , 0 );

}; // class FirstCycle

#endif // SFRAME_USER_FirstCycle_H
