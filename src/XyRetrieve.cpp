#include "XyDiff/DeltaApply.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"

#include "XyDiff/include/XyDelta_FileInterface.hpp"
#include "XyDiff/include/XyStr.hpp"
#include "XyDiff/Tools.hpp"
#include "XyDiff/DeltaException.hpp"
//#include "include/XID_map.hxx"
//#include <stdio.h>
//#include <string>

int main(int argc, const char **argv) {
 
	bool showUsage= false; 
	for(int i=1; i<argc; i++) if (strcmp(argv[i], "--help")==0) showUsage=true;
 	if ((argc<2)||(showUsage)) {
		std::cerr << "============" << std::endl;
		std::cerr << "usage:	exec delta_file.xml [-v backward_number]" << std::endl ;
		std::cerr << std::endl ;
		std::cerr << "description:" << std::endl;
		std::cerr << "	Recovers a specified version of the XML document with a given delta file" << std::endl;
		std::cerr << "	delta_file.xml: 	name of the delta file" << std::endl 
			 << "	backward_number: 	integer representing how many versions to go backward" << std::endl;
		return(0);
	}
 
	try {
		std::cout << "XMLPlatformUtils::Initialize()" << std::endl ;
		xercesc_2_2::XMLPlatformUtils::Initialize();
    }
	catch(const xercesc_2_2::XMLException& toCatch) {
		std::cerr << "Error during Xerces-c Initialization.\n"
			 << "  Exception message:" << toCatch.getMessage() << std::endl;
    }

	std::string deltaName = argv[1];
	int backwardNumber = 0;
	for(int i=2; i<argc-1; i++) {
		if (strcmp(argv[i], "-v")==0) backwardNumber = atoi(argv[i+1]);
	}
	if (backwardNumber < 0) {
		backwardNumber = 0;
	}

	try {
		XyDelta::ApplyDelta( deltaName.c_str(), backwardNumber );
		std::cout << "Job finished." << std::endl ;
	}
	
	catch(DeltaException &e) {
		fprintf(stderr,"DeltaException catched. Abort\n");
		exit(-1);
	} 
	catch( const VersionManagerException &e ) {
		std::cerr << e << std::endl ;
		exit(-1);
	}
	catch( const xercesc_2_2::DOMException &e ) {
		std::cerr << "*** DOMException, code=" << e.code << std::endl ;
		std::cerr << "    DOMException, message=" << e.msg << std::endl ;
		exit(-1);
	}	
	catch(const xercesc_2_2::XMLException& toCatch) {
		std::cerr << "*** XML Exception message:" << toCatch.getMessage() << std::endl;
		exit(-1);
	}
	catch(...) {
		std::cerr << "unknown exception. Abort" << std::endl ;
		exit(-1);
		}
	std::cout << "Quit." << std::endl ;
	return(0);
	}
	
