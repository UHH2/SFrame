// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Cintex
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

/**
 * This function loads all the libraries needed for SFrameCintex (including
 * SFrameCintex itself) on the PROOF master and worker nodes.
 */
int SETUP() {

   if( gSystem->Load( "libCintex" ) == -1 ) return -1;
   if( gSystem->Load( "libSFrameCintex" ) == -1 ) return -1;

   return 0;
}
