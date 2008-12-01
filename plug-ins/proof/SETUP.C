
int SETUP() {

   if( gSystem->Load( "libGenVector" ) == -1 ) return -1;
   if( gSystem->Load( "libSFramePlugIns" ) == -1 ) return -1;

   return 0;

}
