// Dear emacs, this is -*- c++ -*-
// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

#ifndef SFRAME_CORE_SGeneratorCut_H
#define SFRAME_CORE_SGeneratorCut_H

// ROOT include(s):
#include "TObject.h"
#include "TString.h"

/**
 *   @short Class describing a set of MC generator cuts
 *
 *          Class defining ensamble of cuts applied at MC-generation
 *          level. Used for normalising MC samples with different
 *          generator cuts to each other.
 *
 * @version $Revision$
 */
class SGeneratorCut : public TObject {

public:
   /// Constructor specifying a tree name and a formula
   SGeneratorCut( const TString& treename = "", const TString& formula = "" );

   /// Get the name of the tree
   /**
    * Cuts can be defined on variables that are available in one of
    * the input trees. (True particle p<sub>T</sub>, missing
    * E<sub>T</sub>, etc.) This property specifies the name of the
    * tree in the input file that holds the variables that we want
    * to cut on.
    */
   const TString& GetTreeName() const { return m_tree; }
   /// Get the formula of the generator cut
   /**
    * The cut formula follows the syntax accepted by TTreeFormula.
    * This means that the user can specify formulas like he would when
    * using TTree::Draw. For instance the following would be
    * acceptable:
    *
    * <code>
    *   "MissingEt>10000"<br/>
    *   "Mu_p_T[0]>20000"
    * </code>
    */
   const TString& GetFormula()  const { return m_formula; }

   /// Assignment operator
   SGeneratorCut& operator=  ( const SGeneratorCut& parent );
   /// Equality operator
   Bool_t         operator== ( const SGeneratorCut& rh ) const;
   /// Non-equality operator
   Bool_t         operator!= ( const SGeneratorCut& rh ) const;

private:
   TString m_tree;
   TString m_formula;

#ifndef DOXYGEN_IGNORE
   ClassDef( SGeneratorCut, 1 )
#endif // DOXYGEN_IGNORE

}; // class SGeneratorCut

#endif // SFRAME_CORE_SGeneratorCut_H
