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

#ifndef SFRAME_PLUGINS_SOutputVariables_H
#define SFRAME_PLUGINS_SOutputVariables_H

// SFrame include(s):
#include "core/include/SError.h"

// Forward declaration(s):
class TBranch;

/**
 *  @short  Base class for classes holding output variables
 *
 *          This class can be used to serve as a base class for classes whose
 *          only purpose is to collect output variables. This can make it
 *          simpler to fill the variables of an output ntuple with multiple
 *          separate code pieces.
 *
 *          I should note however that when using SToolBase as the basis of the
 *          code pieces calculating the output variables, one can just fill the
 *          variables in the tools directly.
 *
 * @version $Revision$
 */
template< class ParentType >
class SOutputVariables {

public:
   /// Constructor that specifies the parent cycle
   SOutputVariables( ParentType* parent );

protected:
   /// Declare an output variable
   template< typename T >
   TBranch* DeclareVariable( T& obj, const char* name,
                             const char* treeName = 0 ) throw( SError );

private:
   ParentType* m_parent; ///< Pointer to the parent cycle

}; // class SInputVariables

// Include the template implementation:
#ifndef __CINT__
#include "SOutputVariables.icc"
#endif // __CINT__

#endif // SFRAME_PLUGINS_SOutputVariables_H
