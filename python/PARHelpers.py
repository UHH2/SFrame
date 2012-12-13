# $Id$
##########################################################################
# Project: SFrame - ROOT-based analysis framework for ATLAS              #
#                                                                        #
# author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester    #
# author David Berge      <David.Berge@cern.ch>          - CERN          #
# author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg       #
# author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen  #
#                                                                        #
##########################################################################

## @package PARHelpers
#    @short Functions for creating a PAR file from an SFrame package
#
# This package collects the functions used by sframe_parMaker.py
# to create PAR packages out of SFrame packages during compilation.
# It is possible to use the functions directly, by just doing:
#
# <code>
#  >>> import PARHelpers
# </code>

# Import the needed module(s):
import os.path
import os
import re
import shutil

## PAR package creator function
#
# This function creates PAR packages based on a number of parameters.
#
# @param srcdir Base directory from which the PAR package is made
# @param makefile Name of the makefile in the package
# @param include Name of the include directory in the package
# @param src Name of the source file directory in the package
# @param proofdir Name of the directory holding the special PROOF files
# @param output Name of the output file
# @param verbose <code>True</code> if verbose printout is requested
def PARMaker( srcdir, makefile, include, src, proofdir, output, verbose ):

    # Tell the user what we're doing:
    if verbose:
        print "  >>"
        print "  >> Running PARHelpers.PARMaker"
        print "  >>"
        pass

    # Split the output file name into constituents:
    ( outputdir, outputfile ) = os.path.split( output )
    ( outputbase, outputext ) = os.path.splitext( outputfile )
    if verbose:
        print "  >> outputdir  = " + outputdir
        print "  >> outputbase = " + outputbase
        print "  >> outputext  = " + outputext
        pass

    # Check that the output file name has the correct extension:
    if( outputext != ".par" ):
        print "ERROR: The output file's extension must be \".par\""
        return

    # Create an appropriate temporary directory for the package creation:
    import tempfile
    tempdir = tempfile.mkdtemp()
    if verbose:
        print "  >> tempdir = " + tempdir
        pass

    # Create the temporary directories for the package:
    os.system( "rm -rf " + tempdir + "/" + outputbase )
    os.mkdir( tempdir + "/" + outputbase )
    os.mkdir( tempdir + "/" + outputbase + "/" + include )
    os.mkdir( tempdir + "/" + outputbase + "/" + src )
    os.mkdir( tempdir + "/" + outputbase + "/PROOF-INF" )

    # Get the list of header files to be included in the package:
    headers = os.listdir( srcdir + include )
    headers_filt = [ file for file in headers if
                    ( re.search( "\.h$", file ) or
                      re.search( "\.icc$", file ) ) ]
    if verbose:
        print "  >> headers = " + ", ".join( headers_filt )
        pass

    # Get the list of source files to be included in the package:
    sources = os.listdir( srcdir + src )
    sources_filt = [ file for file in sources if
                    ( ( re.search( "\.cxx$", file ) or re.search( "\.h$", file ) ) and not
                      ( re.search( "\_Dict\.cxx$", file ) or re.search( "\_Dict\.h$", file ) ) ) ]
    if verbose:
        print "  >> sources = " + ", ".join( sources_filt )
        pass

    # Copy the header and source files to the correct directories, and
    # then transform them in-situ:
    for header in headers_filt:
        shutil.copy( srcdir + "/" + include + "/" + header,
                     tempdir + "/" + outputbase + "/" + include )
        SourceTransform( tempdir + "/" + outputbase + "/" + include + "/" + header )
        pass
    for source in sources_filt:
        shutil.copy( srcdir + "/" + src + "/" + source,
                     tempdir + "/" + outputbase + "/" + src )
        SourceTransform( tempdir + "/" + outputbase + "/" + src + "/" + source )
        pass

    # Copy the package makefile to the correct directory:
    shutil.copy( srcdir + "/" + makefile,
                 tempdir + "/" + outputbase )
    ( makefiledir, makefilename ) = os.path.split( makefile )
    MakefileTransform( tempdir + "/" + outputbase + "/" + makefilename, verbose )

    # Create the Makefile.proof makefile fragment:
    MakefileProof( tempdir + "/" + outputbase + "/Makefile.proof", verbose )

    # Get the list of files in the proof directory:
    proof = os.listdir( srcdir + proofdir )
    proof_filt = [ file for file in proof if
                   ( not re.search( "~$", file ) and
                     os.path.isfile( srcdir + "/" + proofdir + "/" + file ) ) ]
    if verbose:
        print "  >> proof files = " + ", ".join( proof_filt )
        pass

    # Copy the proof files:
    for pfile in proof_filt:
        shutil.copy( srcdir + "/" + proofdir + "/" + pfile,
                     tempdir + "/" + outputbase + "/PROOF-INF" )
        pass

    # Create the PAR package:
    if verbose:
        print "  >> Now creating " + output
        pass
    os.system( "tar -C " + tempdir + " -czf " + output + " " + outputbase + "/" )

    # Remove the temporary directory:
    if verbose:
        print "  >> Now removing " + tempdir + "/" + outputbase
        pass
    os.system( "rm -rf " + tempdir + "/" + outputbase )
    # The temporary directory itself has to be removed as well:
    os.system( "rmdir " + tempdir )

    # Tell the user what we did:
    print "  Created PAR package: " + output

    return

## Transform the contents of the specified makefile for PROOF usage
#
# @param makefile_path Path to the makefile that should be transformed
# @param verbose <code>True</code> if verbose printout is requested
def MakefileTransform( makefile_path, verbose ):

    if verbose:
        print "  >>"
        print "  >> Running PARHelpers.MakefileTransform"
        print "  >>"

    # Read in the contents of the makefile:
    makefile = open( makefile_path, "r" )
    contents = makefile.read()
    makefile.close()

    # Do the necessary changes:
    new_contents = re.sub( "\$\(SFRAME_DIR\)\/Makefile\.common",
                           "Makefile.proof", contents )

    # Write out the modified contents in the same file:
    makefile = open( makefile_path, "w" )
    makefile.write( new_contents )
    makefile.close()

    return

## Create a makefile fragment with the directives for PROOF usage
#
# @param makefile_path Path to the makefile that should be created
# @param verbose <code>True</code> if verbose printout is requested
def MakefileProof( makefile_path, verbose ):

    if verbose:
        print "  >>"
        print "  >> Running PARHelpers.MakefileProof"
        print "  >>"

    makefile = open( makefile_path, "w" )
    makefile.write( MakefileProofContent )
    makefile.close()

    return

## Transform the contents of the specified source file for PROOF usage
#
# @param file_path Path to the source file that should be transformed
def SourceTransform( file_path ):

    # Read in the contents of the source file:
    file = open( file_path, "r" )
    contents = file.read()
    file.close()

    # Do the necessary changes:
    contents = re.sub( "core\/include",
                       "SFrameCore/include", contents )
    contents = re.sub( "plug\-ins\/include",
                       "SFramePlugIns/include", contents )

    # Write out the modified contents in the same file:
    file = open( file_path, "w" )
    file.write( contents )
    file.close()

    return

## Contents of the makefile for PROOF compilation
MakefileProofContent = """MAKEFLAGS = --no-print-directory -r -s

# Include the architecture definitions from the ROOT source:
ARCH_LOC_1 := $(wildcard $(shell root-config --prefix)/test/Makefile.arch)
ARCH_LOC_2 := $(wildcard $(shell root-config --prefix)/share/root/test/Makefile.arch)
ARCH_LOC_3 := $(wildcard $(shell root-config --prefix)/share/doc/root/test/Makefile.arch)
ARCH_LOC_4 := $(wildcard $(shell root-config --prefix)/etc/Makefile.arch)
ARCH_LOC_5 := $(wildcard $(shell root-config --prefix)/etc/root/Makefile.arch)
ifneq ($(strip $(ARCH_LOC_1)),)
  $(info Using $(ARCH_LOC_1))
  include $(ARCH_LOC_1)
else
  ifneq ($(strip $(ARCH_LOC_2)),)
    $(info Using $(ARCH_LOC_2))
    include $(ARCH_LOC_2)
  else
    ifneq ($(strip $(ARCH_LOC_3)),)
      $(info Using $(ARCH_LOC_3))
      include $(ARCH_LOC_3)
    else
      ifneq ($(strip $(ARCH_LOC_4)),)
        $(info Using $(ARCH_LOC_4))
        include $(ARCH_LOC_4)
      else
        ifneq ($(strip $(ARCH_LOC_5)),)
          $(info Using $(ARCH_LOC_5))
          include $(ARCH_LOC_5)
        else
          $(error Could not find Makefile.arch!)
        endif
      endif
    endif
  endif
endif

# Some compilation options
VPATH    += $(OBJDIR) $(SRCDIR)
INCLUDES += -I./ -I../
CXXFLAGS += -Wall -Wno-overloaded-virtual -Wno-unused $(USERCXXFLAGS)

# Set the locations of some files
DICTHEAD  = $(SRCDIR)/$(LIBRARY)_Dict.h
DICTFILE  = $(SRCDIR)/$(LIBRARY)_Dict.$(SrcSuf)
DICTOBJ   = $(OBJDIR)/$(LIBRARY)_Dict.$(ObjSuf)
DICTLDEF  = $(INCDIR)/$(LIBRARY)_LinkDef.h
SKIPCPPLIST = $(DICTFILE)
SKIPHLIST   = $(DICTHEAD) $(DICTLDEF)
SHLIBFILE = lib$(LIBRARY).$(DllSuf)
UNAME = $(shell uname)

# Set up the default targets
default: shlib

# List of all header and source files to build
HLIST   = $(filter-out $(SKIPHLIST),$(wildcard $(INCDIR)/*.h))
CPPLIST = $(filter-out $(SKIPCPPLIST),$(wildcard $(SRCDIR)/*.$(SrcSuf)))

# List of all object files to build
OLIST = $(patsubst %.$(SrcSuf),%.o,$(notdir $(CPPLIST)))

# Implicit rule to compile all sources
%.o : %.$(SrcSuf)
	@echo "Compiling $<"
	@mkdir -p $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $(OBJDIR)/$(notdir $@) $(INCLUDES)

# Rule to create the dictionary
$(DICTFILE): $(HLIST) $(DICTLDEF)
	@echo "Generating dictionary $@" 
	@$(shell root-config --exec-prefix)/bin/rootcint -f $(DICTFILE) -c -p $(INCLUDES) $^

# Rule to comile the dictionary
$(DICTOBJ): $(DICTFILE)
	@echo "Compiling $<"
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) -o $@ $<

-include $(foreach var,$(notdir $(CPPLIST:.$(SrcSuf)=.d)),$(DEPDIR)/$(var))

$(DEPDIR)/%.d: %.$(SrcSuf)
	@mkdir -p $(DEPDIR)
	if test -f $< ; then \
		echo "Making $(@F)"; \
		$(SHELL) -ec '$(CPP) -MM $(CXXFLAGS) $(INCLUDES) $< | sed '\''/Cstd\/rw/d'\'' > $@'; \
	fi

# Rule to combine objects into a unix shared library
$(SHLIBFILE): $(OLIST) $(DICTOBJ)
	@echo "Making shared library: $(SHLIBFILE)"
	@rm -f $(SHLIBFILE)
ifneq (,$(findstring macosx,$(ARCH)))
	@$(LD) $(LDFLAGS) $(USERLDFLAGS) -dynamiclib -single_module -undefined dynamic_lookup $(addprefix $(OBJDIR)/,$(OLIST)) $(DICTOBJ) -o $(SHLIBFILE)
else
	@$(LD) $(LDFLAGS) $(USERLDFLAGS) $(SOFLAGS) $(addprefix $(OBJDIR)/,$(OLIST)) $(DICTOBJ) -o $(SHLIBFILE)
endif

# Useful build targets
shlib: $(SHLIBFILE)

clean:
	rm -f $(DICTFILE) $(DICTHEAD)
	rm -f $(OBJDIR)/*.o
	rm -f $(SHLIBFILE)
	rm -f lib$(LIBRARY).so

distclean:
	rm -rf $(OBJDIR)
	rm -f *~
	rm -f $(INCDIR)/*~
	rm -f $(SRCDIR)/*~
	rm -f $(DICTFILE) $(DICTHEAD)
	rm -f $(DEPDIR)/*.d
	rm -f $(SHLIBFILE)
	rm -f lib$(LIBRARY).so

.PHONY : shlib default clean
"""
