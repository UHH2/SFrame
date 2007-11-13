// Dear emacs, this is -*- c++ -*-
// $Id: SGeneratorCut.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SGeneratorCut_H
#define SFRAME_CORE_SGeneratorCut_H

// ROOT include(s):
#include "TObject.h"
#include "TString.h"

/**
 * Class defining ensamble of cuts applied at MC-generation level.
 * Used for normalising MC samples with different generator cuts to
 * each other.
 */
class SGeneratorCut : public TObject {

public:
   SGeneratorCut( TString treename, TString formula )
      : m_tree( treename ), m_formula ( formula ) {}

	~SGeneratorCut() {}

   TString GetTreeName() const { return m_tree; }
   TString GetFormula()  const { return m_formula; }

   SGeneratorCut& operator=  ( const SGeneratorCut& parent );
   Bool_t         operator== ( const SGeneratorCut& rh ) const;
   Bool_t         operator!= ( const SGeneratorCut& rh ) const;

private:
   TString m_tree;
   TString m_formula;

}; // class SGeneratorCut

#endif // SFRAME_CORE_SGeneratorCut_H
