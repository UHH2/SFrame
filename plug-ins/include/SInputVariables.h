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

#ifndef SFRAME_PLUGINS_SInputVariables_H
#define SFRAME_PLUGINS_SInputVariables_H

// SFrame include(s):
#include "core/include/SError.h"

/**
 *  @short  Base class for classes holding input variables
 *
 *          This class can be used to serve as a base class for classes whose
 *          only purpose is to collect input variables. By collecting input
 *          variables into objects, it's quite convenient to pass them around
 *          between objects that process their information.
 *
 * @version $Revision$
 */
template< class ParentType >
class SInputVariables {

public:
   /// Constructor that specifies the parent cycle
   SInputVariables( ParentType* parent );

protected:
   /// Connect an input variable
   template< typename T >
   bool ConnectVariable( const char* treeName, const char* branchName,
                         T& variable ) throw ( SError );

private:
   ParentType* m_parent; ///< Pointer to the parent cycle

}; // class SInputVariables

// Include the template implementation:
#ifndef __CINT__
#include "SInputVariables.icc"
#endif // __CINT__

#endif // SFRAME_PLUGINS_SInputVariables_H
