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
# Script creating a PAR package from the contents of a directory.
# (As long as the directory follows the SFrame layout...)

# Import base module(s):
import sys
import os.path
import optparse

def main():
    print "  -- Proof ARchive creator for SFrame --"

    parser = optparse.OptionParser( usage="%prog [options]" )
    parser.add_option( "-s", "--scrdir", dest="srcdir",
                       action="store", type="string", default="./",
                       help="Directory that is to be converted" )
    parser.add_option( "-o", "--output",  dest="output",
                       action="store", type="string", default="Test.par",
                       help="Output PAR file" )
    parser.add_option( "-m", "--makefile", dest="makefile",
                       action="store", type="string", default="Makefile",
                       help="Name of the makefile in the package" )
    parser.add_option( "-i", "--include", dest="include",
                       action="store", type="string", default="include",
                       help="Directory holding the header files" )
    parser.add_option( "-c", "--src", dest="src",
                       action="store", type="string", default="src",
                       help="Directory holding the source files" )
    parser.add_option( "-p", "--proofdir", dest="proofdir",
                       action="store", type="string", default="proof",
                       help="Directory holding the special files for PROOF" )
    parser.add_option( "-v", "--verbose", dest="verbose",
                       action="store_true",
                       help="Print verbose information about package creation" )

    ( options, garbage ) = parser.parse_args()
    if len( garbage ):
        print "The following options were not recognised:"
        print ""
        print "   " + garbage
        parser.print_help();
        return

    if options.verbose:
        print "  >> srcdir   = " + options.srcdir
        print "  >> output   = " + options.output
        print "  >> makefile = " + options.makefile
        print "  >> include  = " + options.include
        print "  >> src      = " + options.src
        print "  >> proofdir = " + options.proofdir

    import PARHelpers
    PARHelpers.PARMaker( options.srcdir, options.makefile, options.include,
                         options.src, options.proofdir, options.output,
                         options.verbose )

    return

# Call the main function:
if __name__ == "__main__":
  main()
