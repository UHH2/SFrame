NAME="Test"

directories="config include proof src"
files="ChangeLog Makefile include/${NAME}_LinkDef.h proof/BUILD.sh proof/SETUP.C"

mkdir $NAME
for d in $directories; do mkdir $NAME/$d; done
for f in $files; do touch $NAME/$f; done

cat << EOT > $NAME/Makefile
# Package information
LIBRARY = $NAME
OBJDIR  = obj
DEPDIR  = \$(OBJDIR)/dep
SRCDIR  = src
INCDIR  = include

# Include the generic compilation rules
include \$(SFRAME_DIR)/Makefile.common
EOT

cat << EOT > $NAME/include/${NAME}_LinkDef.h
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;

// The example cycles:
//#pragma link C++ class FirstCycle+;

#endif // __CINT__
EOT

cat << EOT > $NAME/proof/SETUP.C
int SETUP() {

   if( gSystem->Load( "libTree" ) == -1 ) return -1;
   if( gSystem->Load( "libHist" ) == -1 ) return -1;
   if( gSystem->Load( "libGraf" ) == -1 ) return -1;
   if( gSystem->Load( "libSFramePlugIns" ) == -1 ) return -1;

   return 0;
}
EOT

cat << EOT > $NAME/proof/BUILD.sh
if [ "$1" = "clean" ]; then
    make distclean
    exit 0
fi

make default
EOT
