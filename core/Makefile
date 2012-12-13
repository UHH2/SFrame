 # $Id$
 ###########################################################################
 # @Project: SFrame - ROOT-based analysis framework for ATLAS              #
 # @Package: Core                                                          #
 #                                                                         #
 # @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester    #
 # @author David Berge      <David.Berge@cern.ch>          - CERN          #
 # @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg       #
 # @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen  #
 #                                                                         #
 # Makefile compiling the SFrameCore library.                              #
 #                                                                         #
 ###########################################################################

# Package information
LIBRARY = SFrameCore
OBJDIR  = obj
DEPDIR  = $(OBJDIR)/dep
SRCDIR  = src
INCDIR  = include

# Overwrite the default rule defined in Makefile.common
coredefault: default $(SFRAME_BIN_PATH)/sframe_main

# Include the library compilation rules
include $(SFRAME_DIR)/Makefile.common

#
# Rules for compiling the executable
#
# Reminder: $(ROOTLIBS) comes from Makefile.arch which is picked up from the ROOT
# sources by Makefile.common...
#
$(SFRAME_BIN_PATH)/sframe_main: sframe_main.o $(SHLIBFILE)
	@echo "Linking " $@
	@$(LD) $(LDFLAGS) $(OBJDIR)/sframe_main.o -L$(SFRAME_LIB_PATH) -lSFrameCore \
		$(ROOTLIBS) -lTreePlayer -lXMLParser -lPyROOT -lProof -lProofPlayer -lutil -lpcre -o $@

sframe_main.o: app/sframe_main.cxx include/SCycleController.h \
               include/SError.h include/SLogger.h
	@echo "Compiling $<"
	@mkdir -p $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $(OBJDIR)/$(notdir $@) $(INCLUDES)
