 # $Id: Makefile,v 1.2.2.1 2008-12-01 14:52:55 krasznaa Exp $
 ###########################################################################
 # @Project: SFrame - ROOT-based analysis framework for ATLAS              #
 #                                                                         #
 # @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester    #
 # @author David Berge      <David.Berge@cern.ch>          - CERN          #
 # @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg       #
 # @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen #
 #                                                                         #
 # Top level Makefile for compiling all the SFrame code                    #
 #                                                                         #
 ###########################################################################

all: core plug-ins user

core::
	(cd core; make)

plug-ins::
	(cd plug-ins; make)

user::
	(cd user; make)

clean::
	(cd core; make clean)
	(cd plug-ins; make clean)
	(cd user; make clean)

distclean::
	(cd core; make distclean)
	(cd plug-ins; make distclean)
	(cd user; make distclean)
