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
import os.path
import ROOT

def main():

    descr = "This script can create data sets from existing files"
    vers  = "$Revision$"
    parser = optparse.OptionParser( description = descr, version = vers,
                                    usage = "%prog [options] <files>" )
    parser.add_option( "-s", "--server", dest="server",
                       action="store", type="string", default="localhost",
                       help="The PROOF server to use" )
    parser.add_option( "-d", "--dset", dest="dset",
                       action="store", type="string", default="",
                       help="The name of the dataset to create" )
    parser.add_option( "-p", "--prefix", dest="prefix",
                       action="store", type="string", default="",
                       help="Prefix to be put before the file names" )

    ( options, files ) = parser.parse_args()

    if not len( files ):
        print "ERROR:"
        print "ERROR: You have to define at least one file for the dataset!"
        print "ERROR:"
        return 255

    if options.dset == "":
        print "ERROR:"
        print "ERROR: You have to specify the dataset name!"
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

    filecoll = ROOT.TFileCollection( "dsetcoll", "File collection for making a data set" )
    for file in files:
        filename = options.prefix + os.path.abspath( os.path.realpath( file ) )
        print "Adding to the dataset: " + filename
        fileinfo = ROOT.TFileInfo( filename )
        filecoll.Add( fileinfo )

    if not proof.RegisterDataSet( options.dset, filecoll ):
        print "ERROR:"
        print "ERROR: Couldn't register data set with name \"" + options.dset + "\""
        print "ERROR:"
        return 255
    else:
        print "Data set registered with name \"" + options.dset + "\""

    print "Verifying data set..."
    if proof.VerifyDataSet( options.dset ):
        print "WARNING:"
        print "WARNING: Some files are physically missing from the new dataset!"
        print "WARNING:"

    return 0

#
# Execute the main() function, when running the script directly:
#
if __name__ == "__main__":
    sys.exit( main() )
