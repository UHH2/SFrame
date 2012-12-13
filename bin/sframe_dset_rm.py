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
import optparse
import ROOT

def main():

    descr = "This script can remove a specified dataset from a PROOF cluster"
    vers  = "$Revision$"
    parser = optparse.OptionParser( description = descr, version = vers,
                                    usage = "%prog [options]" )
    parser.add_option( "-s", "--server", dest="server",
                       action="store", type="string", default="localhost",
                       help="The PROOF server to investigate" )
    parser.add_option( "-d", "--dset", dest="dset",
                       action="store", type="string", default="",
                       help="The name of the dataset to create" )

    ( options, unrec ) = parser.parse_args()

    if len( unrec ):
        print "WARNING:"
        print "WARNING: Didn't recognise the following option(s): " + unrec
        print "WARNING:"
        pass

    if options.dset == "":
        print "ERROR:"
        print "ERROR: You have to specify a data set!"
        print "ERROR:"
        return 255

    # Switch ROOT to batch mode:
    ROOT.gROOT.SetBatch()

    print "Opening connection to PROOF server: " + options.server
    proof = ROOT.TProof.Open( options.server, "masteronly" )
    if ( not proof ) or ( not proof.IsValid() ):
        print "ERROR:"
        print "ERROR: Coulnd't connect to PROOF server: " + options.server
        print "ERROR:"
        return 255

    print "Removing data set: " + options.dset
    # Unfortunately the following check doesn't seem to work. The script always
    # returns with a success even for non-existent datasets...
    if proof.RemoveDataSet( options.dset ) != 0 :
        print "ERROR:"
        print "ERROR: Something went wrong while removing dataset:"
        print "ERROR:    " + options.dset
        print "ERROR:"
        return 255

    return 0

#
# Execute the main() function, when running the script directly:
#
if __name__ == "__main__":
    main()
