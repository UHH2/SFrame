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

#ifndef SFRAME_USER_SecondCycle_H
#define SFRAME_USER_SecondCycle_H

// STL include(s):
#include <vector>
#include <string>

// Local include(s):
#include "core/include/SCycleBase.h"
#include "plug-ins/include/SParticle.h"

/**
 * Example cycle reading the ntuple created by FirstCycle and producing
 * some histograms.
 */
class SecondCycle : public SCycleBase {

public:
   SecondCycle();

   virtual void BeginCycle() throw( SError );
   virtual void EndCycle() throw( SError );

   virtual void BeginInputFile( const SInputData& ) throw( SError );

   virtual void BeginInputData( const SInputData& ) throw( SError );
   virtual void EndInputData  ( const SInputData& ) throw( SError );

   virtual void ExecuteEvent( const SInputData&, Double_t weight ) throw( SError );

private:
   std::string m_FirstCycleTreeName;

   //
   // The input variables:
   //
   int                       m_example_variable;
   std::vector< double >*    m_El_p_T;
   std::vector< SParticle >* m_El;

   ClassDef( SecondCycle , 0 );

}; // class SecondCycle

#endif // SFRAME_USER_SecondCycle_H
