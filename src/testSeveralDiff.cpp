#include "xercesc/util/PlatformUtils.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "include/XID_DOMDocument.hpp"
#include "include/XyDelta_DOMInterface.hpp"
#include "include/XyLatinStr.hpp"

#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"

int main(int argc, char *argv[]) {
	if (argc<3) {
		printf("usage: testSeveralDiff <nb> <file1> <file2> ... <fileN>\n");
		printf("                       N can be smaller than <nb>\n");
		return(-1);
	}	

	xercesc_3_0::XMLPlatformUtils::Initialize();

	const int REPEAT = atoi(argv[1]);
	int nbFiles = argc-2;
	char **files = &argv[2];
	int fCount = 0;
	for(int i=0; i<REPEAT; i++) {
		const char * file1 = files[fCount%nbFiles];
		const char * file2 = files[(fCount+1)%nbFiles];
		printf("DIFF %03d - from %s to %s\n", fCount, file1, file2);
		fflush(stdout);
		XID_DOMDocument doc1(file1, false);		
		XID_DOMDocument doc2(file2, false);

		xercesc_3_0::DOMDocument *delta = XyDelta::XyDiff(doc1.getDOMDocumentOwnership(), file1, doc2.getDOMDocumentOwnership(), file2);
		if (delta==NULL) {
			fprintf(stderr, "failed to compute delta\n");
			return -1;
		}
		printf("    releasing...\n");
		fflush(stdout);
		delta->release();
		fCount++;
	}
	return 0;
}
