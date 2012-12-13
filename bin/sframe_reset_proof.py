#!/usr/bin/env python
# $Id$
#***************************************************************************
#* @Project: SFrame - ROOT-based analysis framework for ATLAS
#* @Package: Core
#*
#* @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
#* @author David Berge      <David.Berge@cern.ch>          - CERN
#* @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
#* @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
#*
#***************************************************************************

# Import the needed modules:
import sys
import optparse
import ROOT

def main():

    descr = "This script can be used to 'reset' a PROOF server. This can be " \
            "useful when a PROOF job has to be stopped. This can usually not " \
            "be done using SFrame directly."
    vers  = "$Revision$"
    parser = optparse.OptionParser( description = descr, version = vers,
                                   usage = "%prog [options]" )
    parser.add_option( "-s", "--server", dest="server",
                       action="store", type="string", default="localhost",
                       help="The PROOF server to use" )
    parser.add_option( "-r", "--hard-reset", dest="hard_reset",
                       action="store_true", default=False,
                       help="Perform a 'hard reset' on the PROOF server" )

    ( options, unrec ) = parser.parse_args()

    if len( unrec ):
        print "WARNING:"
        print "WARNING: Didn't recognise the following option(s): " + unrec
        print "WARNING:"
        pass

    # Switch ROOT to batch mode:
    ROOT.gROOT.SetBatch()

    # Send the reset signal:
    ROOT.TProof.Reset( options.server, options.hard_reset );

    return 0

#
# Execute the main() function, when running the script directly:
#
if __name__ == "__main__":
    sys.exit( main() )
