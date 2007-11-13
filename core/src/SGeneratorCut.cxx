// $Id: SGeneratorCut.cxx,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// Local include(s):
#include "../include/SGeneratorCut.h"

SGeneratorCut& SGeneratorCut::operator= ( const SGeneratorCut& parent ) {

  this->m_tree = parent.m_tree;
  this->m_formula = parent.m_formula;

  return *this;

}

Bool_t SGeneratorCut::operator== ( const SGeneratorCut& rh ) const {

  if( ( this->m_tree == rh.m_tree ) &&
      ( this->m_formula == rh.m_formula ) ) {
    return kTRUE;
  } else {
    return kFALSE;
  }

}

Bool_t SGeneratorCut::operator!= ( const SGeneratorCut& rh ) const {

  return ( ! ( *this == rh ) );

}
