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
   parser.add_option( "-p", "--package", dest="package",
                      action="store", type="string", default="",
                      help="The package to be cleared" )

   ( options, unrec ) = parser.parse_args()

   if len( unrec ):
      print "WARNING:"
      print "WARNING: Didn't recognise the following option(s): " + unrec
      print "WARNING:"
      pass

   # Switch ROOT to batch mode:
   ROOT.gROOT.SetBatch()

   print "Opening connection to PROOF server: " + options.server
   proof = ROOT.TProof.Open( options.server )
   if ( not proof ) or ( not proof.IsValid() ):
      print "ERROR:"
      print "ERROR: Coulnd't connect to PROOF server: " + options.server
      print "ERROR:"
      return 255

   if options.package == "":
      if proof.ClearPackages() != 0:
         print "ERROR:"
         print "ERROR: There was a problem clearing the packages from server " + \
             options.server
         print "ERROR:"
         return 255
      else:
         print "PAR packages cleared from server " + options.server
         pass
   else:
      if proof.ClearPackage(options.package) != 0:
         print "ERROR:"
         print "ERROR: There was a problem clearing the package %s from server %s " % \
             (options.package, options.server)
         print "ERROR:"
         return 255
      else:
         print "PAR package %s cleared from server %s" % \
             (options.package, options.server)
         pass

   return 0

#
# Execute the main() function, when running the script directly:
#
if __name__ == "__main__":
    sys.exit( main() )
