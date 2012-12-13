# $Id$
#***************************************************************************
#* @Project: SFrame - ROOT-based analysis framework for ATLAS
#* @Package: Plug-ins
#*
#* @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
#* @author David Berge      <David.Berge@cern.ch>          - CERN
#* @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
#* @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
#*
#***************************************************************************

#
# This script builds the SFramePlugIns package on the PROOF worker and master
# nodes.
#

if [ "$1" = "clean" ]; then
    make distclean
    exit 0
fi

if [ "x$ROOTPROOFLITE" != "x" ]; then
    echo "Running on PROOF-Lite, skipping build"
    exit 0
fi

make default
