#!/bin/env python
#
#

import sys
import os.path
import optparse

def main():

    parser = optparse.OptionParser( usage = "%prog [options] <input files>" )
    parser.add_option( "-o", "--output", dest="output",
                       action="store", type="string", default="TestID.xml",
                       help="Output XML file" )

    ( options, files ) = parser.parse_args()

    if not len( files ):
        print "You have to specify at least one input file!"
        print ""
        parser.print_help()
        return

    print "Making " + str( len( files ) ) + " InputData definitions into file: " + \
          options.output

    outfile = open( options.output, "w" )

    counter = 0
    for file in files:
        outfile.write( "<InputData Lumi=\"0.0\" NEventsMax=\"-1\" Type=\"Synthetic\"" \
                       " Version=\"Test" + str( counter ) + "\" >\n" )
        outfile.write( "   <In FileName=\"" + \
                       os.path.abspath( os.path.realpath( file ) ) + \
                       "\" Lumi=\"1.0\" />\n" )
        outfile.write( "   <InputTree Name=\"CollectionTree\" />\n" )
        outfile.write( "   <OutputTree Name=\"IDTest\" />\n" )
        outfile.write( "</InputData>\n" )
        counter = counter + 1

    outfile.close()

if __name__ == "__main__":
    main()
