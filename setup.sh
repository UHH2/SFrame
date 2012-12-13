#! /bin/sh
# $Id$
###########################################################################
# @Project: SFrame - ROOT-based analysis framework for ATLAS              #
#                                                                         #
# @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester    #
# @author David Berge      <David.Berge@cern.ch>          - CERN          #
# @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg       #
# @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen  #
#                                                                         #
# Script that has to be sourced before compiling/running SFrame.          #
#                                                                         #
###########################################################################

# Greet the user
echo "Setting up environment for compiling/running SFrame"

if [ $SFRAME_DIR ]; then
    echo SFRAME_DIR is already defined, use a clean shell
    return 1
fi

# speficy the SFRAME base directory, i.e. the directory in which this file lives
export SFRAME_DIR=${PWD}

# Modify to describe your directory structure. Default is to use the a structure where
# all directories are below the SFrame base directory specified above
export SFRAME_BIN_PATH=${SFRAME_DIR}/bin
export SFRAME_LIB_PATH=${SFRAME_DIR}/lib

# Check if bin/lib/include directories exist, if not create them
if [ ! -d ${SFRAME_BIN_PATH} ]; then
    echo Directory ${SFRAME_BIN_PATH} does not exist ... creating it
    mkdir ${SFRAME_BIN_PATH}
fi
if [ ! -d ${SFRAME_LIB_PATH} ]; then
    echo Directory ${SFRAME_LIB_PATH} does not exist ... creating it
    mkdir ${SFRAME_LIB_PATH}
fi

# The Makefiles depend only on the root-config script to use ROOT,
# so make sure that is available
if [[ `which root-config` == "" ]]; then
    echo "Error: ROOT environment doesn't seem to be configured!"
fi

if [[ `root-config --platform` == "macosx" ]]; then

    # With Fink ROOT installations, DYLD_LIBRARY_PATH doesn't have
    # to be defined for ROOT to work. So let's leave the test for it...
    export DYLD_LIBRARY_PATH=${SFRAME_LIB_PATH}:${DYLD_LIBRARY_PATH}

else

    if [ ! $LD_LIBRARY_PATH ]; then
        echo "Warning: so far you haven't setup your ROOT enviroment properly (no LD_LIBRARY_PATH): SFrame will not work"
    fi

    export LD_LIBRARY_PATH=${SFRAME_LIB_PATH}:${LD_LIBRARY_PATH}

fi

export PATH=${SFRAME_BIN_PATH}:${PATH}
export PYTHONPATH=${SFRAME_DIR}/python:${PYTHONPATH}

export PAR_PATH=./:${SFRAME_LIB_PATH}
