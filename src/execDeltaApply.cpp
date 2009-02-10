#include "DeltaApply.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"

#include "include/XyDelta_FileInterface.hpp"
#include "include/XyLatinStr.hpp"
#include "Tools.hpp"
#include "DeltaException.hpp"
#include "include/XID_map.hpp"
#include <stdio.h>
#include <string>

XERCES_CPP_NAMESPACE_USE

int main(int argc, const char **argv) {
 
	bool showUsage= false; 
	for(int i=1; i<argc; i++) if (strcmp(argv[i], "--help")==0) showUsage=true;
 	if ((argc<2)||(showUsage)) {
		std::cerr << "usage: " << argv[0] << " [--apply-annotations] [-i inputFile.xml] [-o outputFile.xml] deltaFile.xml" << std::endl ;
		std::cerr << std::endl ;
		std::cerr << "by default input and output files are read in the 'from' and 'to' attributes of the delta" << std::endl ;
		return(0);
		}
 
  try {
	  std::cout << "XMLPlatformUtils::Initialize()" << std::endl ;
    XMLPlatformUtils::Initialize();
    }
  catch(const XMLException& toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
	       << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
    }

	std::string deltaName = argv[argc-1];
	const char *sourceName = NULL ;
	const char *destinationName = NULL ;
	bool applyAnnotations = false;
	for(int i=1; i<argc-1; i++) {
		if (strcmp(argv[i], "-i")==0) sourceName = argv[i+1];
		if (strcmp(argv[i], "-o")==0) destinationName = argv[i+1];
		if (strcmp(argv[i], "--apply-annotations")==0) applyAnnotations = true;
		}
		
	try {
		XyDelta::ApplyDelta( deltaName.c_str(), sourceName, destinationName, applyAnnotations );
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
	catch( const DOMException &e ) {
		std::cerr << "*** DOMException, code=" << e.code << std::endl ;
		std::cerr << "    DOMException, message=" << XMLString::transcode(e.msg) << std::endl ;
		exit(-1);
	}	
	catch(const XMLException& toCatch) {
		std::cerr << "*** XML Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
		exit(-1);
	}
	catch(...) {
		std::cerr << "unknown exception. Abort" << std::endl ;
		exit(-1);
		}
	std::cout << "Quit." << std::endl ;
	return 0;
	}
	
