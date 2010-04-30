#!/usr/bin/env python
# $Id$
#***************************************************************************
#* @Project: SFrame - ROOT-based analysis framework for ATLAS
#* @Package: Core
#*
#* @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
#* @author David Berge      <David.Berge@cern.ch>          - CERN
#* @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
#* @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
#*
#***************************************************************************

# Import the needed modules:
import optparse
import ROOT

def main():

    descr = "This script can clear the PAR packages off of a PROOF server." + \
            "This might be necessary to do when the analysis code changes" + \
            "too much for ROOT to handle it properly. (E.g. removing files" + \
            "from a package)"
    vers  = "$Revision$"
    parser = optparse.OptionParser( description = descr, version = vers,
                                    usage = "%prog [options]" )
    parser.add_option( "-s", "--server", dest="server",
                       action="store", type="string", default="localhost",
                       help="The PROOF server to use" )

    ( options, unrec ) = parser.parse_args()

    if len( unrec ):
        print "WARNING:"
        print "WARNING: Didn't recognise the following option(s): " + unrec
        print "WARNING:"

    print "Opening connection to PROOF server: " + options.server
    proof = ROOT.TProof.Open( options.server )

    if proof.ClearPackages() != 0:
        print "ERROR: There was a problem clearing the packages from server " + \
              options.server
    else:
        print "PAR packages cleared from server " + options.server

    return

#
# Execute the main() function, when running the script directly:
#
if __name__ == "__main__":
    main()
