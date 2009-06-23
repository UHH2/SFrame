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
# It's a little script creating the <In ... /> lines for an analysis cycle
# from the files specified in it's input. It calculates the correct luminosity
# of each file from the supplied cross section, and the number of events in
# the file. The script creates an output XML file with the <In> nodes, that the
# user can either copy-paste into his/her configuration XML, or include the file
# as an external element. (ROOT >=5.17 required)
#

# Import base module(s):
import sys
import os.path
import optparse

## Main function of this script
#
# The executable code of the script is put into this function, much like
# it is done in the case of C++ programs. The main function is then called
# from outside using the usual method. (Which I've seen in various
# places...)
def main():
  # Print some welcome message before doing anything else:
  print ">>"
  print ">> %s : <In> node creator for SFrame analysis jobs" % \
        os.path.basename( sys.argv[ 0 ] )
  print ">>"
  print ""

  # Parse the command line parameters:
  parser = optparse.OptionParser( usage="%prog [options] <input files>" )
  parser.add_option( "-x", "--xsection", dest="xsection",
                     action="store", type="float", default=1.0,
                     help="Cross-section of the MC data in pb-1" )
  parser.add_option( "-d", "--data", dest="data",
                     action="store_true",
                     help="The input files are DATA" )
  parser.add_option( "-o", "--output",  dest="output",
                     action="store", type="string", default="data.xml",
                     help="Output XML file" )

  ( options, files ) = parser.parse_args()

  if not len( files ):
    print "You should define at least one input file!"
    print ""
    parser.print_help()
    return

  # Call the actual function:
  import SFrameHelpers
  if options.data:
    print "The input files are DATA files"
    print ""
    SFrameHelpers.CreateDataInput( files, options.output )
  else:
    print "The input files are Monte Carlo files"
    print ""
    SFrameHelpers.CreateInput( options.xsection, files, options.output )

# Call the main function:
if __name__ == "__main__":
  main()
