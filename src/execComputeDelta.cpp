#include "Tools.hpp"
#include "xydiff/XID_map.hpp"
#include "xydiff/XyDelta_FileInterface.hpp"
#include "xydiff/XyLatinStr.hpp"
#include "xydiff/DeltaException.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"


#include <stdio.h>
#include <math.h>
#include <fstream>

#include <sys/timeb.h>

#define HWPROF_LOG 0

XERCES_CPP_NAMESPACE_USE

unsigned long getFileSize(const char *filename) {
        FILE *file = fopen(filename, "rb");
        if (file==NULL) return(0);
 
        (void)fseek(file, 0, SEEK_END);
        unsigned long size = ftell( file );
        fclose(file);
        return size ;
        }    
			
                                                                   
int main(int argc, char **argv) {
 
	std::string file1, file2 ;
	bool ignoreSpacesFlag=false;
	bool defaultDeltafile=true;
	bool verbose=false;
	std::string deltafile ;
	
	bool help=false;
	for(int i=1; i<argc; i++) if (strcmp(argv[i], "--help")==0) help=true;
	
	int pos=0;
	if (argc<pos+2) help=true;
	if ((!help)&&(strcmp(argv[pos+1], "-v")==0)) {
		verbose=true;
		pos++;
		}
	if (argc<pos+2) help=true;
	if ((!help)&&(strcmp(argv[pos+1], "--ignore-spaces")==0)) {
		ignoreSpacesFlag=true;
		pos++;
		}
	if (argc<pos+2) help=true;
	if ((!help)&&(strcmp(argv[pos+1], "-o")==0)) {
		if (argc<pos+3) help=true;
		else deltafile=argv[pos+2];
		pos+=2;
		defaultDeltafile=false;
		}
	if (argc<pos+3) help=true;
	if (!help) {
		file1=argv[pos+1];
		file2=argv[pos+2];
		pos+=2;
		if (argc>pos+1) help=true;
		}	
	
	if (help) {
		std::cerr << "usage       : exec [-v] [--ignore-spaces] [-o deltafile.xml] file1.xml file2.xml" << std::endl ;
		std::cerr << "example     : exec -o ex_delta.xml example1.xml example1bis.xml" << std::endl ;
		std::cerr << "description : " << std::endl ;
		std::cerr << "              file1.xml, file2.xml : XyDiff creates a delta representing changes\n"
		          << "                                     that transform <file1.xml> into <file2.xml>" << std::endl;
		std::cerr << "              -v                   : verbose\n" << std::endl ;
		std::cerr << "              --ignore-spaces      : ignore some differences between documents\n"
				  << "                                     if they only concern blank text parts\n"
				  << "                                     (e.g. white spaces used for indentation)" << std::endl;
		std::cerr << "              -o deltafile.xml     : file to store the delta.\n"
				  << "                                     by default <file1.xml.forwardDelta.xml>" << std::endl ;
		exit(-2);
		}
	
	try {
		XMLPlatformUtils::Initialize();
	}
	catch(const XMLException& toCatch) {
		std::cerr << "Error during Xerces-c Initialization.\n" << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
	}

  // Open source document, and create DELTA

	try {
		// if (defaultDeltafile) deltafile=file1+".forwardDelta.xml";
		if (defaultDeltafile) deltafile="stdout";
		/* Compute Delta over FILE1.XML and FILE2.XML */
		XyDelta::XyDiff(file1.c_str(), file2.c_str(), deltafile.c_str(), ignoreSpacesFlag);

		}

	catch( const DeltaException &e ) {
		std::cerr << "delta exception. Exit program" << std::endl ;
		exit(-1);
		}
		
	catch( const VersionManagerException &e ) {
		std::cerr << e << std::endl ;
		exit(-1);
		}
		
	catch( const DOMException &e ) {
		std::cerr << "DOMException, code=" << e.code << std::endl ;
		char * tmpChar = XMLString::transcode(e.msg);
		std::cerr << "DOMException, message=" << tmpChar << std::endl ;
		XMLString::release(&tmpChar);
		exit(-1);
		}	

	return(0);
	}
	
