/**
   @mainpage SFrame core library documentation

   @author Stefan Ask <Stefan.Ask@cern.ch> - Manchester
   @author David Berge <David.Berge@cern.ch> - CERN
   @author Johannes Haller <Johannes.Haller@cern.ch> - Hamburg
   @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen

   @section SFrameCoreInto Introduction

   SFrame is a general analysis framework developed for use in HEP
   analyses. It builds on the idea, that analyses in HEP are usually
   broken into "cycles". These cycles usually do things like event
   selection, data thinning, calculation of new data, etc.

   SFrame provides a framework for running analysis cycles that read
   ROOT TTrees as input and write other TTrees and histograms as
   output.

   Further information about SFrame is available under:
   http://sourceforge.net/apps/mediawiki/sframe/

   @section SFrameCoreOverview Class overview

   The main classes defined in the core library are the following:

    - SCycleController : This object is used to control the execution of
      an analysis cycle.
    - SCycleBase : Base class for the analysis cycles, providing a lot of
      features. This should be the most useful part of this documentation...
    - SInputData : Class describing a set of input files for the analysis.
    - SLogger : A universal message logging class. This should be used by the
      users for printing messages in the terminal.

   @section SFrameCoreAvail Availability

   If you're reading this documentation, you probably already know how to
   obtain the SFrame sources. The commands to check out the code and generate
   this documentation are (in BASH on lxplus):

   <code>
     > svn co https://sframe.svn.sourceforge.net/svnroot/sframe/SFrame/trunk SFrame<br/>
     > cd SFrame<br/>
     > doxygen
   </code>

*/
