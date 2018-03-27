/* writes data to DTK shared memory.  See Usage for details

   John Kelso, 5/04, kelso@nist.gov

 */

#include <dtk.h>
#include <stdlib.h>
#include <string.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <strings.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <vector>

#define PROGRAM_NAME "dtk-writeShm"

static int Usage(char *name) {
  fprintf(stdout,
	 " Usage: "PROGRAM_NAME" SHM_FILE SIZE NUM FORMAT [data]\n\n"
	 "  Write data to the DTK shared memory file named SHM_FILE\n"
	 "  whose size is SIZE bytes.\n"
	 "  SIZE can be give as \"-\" if the shared memory segment\n"
	 "  already exists.\n"
	 "  Data can be specified on the command line, or if none\n"
	 "  are given, via stdin.  Multiple lines of stdin will be\n"
	 "  processed until EOD.  Data will be read using\n"
	 "  scanf(3) with the supplied FORMAT being applied to NUM\n"
	 "  whitespace delimited input items.\n"
	 "  Multiple data sets can be specified per line or command line\n"
	 "  as long as the number of items is divisible by NUM\n"
	 "  SIZE must be divisible by NUM, so SIZE/NUM is the number of\n"
	 "  bytes of each data item.\n"
	 " For reference, here are some popular data types and their sizes in bytes:\n"
	 "   int     %d\n"
	 "   long    %d\n"
	 "   float   %d\n"
	 "   double  %d\n"
	 "   short   %d\n"
	 "   char    %d\n"
	 "   strings will be null padded so each item is SIZE/NUM bytes\n"
	 " Examples\n"
	 "    # six floats on command line- 6*4 = 24\n"
	 "    %s position 24 6 \"%%f\" 1 2 3 90 180 270\n"
	 "    # six floats via stdin\n"
	 "    echo 1 2 3 90 180 270 | %s position 24 6 \"%%f\"\n"
	 "    # three strings, the largest of which is 5 chars + null- 6*3=18\n"
	 "    %s names 18 3 \"%%s\" \"manny\" \"moe\" \"jack\"\n"
	 "    # four chars\n"
	 "    %s foo 4 4 %%c a s d f\n",
	 sizeof(int), sizeof(long), sizeof(float), 
	 sizeof(double), sizeof(short), sizeof(char),
	 name, name, name, name) ;
  return 1 ;
}

int main(int argc, char **argv) {

  dtkMsg.setPreMessage(argv[0]) ;

  for( int i=1;i<argc;i++ )
  {
    if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
      return Usage(argv[0]);
  }
  
  if (argc < 5) 
    return Usage(argv[0]) ;
  
  char *name = argv[1] ;

  int size ;
  bool useExistingSize = false ;
  if (!(strcmp(argv[2],"-"))) {
    useExistingSize = true ;
  } else if ((1 != sscanf(argv[2],"%d",&size) || size<1)) {
    dtkMsg.add(DTKMSG_ERROR, "Malformed SIZE\n") ;
    return 1 ;
  }

  int num ;
  if ((1 != sscanf(argv[3],"%d",&num) || num<1)) {
    dtkMsg.add(DTKMSG_ERROR, "Malformed number of data items (NUM)\n") ;
    return 1 ;
  }

  if (useExistingSize && size%num != 0) {
    dtkMsg.add(DTKMSG_ERROR, "number of bytes (SIZE) is not "
	       "divisible by number of data items (NUM)\n") ;
    return 1 ;
  }

  char *format = argv[4] ;
  dtkSharedMem *shm = NULL ;
  if (useExistingSize) {
    shm = new dtkSharedMem(name) ;
    if(shm->isInvalid()) 
      return 1 ;
    size = shm->getSize() ;
    //printf("using existing size of %d bytes\n",size) ;
  } else {
    shm = new dtkSharedMem(size, name) ;
    if(shm->isInvalid()) 
      return 1 ;
  }
  
  int bytes = size/num ;

  // holds data to be writ to shm
  char *val = (char *) dtk_malloc(size) ;
  memset(val, 0, size) ;

  if (argc>5) { // read from command line if supplied
    if ((argc-5)%num != 0) {
      dtkMsg.add(DTKMSG_ERROR, "number of command line items (%d) "
		 "is not divisible by number of data items (%d)\n", 
		 argc-5, num) ;
      return 1 ;
    }
    for (int i=5; i<argc; i+=num) {
      for (int j=i,jj=0; j<(i+num); j++,jj++) {
	if (!(strcasecmp(argv[4],"%s"))) {  // %s, but can include spaces
	  strcpy(val+(jj*bytes),argv[j]) ;
	} else {  // not %s
	  if (1!=(sscanf(argv[j],format,val+(jj*bytes)))) {
	    dtkMsg.add(DTKMSG_ERROR, "data is not of format \"%s\"\n",
		       format) ;	     
	    return 1 ;
	  }
	}
      }
      if(shm->write(val)) 
	return 1 ;
      memset(val, 0, size) ;
    }
  } else {      // read from stdin
    std::string line ;
    while (getline(std::cin, line)>0) {
      std::vector<std::string> vec;
      tokenize( line, vec, " \t" );
      if (vec.size()%num != 0) {
	dtkMsg.add(DTKMSG_ERROR, "number of items (%d) "
		   "is not divisible by number of data items (%d)\n", 
		   vec.size(), num) ;
	return 1 ;
      }
      for (unsigned int i=0; i<vec.size(); i+=num) {
	for (unsigned int j=i,jj=0; j<(i+num); j++,jj++) {
	  if (1 != (sscanf(vec[j].c_str(),format,val+(jj*bytes)))) {
	    dtkMsg.add(DTKMSG_ERROR, "data \"%s\" not of format \"%s\"\n",
		       vec[j].c_str(), format) ;	     
	    return 1 ;
	  }
	}
	if(shm->write(val)) 
	  return 1 ;
	  memset(val, 0, size) ;
      }
    }
  }
  return 0 ;
}
