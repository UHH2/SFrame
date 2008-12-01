
int SETUP() {

   if( gSystem->Load( "libTree" ) == -1 ) return -1;
   if( gSystem->Load( "libHist" ) == -1 ) return -1;
   if( gSystem->Load( "libXMLParser" ) == -1 ) return -1;
   if( gSystem->Load( "libProof" ) == -1 ) return -1;
   if( gSystem->Load( "libSFrameCore" ) == -1 ) return -1;

   return 0;

}
