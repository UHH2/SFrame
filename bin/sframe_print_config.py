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
#
# This script can be used to easily print the configuration that was used
# to create an SFrame output file. This is useful to keep track of how files
# were created.
#

# We need ROOT for this:
import ROOT

##
# The C(++) style main function
#
# @returns <code>0</code> if everything went fine, something else otherwise
def main():

    # Access the command line argument(s):
    import sys
    if len( sys.argv ) != 2:
        print( "ERROR: The script expects to receive a single file name" )
        return 255

    # Print some initialization message:
    fileName = sys.argv[ 1 ]
    print( "Configuration stored in file: %s" % fileName )
    print( "" )

    # Disable the ROOT warnings:
    ROOT.gErrorIgnoreLevel = ROOT.kError

    # Open the file:
    ifile = ROOT.TFile.Open( fileName, "READ" )
    if ( not ifile ) or ifile.IsZombie():
        print( "ERROR: Couldn't open file '%s'" % fileName )
        return 255

    # Access the cycle configuration:
    config = ifile.Get( "SFrame/CycleConfiguration" )
    if not config:
        print( "ERROR: The specified file doesn't hold SFrame configuration "
               "metadata" )
        return 255

    # Print the configuration:
    print( "%s" % config.GetString().Data() )

    # Return gracefully:
    return 0

# Execute the main function:
if __name__ == "__main__":
    import sys
    sys.exit( main() )
