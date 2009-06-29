// Dear emacs, this is -*- c++ -*-
// $Id: CycleCreators.py,v 1.1 2008/02/14 17:59:20 krasznaa Exp $
#ifndef LeakCheck_H
#define LeakCheck_H

// STL include(s):
#include <vector>
#include <string>

// SFrame include(s):
#import "core/include/SCycleBase.h"

/**
 *   @short Put short description of class here
 *
 *          Put a longer description over here...
 *
 *  @author Put your name here
 * @version $Revision: 1.1 $
 */
class LeakCheck : public SCycleBase {

public:
   /// Default constructor
   LeakCheck();
   /// Default destructor
   ~LeakCheck();

   /// Function called at the beginning of the cycle
   virtual void BeginCycle() throw( SError );
   /// Function called at the end of the cycle
   virtual void EndCycle() throw( SError );

   /// Function called at the beginning of a new input data
   virtual void BeginInputData( const SInputData& ) throw( SError );
   /// Function called after finishing to process an input data
   virtual void EndInputData  ( const SInputData& ) throw( SError );

   /// Function called after opening each new input file
   virtual void BeginInputFile( const SInputData& ) throw( SError );

   /// Function called for every event
   virtual void ExecuteEvent( const SInputData&, Double_t ) throw( SError );

private:
   //
   // Put all your private variables here
   //
   Int_t m_TestInt;
   std::vector< double >* m_TestVec;

   Int_t m_oInt;
   std::vector< double > m_oVec;

   std::string m_treeName;

   // Macro adding the functions for dictionary generation
   ClassDef( LeakCheck, 0 );

}; // class LeakCheck

#endif // LeakCheck_H

