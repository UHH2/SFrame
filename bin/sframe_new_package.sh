# $Id$
###########################################################################
# @Project: SFrame - ROOT-based analysis framework for ATLAS              #
#                                                                         #
# @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester    #
# @author David Berge      <David.Berge@cern.ch>          - CERN          #
# @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg       #
# @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen  #
#                                                                         #
# Top level Makefile for compiling all the SFrame code                    #
#                                                                         #
###########################################################################

#
# Pick up the command line parameter for the package name:
#
if (( $# != 1 )); then 
  echo "Provide the name of the package"
  exit; 
fi

NAME=$1
LIBNAME=$NAME

#
# Create the directory structure:
#
directories="config include proof src"
files="ChangeLog Makefile include/${LIBNAME}_LinkDef.h proof/BUILD.sh proof/SETUP.C"

mkdir $NAME
for d in $directories; do mkdir $NAME/$d; done
for f in $files; do touch $NAME/$f; done

#
# Create the Makefile:
#
cat << EOT > $NAME/Makefile
# Package information
LIBRARY = $LIBNAME
OBJDIR  = obj
DEPDIR  = \$(OBJDIR)/dep
SRCDIR  = src
INCDIR  = include

# Include the generic compilation rules
include \$(SFRAME_DIR)/Makefile.common
EOT

#
# Create the LinkDef header file:
#
cat << EOT > $NAME/include/${LIBNAME}_LinkDef.h
// Dear emacs, this is -*- c++ -*-
// \$Id\$
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;

// Add the declarations of your cycles, and any other classes for which you
// want to generate a dictionary, here. The usual format is:
//
// #pragma link C++ class MySuperClass+;

#endif // __CINT__
EOT

#
# Create the "PAR files":
#
cat << EOT > $NAME/proof/SETUP.C
// \$Id\$

int SETUP() {

   /// Add all the additional libraries here that this package
   /// depends on. (With the same command that loads this package's library...)
   if( gSystem->Load( "lib$LIBNAME" ) == -1 ) return -1;

   return 0;
}
EOT

cat << EOT > $NAME/proof/BUILD.sh
# \$Id\$

if [ "\$1" = "clean" ]; then
    make distclean
    exit 0
fi

if [ "x\$ROOTPROOFLITE" != "x" ]; then
    echo "Running on PROOF-Lite, skipping build"
    exit 0
fi

make default
EOT
chmod 755 $NAME/proof/BUILD.sh

#
# End with some printout:
#
echo "Created package: $NAME"
