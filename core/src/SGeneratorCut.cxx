// $Id: SGeneratorCut.cxx,v 1.2 2007-11-22 18:19:26 krasznaa Exp $
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

// Local include(s):
#include "../include/SGeneratorCut.h"

/**
 * Not much to say about the constructor. It just initialises the member
 * variables based on the parameters provided to it.
 *
 * @param treename Name of the tree holding the variables that we cut on
 * @param formula  The formula describing the cut
 */
SGeneratorCut::SGeneratorCut( const TString& treename, const TString& formula )
   : m_tree( treename ), m_formula( formula ) {

}

/**
 * Another one of the "I don't do anything" destructors.
 */
SGeneratorCut::~SGeneratorCut() {

}

/**
 * It is only necessary for some technical affairs.
 */
SGeneratorCut& SGeneratorCut::operator= ( const SGeneratorCut& parent ) {

  this->m_tree = parent.m_tree;
  this->m_formula = parent.m_formula;

  return *this;

}

/**
 * The equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 == inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SGeneratorCut::operator== ( const SGeneratorCut& rh ) const {

  if( ( this->m_tree == rh.m_tree ) &&
      ( this->m_formula == rh.m_formula ) ) {
    return kTRUE;
  } else {
    return kFALSE;
  }

}

/**
 * The non-equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 != inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SGeneratorCut::operator!= ( const SGeneratorCut& rh ) const {

  return ( ! ( *this == rh ) );

}
