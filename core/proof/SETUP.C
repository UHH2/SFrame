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

/**
 * This function loads all the libraries needed for SFrameCore (including
 * SFrameCore itself) on the PROOF master and worker nodes.
 */
int SETUP() {

   if( gSystem->Load( "libTree" ) == -1 ) return -1;
   if( gSystem->Load( "libHist" ) == -1 ) return -1;
   if( gSystem->Load( "libXMLParser" ) == -1 ) return -1;
   if( gSystem->Load( "libProof" ) == -1 ) return -1;
   if( gSystem->Load( "libProofPlayer" ) == -1 ) return -1;
   if( gSystem->Load( "libSFrameCore" ) == -1 ) return -1;

   return 0;
}
