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

## @package CycleCreators
#    @short Functions for creating a new analysis cycle torso
#
# This package collects the functions used by sframe_create_cycle.py
# to create the torso of a new analysis cycle. Apart from using
# sframe_create_cycle.py, the functions can be used in an interactive
# python session by executing:
#
# <code>
#  >>> import CycleCreators
# </code>

## @short Class creating analysis cycle templates
#
# This class can be used to create a template cycle inheriting from
# SCycleBase. It is quite smart actually. If you call CycleCreator.CreateCycle
# from inside an "SFrame package", it will find the right locations for the
# created files and extend an already existing LinkDef.h file with the
# line for the new cycle.
class CycleCreator:

    _headerFile = ""
    _sourceFile = ""

    def __init__( self ):
        self._headerFile = ""
        self._sourceFile = ""

    ## @short Template for cycle outside of a namespace
    #
    # This string is used by CreateHeader to create a header file that
    # holds a cycle which is not in a namespace.
    _header = """// Dear emacs, this is -*- c++ -*-
// $Id$
#ifndef %(class)-s_H
#define %(class)-s_H

// SFrame include(s):
#include \"core/include/SCycleBase.h\"

/**
 *   @short Put short description of class here
 *
 *          Put a longer description over here...
 *
 *  @author Put your name here
 * @version $Revision$
 */
class %(class)-s : public SCycleBase {

public:
   /// Default constructor
   %(class)-s();
   /// Default destructor
   ~%(class)-s();

   /// Function called at the beginning of the cycle
   virtual void BeginCycle() throw( SError );
   /// Function called at the end of the cycle
   virtual void EndCycle() throw( SError );

   /// Function called at the beginning of a new input data
   virtual void BeginInputData( const SInputData& ) throw( SError );
   /// Function called after finishing to process an input data
   virtual void EndInputData  ( const SInputData& ) throw( SError );

   /// Function called after opening each new input file
   virtual void BeginInputFile( const SInputData& ) throw( SError );

   /// Function called for every event
   virtual void ExecuteEvent( const SInputData&, Double_t ) throw( SError );

private:
   //
   // Put all your private variables here
   //

   // Macro adding the functions for dictionary generation
   ClassDef( %(class)-s, 0 );

}; // class %(class)-s

#endif // %(class)-s_H

"""

    ## @short Template for cycle in a namespace
    #
    # This string is used by CreateHeader to create a header file that
    # holds a cycle which is in a namespace.
    _headerNs = """// Dear emacs, this is -*- c++ -*-
// $Id$
#ifndef %(ns)-s_%(class)-s_H
#define %(ns)-s_%(class)-s_H

// SFrame include(s):
#include \"core/include/SCycleBase.h\"

namespace %(ns)-s {

   /**
    *   @short Put short description of class here
    *
    *          Put a longer description over here...
    *
    *  @author Put your name here
    * @version $Revision$
    */
   class %(class)-s : public SCycleBase {

   public:
      /// Default constructor
      %(class)-s();
      /// Default destructor
      ~%(class)-s();

      /// Function called at the beginning of the cycle
      virtual void BeginCycle() throw( SError );
      /// Function called at the end of the cycle
      virtual void EndCycle() throw( SError );

      /// Function called at the beginning of a new input data
      virtual void BeginInputData( const SInputData& ) throw( SError );
      /// Function called after finishing to process an input data
      virtual void EndInputData  ( const SInputData& ) throw( SError );

      /// Function called after opening each new input file
      virtual void BeginInputFile( const SInputData& ) throw( SError );

      /// Function called for every event
      virtual void ExecuteEvent( const SInputData&, Double_t ) throw( SError );

   private:
      //
      // Put all your private variables here
      //

      // Macro adding the functions for dictionary generation
      ClassDef( %(ns)-s::%(class)-s, 0 );

   }; // class %(class)-s

} // namespace %(ns)-s

#endif // %(ns)-s_%(class)-s_H

"""

    ## @short Function creating an analysis cycle header
    #
    # This function can be used to create the header file for a new analysis
    # cycle. It can correctly create the header file if the cycle name is
    # defined like "Ana::AnalysisCycle". In this case it creates a cycle
    # called "AnalysisCycle" that is in the C++ namespace "Ana". Multiple
    # namespaces such as "Ana::MyAna::AnalysisCycle" are not supported!
    #
    # @param cycleName Name of the analysis cycle. Can contain the namespace name.
    # @param fileName  Optional parameter with the output header file name
    def CreateHeader( self, cycleName, fileName = "" ):

        # Extract the namespace name if it has been specified:
        namespace = ""
        import re
        if re.search( "::", cycleName ):
            print "CreateHeader:: We're creating a cycle in a namespace"
            m = re.match( "(.*)::(.*)", cycleName )
            namespace = m.group( 1 )
            cycleName = m.group( 2 )
            print "CreateHeader:: Namespace name = " + namespace

        # Construct the file name if it has not been specified:
        if fileName == "":
            fileName = cycleName + ".h"

        # Some printouts:
        print "CreateHeader:: Cycle name     = " + cycleName
        print "CreateHeader:: File name      = " + fileName
        self._headerFile = fileName

        # Create a backup of an already existing header file:
        import os.path
        if os.path.exists( fileName ):
            print "CreateHeader:: File \"" + fileName + "\" already exists"
            print "CreateHeader:: Moving \"" + fileName + "\" to \"" + \
                  fileName + ".backup\""
            import shutil
            shutil.move( fileName, fileName + ".backup" )

        # Write the header file:
        output = open( fileName, "w" )
        if namespace == "":
            output.write( self._header % { 'class' : cycleName } )
        else:
            output.write( self._headerNs % { 'class' : cycleName,
                                             'ns'    : namespace } )

        return

    ## @short Template for cycle outside of a namespace
    #
    # This string is used by CreateSource to create a source file that
    # holds a cycle which is not in a namespace.
    _source = """// $Id$

// Local include(s):
#include \"%(dir)-s/%(class)-s.h\"

ClassImp( %(class)-s );

%(class)-s::%(class)-s()
   : SCycleBase() {

   SetLogName( GetName() );
}

%(class)-s::~%(class)-s() {

}

void %(class)-s::BeginCycle() throw( SError ) {

   return;

}

void %(class)-s::EndCycle() throw( SError ) {

   return;

}

void %(class)-s::BeginInputData( const SInputData& ) throw( SError ) {

   return;

}

void %(class)-s::EndInputData( const SInputData& ) throw( SError ) {

   return;

}

void %(class)-s::BeginInputFile( const SInputData& ) throw( SError ) {

   return;

}

void %(class)-s::ExecuteEvent( const SInputData&, Double_t ) throw( SError ) {

   return;

}

"""

    ## @short Template for cycle in a namespace
    #
    # This string is used by CreateSource to create a source file that
    # holds a cycle which is in a namespace.
    _sourceNs = """// $Id$

// Local include(s):
#include \"%(dir)-s/%(class)-s.h\"

ClassImp( %(ns)-s::%(class)-s );

namespace %(ns)-s {

   %(class)-s::%(class)-s()
      : SCycleBase() {

      SetLogName( GetName() );
   }

   %(class)-s::~%(class)-s() {

   }

   void %(class)-s::BeginCycle() throw( SError ) {

      return;

   }

   void %(class)-s::EndCycle() throw( SError ) {

      return;

   }

   void %(class)-s::BeginInputData( const SInputData& ) throw( SError ) {

      return;

   }

   void %(class)-s::EndInputData( const SInputData& ) throw( SError ) {

      return;

   }

   void %(class)-s::BeginInputFile( const SInputData& ) throw( SError ) {

      return;

   }

   void %(class)-s::ExecuteEvent( const SInputData&, Double_t ) throw( SError ) {

      return;

   }

} // namespace %(ns)-s

"""

    ## @short Function creating the analysis cycle source file
    #
    # This function creates the source file that works with the header created
    # by CreateHeader. It is important that CreateHeader is executed before
    # this function, as it depends on knowing where the header file is
    # physically. (To include it correctly in the source file.) It can
    # handle cycles in namespaces, just like CreateHeader. (The same
    # rules apply.)
    #
    # @param cycleName Name of the analysis cycle. Can contain the namespace name.
    # @param fileName  Optional parameter with the output source file name
    def CreateSource( self, cycleName, fileName = "" ):

        # Extract the namespace name if it has been specified:
        namespace = ""
        import re
        if re.search( "::", cycleName ):
            print "CreateSource:: We're creating a cycle in a namespace"
            m = re.match( "(.*)::(.*)", cycleName )
            namespace = m.group( 1 )
            cycleName = m.group( 2 )
            print "CreateSource:: Namespace name = " + namespace

        # Construct the file name if it has not been specified:
        if fileName == "":
            fileName = cycleName + ".cxx"

        # Some printouts:
        print "CreateSource:: Cycle name     = " + cycleName
        print "CreateSource:: File name      = " + fileName
        self._sourceFile = fileName

        # The following is a tricky part. Here I evaluate how the source file
        # will be able to include the previously created header file.
        # Probably a Python guru could've done it in a shorter way, but
        # at least it works.
        import os.path
        hdir = os.path.dirname( self._headerFile )
        sdir = os.path.dirname( self._sourceFile )
        prefix = os.path.commonprefix( [ self._headerFile, self._sourceFile ] )

        hdir = hdir.replace( prefix, "" )
        sdir = sdir.replace( prefix, "" )

        nup = sdir.count( "/" );
        nup = nup + 1
        dir = ""
        for i in range( 0, nup ):
            dir = dir.join( [ "../", hdir ] )

        # Create a backup of an already existing header file:
        if os.path.exists( fileName ):
            print "CreateHeader:: File \"" + fileName + "\" already exists"
            print "CreateHeader:: Moving \"" + fileName + "\" to \"" + \
                  fileName + ".backup\""
            import shutil
            shutil.move( fileName, fileName + ".backup" )

        # Write the source file:
        output = open( fileName, "w" )
        if namespace == "":
            output.write( self._source % { 'dir'   : dir,
                                           'class' : cycleName } )
        else:
            output.write( self._sourceNs % { 'dir'   : dir,
                                             'class' : cycleName,
                                             'ns'    : namespace } )

        return

    ## @short Function adding link definitions for rootcint
    #
    # Each new analysis cycle has to declare itself in a so called "LinkDef
    # file". This makes sure that rootcint knows that a dictionary should
    # be generated for this C++ class.
    #
    # This function is also quite smart. If the file name specified does
    # not yet exist, it creates a fully functionaly LinkDef file. If the
    # file already exists, it just inserts one line declaring the new
    # cycle into this file.
    #
    # @param cycleName Name of the analysis cycle. Can contain the namespace name.
    # @param fileName  Optional parameter with the LinkDef file name
    def AddLinkDef( self, cycleName, fileName = "LinkDef.h" ):

        import os.path
        if os.path.exists( fileName ):
            print "AddLinkDef:: Extending already existing file \"" + fileName + "\""
            # Read in the already existing file:
            output = open( fileName, "r" )
            lines = output.readlines()
            output.close()

            # Find the "#endif" line:
            endif_line = ""
            import re
            for line in lines:
                if re.search( "#endif", line ):
                    endif_line = line
            if endif_line == "":
                print "AddLinkDef:: ERROR File \"" + file + "\" is not in the right format!"
                print "AddLinkDef:: ERROR Not adding link definitions!"
                return
            index = lines.index( endif_line )

            # Add the line defining the current analysis cycle:
            lines.insert( index, "#pragma link C++ class %s+;\n" % cycleName )
            lines.insert( index + 1, "\n" )

            # Overwrite the file with the new contents:
            output = open( fileName, "w" )
            for line in lines:
                output.write( line )
            output.close()

        else:
            # Create a new file and fill it with all the necessary lines:
            print "AddLinkDef:: Creating new file called \"" + fileName + "\""
            output = open( fileName, "w" )
            output.write( "// Dear emacs, this is -*- c++ -*-\n" )
            output.write( "// $Id$\n\n" )
            output.write( "#ifdef __CINT__\n\n" )
            output.write( "#pragma link off all globals;\n" )
            output.write( "#pragma link off all classes;\n" )
            output.write( "#pragma link off all functions;\n\n" )
            output.write( "#pragma link C++ nestedclass;\n\n" )
            output.write( "#pragma link C++ class %s+;\n\n" % cycleName )
            output.write( "#endif // __CINT__\n" )

        return

    ## @short Function creating a configuration file for the new cycle
    #
    # This function is supposed to create an example configuration file
    # for the new cycle. It uses PyXML to write the configuration, and
    # exactly this causes a bit of trouble. PyXML is about the worst
    # XML implementation I ever came accross... There are tons of things
    # that it can't do. Not to mention the lack of any proper documentation.
    #
    # All in all, the resulting XML file is not too usable at the moment,
    # it's probably easier just copying one of the example cycles from
    # SFrame/user/config and adjusting it to the user's needs...
    #
    # @param cycleName Name of the analysis cycle. Can contain the namespace name.
    # @param fileName  Optional parameter with the configuration file name
    def CreateConfig( self, cycleName, fileName = "" ):

        # Extract the namespace name if it has been specified:
        namespace = ""
        import re
        if re.search( "::", cycleName ):
            print "CreateConfig:: We're creating a cycle in a namespace"
            m = re.match( "(.*)::(.*)", cycleName )
            namespace = m.group( 1 )
            cycleName = m.group( 2 )
            print "CreateConfig:: Namespace name = " + namespace

        # Construct the file name if it has not been specified:
        if fileName == "":
            fileName = cycleName + "_config.xml"

        # Some printouts:
        print "CreateConfig:: Cycle name     = " + cycleName
        print "CreateConfig:: File name      = " + fileName

        # Use PyXML for the configuration creation:
        import xml.dom.minidom

        doc = xml.dom.minidom.Document()

        doctype = xml.dom.minidom.DocumentType( "JobConfiguration" )
        doctype.publicId = ""
        doctype.systemId = "JobConfig.dtd"
        doc.doctype = doctype

        jobconfig = doc.createElement( "JobConfiguration" )
        doc.appendChild( jobconfig )
        jobconfig.setAttribute( "JobName", cycleName + "Job" )
        jobconfig.setAttribute( "OutputLevel", "INFO" )

        userlib = doc.createElement( "Library" )
        jobconfig.appendChild( userlib )
        userlib.setAttribute( "Name", "YourLibraryNameComesHere" )

        cycle = doc.createElement( "Cycle" )
        jobconfig.appendChild( cycle )
        cycle.setAttribute( "Name", cycleName )
        cycle.setAttribute( "OutputDirectory", "./" )
        cycle.setAttribute( "PostFix", "" )
        cycle.setAttribute( "TargetLumi", "1.0" )

        inputdata = doc.createElement( "InputData" )
        cycle.appendChild( inputdata )
        inputdata.setAttribute( "Type", "Data1" )
        inputdata.setAttribute( "Version", "Reco" )
        inputdata.setAttribute( "Lumi", "0.0" )
        inputdata.setAttribute( "NEventsMax", "-1" )

        infile = doc.createElement( "In" )
        inputdata.appendChild( infile )
        infile.setAttribute( "FileName", "YourInputFileComesHere" )

        userconf = doc.createElement( "UserConfig" )
        cycle.appendChild( userconf )

        confitem = doc.createElement( "Item" )
        userconf.appendChild( confitem )
        confitem.setAttribute( "Name", "NameOfUserProperty" )
        confitem.setAttribute( "Value", "ValueOfUserProperty" )
        
        output = open( fileName, "w" )
        output.write( doc.toprettyxml( encoding="UTF-8" ) )

        return

    ## @short Main analysis cycle creator function
    #
    # The users of this class should normally just use this function
    # to create a new analysis cycle.
    #
    # It only really needs to receive the name of the new cycle, it can guess
    # the name of the LinkDef file by itself if it has to. It calls all the
    # other functions of this class to create all the files for the new
    # cycle.
    #
    # @param cycleName Name of the analysis cycle. Can contain the namespace name.
    # @param linkdef Optional parameter with the name of the LinkDef file
    def CreateCycle( self, cycleName, linkdef = "" ):

        # If the specified name contains a namespace, get just the class name:
        className = cycleName
        import re
        if re.search( "::", cycleName ):
            m = re.match( ".*::(.*)", cycleName )
            className = m.group( 1 )

        # Check if a directory called "include" exists in the current directory.
        # If it does, put the new header in that directory. Otherwise leave it up
        # to the CreateHeader function to put it where it wants.
        import os.path
        if os.path.exists( "./include" ):
            self.CreateHeader( cycleName, "./include/" + className + ".h" )

            if linkdef == "":
                import glob
                filelist = glob.glob( "./include/*LinkDef.h" )
                if len( filelist ) == 0:
                    print "CreateCycle:: WARNING There is no LinkDef file under ./include"
                    print "CreateCycle:: WARNING Creating one with the name ./include/LinkDef.h"
                    linkdef = "./include/LinkDef.h"
                elif len( filelist ) == 1:
                    linkdef = filelist[ 0 ]
                else:
                    print "CreateCycle:: ERROR Multiple header files ending in LinkDef.h"
                    print "CreateCycle:: ERROR I don't know which one to use..."
                    return

            self.AddLinkDef( cycleName, linkdef )

        else:
            self.CreateHeader( cycleName )

            if linkdef == "":
                import glob
                filelist = glob.glob( "*LinkDef.h" )
                if len( filelist ) == 0:
                    print "CreateCycle:: Creating new LinkDef file: LinkDef.h"
                    linkdef = "LinkDef.h"
                elif len( filelist ) == 1:
                    linkdef = filelist[ 0 ]
                else:
                    print "CreateCycle:: ERROR Multiple header files ending in LinkDef.h"
                    print "CreateCycle:: ERROR I don't know which one to use..."
                    return

            self.AddLinkDef( cycleName, linkdef )

        # Check if a directory called "src" exists in the current directory.
        # If it does, put the new source in that directory. Otherwise leave it up
        # to the CreateSource function to put it where it wants.
        if os.path.exists( "./src" ):
            self.CreateSource( cycleName, "./src/" + className + ".cxx" )
        else:
            self.CreateSource( cycleName )

        # Check if a directory called "config" exists in the current directory.
        # If it does, put the new configuration in that directory. Otherwise leave it up
        # to the CreateConfig function to put it where it wants.
        if os.path.exists( "./config" ):
            self.CreateConfig( cycleName, "./config/" + className + "_config.xml" )
        else:
            self.CreateConfig( cycleName )

        return
