
int SETUP() {

   if( gSystem->Load( "libTree" ) == -1 ) return -1;
   if( gSystem->Load( "libHist" ) == -1 ) return -1;
   if( gSystem->Load( "libGraf" ) == -1 ) return -1;
   if( gSystem->Load( "libSFrameUser" ) == -1 ) return -1;

   return 0;

}
