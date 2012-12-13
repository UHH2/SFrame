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
                     help="Cross-section of the MC (for instance in pb)" )
  parser.add_option( "-d", "--data", dest="data",
                     action="store_true",
                     help="The input files are DATA" )
  parser.add_option( "-o", "--output",  dest="output",
                     action="store", type="string", default="data.xml",
                     help="Output XML file" )
  parser.add_option( "-t", "--tree", dest="tree",
                     action="store", type="string", default="CollectionTree",
                     help="TTree name in the files" )
  parser.add_option( "-p", "--prefix", dest="prefix",
                     action="store", type="string", default="",
                     help="Prefix to be put before the absolute path" )
  parser.add_option( "-r", "--real-filenames", dest="real_filenames",
                     action="store_true",
                     help="The file names should not be modified by the script" )

  ( options, files ) = parser.parse_args()

  # Check that at least one file is specified:
  if not len( files ):
    print "You should define at least one input file!"
    print ""
    parser.print_help()
    return 255

  # To avoid a PyROOT bug the script receives "-" as the first argument when being
  # executed on the GRID. This argument should just be ignored by the script...
  if "-" in files:
    files.remove( "-" )

  # On Panda the list of input files is given as a comma separated list.
  # While it is possible to transform that list into a space separated list
  # outside of this script, it is very conventient if this script can handle
  # such inputs as well.
  if ( len( files ) == 1 ) and files[ 0 ].count( ',' ):
    files = files[ 0 ].split( ',' )

  # Switch ROOT to batch mode:
  import ROOT
  ROOT.gROOT.SetBatch()

  # Call the actual function:
  import SFrameHelpers
  if options.data:
    print "The input files are DATA files"
    print ""
    return SFrameHelpers.CreateDataInput( files, options.output, options.tree, options.prefix,
                                          options.real_filenames )
  else:
    print "The input files are Monte Carlo files"
    print ""
    return SFrameHelpers.CreateInput( options.xsection, files, options.output, options.tree,
                                      options.prefix, options.real_filenames )

# Call the main function:
if __name__ == "__main__":
  sys.exit( main() )
