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

#ifndef SFRAME_CORE_SParLocator_H
#define SFRAME_CORE_SParLocator_H

// STL include(s):
#include <list>

// ROOT include(s):
#include <TString.h>

// Local include(s):
#include "SLogger.h"

/**
 *   @short Helper class for finding the full path name of PAR packages
 *
 *          Unfortunately ROOT does not provide any facilities for finding a
 *          given (PAR) file in a list of directories. (Like in $PATH.)
 *          So I created this little class for constructing the full path name
 *          for the PAR packages. The class reads the list of directories to
 *          be searched from the PAR_PATH environment variable, which is
 *          usually set in SFrame/setup.[c]sh.
 *
 *          This is another part of the SFrame code that probably doesn't work
 *          on Windows, as I had to use standard POSIX code for locating the
 *          package files. Somehow I couldn't find any ROOT functions for
 *          listing all the files in a given directory. Very strange...
 *
 * @version $Revision$
 */
class SParLocator {

public:
   /// Locate a package file
   static TString Locate( const TString& parName );

private:
   /// Read the PAR_PATH environment variable
   static void ReadParDirs();

   static std::list< TString> m_parDirs; ///< List of directories to be searched
   static SLogger             m_logger;  ///< Logger object

}; // class SParLocator

#endif // SFRAME_CORE_SParLocator_H
