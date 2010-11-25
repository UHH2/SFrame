
int SETUP() {

   if( gSystem->Load( "libCintex" ) == -1 ) return -1;
   if( gSystem->Load( "libSFrameCintex" ) == -1 ) return -1;

   return 0;

}
