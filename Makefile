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

all: core plug-ins cintex user

core::
	+(cd core; make)

plug-ins::
	+(cd plug-ins; make)

cintex::
	+(cd cintex; make)

user::
	+(cd user; make)

clean::
	(cd core; make clean)
	(cd plug-ins; make clean)
	(cd cintex; make clean)
	(cd user; make clean)

distclean::
	(cd core; make distclean)
	(cd plug-ins; make distclean)
	(cd cintex; make distclean)
	(cd user; make distclean)
