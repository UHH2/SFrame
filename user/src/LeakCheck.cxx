// $Id: CycleCreators.py,v 1.1 2008/02/14 17:59:20 krasznaa Exp $

// Local include(s):
#include "../include/LeakCheck.h"

ClassImp( LeakCheck );

LeakCheck::LeakCheck()
   : SCycleBase() {

   SetLogName( this->GetName() );

   DeclareProperty( "TestTreeName", m_treeName = "SFrameTree" );

}

LeakCheck::~LeakCheck() {

}

void LeakCheck::BeginCycle() throw( SError ) {

   m_logger << INFO << "Beginning cycle" << SLogger::endmsg;

   return;

}

void LeakCheck::EndCycle() throw( SError ) {

   m_logger << INFO << "Finished cycle" << SLogger::endmsg;

   return;

}

void LeakCheck::BeginInputData( const SInputData& ) throw( SError ) {

   Book( TH1D( "TestIntHist", "Test histogram with Int_t-s",
               10, 0.0, 10.0 ) );
   Book( TH1D( "TestVecHist", "Test histogram with double-s",
               50, -40.0, 40.0 ) );

   DeclareVariable( m_oInt, "OutputInt" );
   DeclareVariable( m_oVec, "OutputVec" );

   return;

}

void LeakCheck::EndInputData( const SInputData& ) throw( SError ) {

   return;

}

void LeakCheck::BeginInputFile( const SInputData& ) throw( SError ) {

   ConnectVariable( m_treeName.c_str(), "TestInt", m_TestInt );
   ConnectVariable( m_treeName.c_str(), "TestVec", m_TestVec );

   return;

}

void LeakCheck::ExecuteEvent( const SInputData&, Double_t ) throw( SError ) {

   Hist( "TestIntHist" )->Fill( m_TestInt );
   m_oInt = m_TestInt * 2;

   m_oVec.clear();
   for( std::vector< double >::const_iterator it = m_TestVec->begin();
        it != m_TestVec->end(); ++it ) {
      Hist( "TestVecHist" )->Fill( *it );
      m_oVec.push_back( *it / 2.0 );
   }

   return;

}

