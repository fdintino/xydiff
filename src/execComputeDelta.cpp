#include "XyDiff/Tools.hpp"
#include "XyDiff/include/XID_map.hpp"
#include "XyDiff/include/XyDelta_FileInterface.hpp"
#include "XyDiff/include/XyLatinStr.hpp"
#include "XyDiff/DeltaException.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"


#include <stdio.h>
#include <math.h>
#include <fstream>

#include <sys/timeb.h>

#define HWPROF_LOG 0

#if HWPROF_LOG
FILE *timeFile ;

extern unsigned long long int   clocksCDDsaveXidMap        ;
extern unsigned long long int clocksRegisterSubtree        ;
extern unsigned long long int clocksTopDownMatch           ;
extern unsigned long long int clocksOptimize               ;
extern unsigned long long int clocksConstructDeltaDocument ;
extern unsigned long long int clocksSaveDelta              ;

#endif

unsigned long getFileSize(const char *filename) {
        FILE *file = fopen(filename, "rb");
        if (file==NULL) return(0);
 
        (void)fseek(file, 0, SEEK_END);
        unsigned long size = ftell( file );
        fclose(file);
        return size ;
        }    
			
                                                                   
int main(int argc, char **argv) {
 
#if HWPROF_LOG
	timeFile = fopen("timefile.txt", "wb");
	if (timeFile==NULL) {
 		fprintf(stderr, "could not open timefile.txt\n");
		exit(0);
		}
	printf("Logging time profile\n");
#endif

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
    xercesc_2_2::XMLPlatformUtils::Initialize();
    }
  catch(const xercesc_2_2::XMLException& toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n" << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
    }

  // Open source document, and create DELTA

	try {
		if (defaultDeltafile) deltafile=file1+".forwardDelta.xml";
		/* Compute Delta over FILE1.XML and FILE2.XML */
		XyDelta::XyDiff(file1.c_str(), file2.c_str(), deltafile.c_str(), ignoreSpacesFlag);

#if HWPROF_LOG
		if (timeFile!=NULL) {
			printf("add timing profile to log\n");
			// Time Data for Algorithm Analysis
			unsigned long long int phase2 = clocksRegisterSubtree ;
			unsigned long long int phase3 = clocksTopDownMatch ;
			unsigned long long int phase4 = clocksOptimize ;
			unsigned long long int phase5 = clocksConstructDeltaDocument - clocksCDDsaveXidMap ;
			fprintf(timeFile, "# data size, phase1+2; phase3; phase4; phase5\n");
			unsigned long dataSize = getFileSize(v0filename)+getFileSize(v1filename) ;
			fprintf(timeFile, "%7d %7d %7d %7d %7d\n",
								(int)dataSize,
						    (int)(phase2/(CLOCKRATE/1000000)),
								(int)(phase3/(CLOCKRATE/1000000)),
								(int)(phase4/(CLOCKRATE/1000000)),
								(int)(phase5/(CLOCKRATE/1000000))
								);
			}
#endif
		}

	catch( const DeltaException &e ) {
		std::cerr << "delta exception. Exit program" << std::endl ;
		exit(-1);
		}
		
	catch( const VersionManagerException &e ) {
		std::cerr << e << std::endl ;
		exit(-1);
		}
		
	catch( const xercesc_2_2::DOMException &e ) {
		std::cerr << "DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOMException, message=" << e.msg << std::endl ;
		exit(-1);
		}	
	
#if HWPROF_LOG
	fclose( timeFile );
#endif
	
	return(0);
	}
	
