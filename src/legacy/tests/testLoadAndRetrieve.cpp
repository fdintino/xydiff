#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <string>
#include <iostream>

#define MIN_VERSIONS_NUMBER 50

using namespace std;

int main(int argc, char *argv[]) {
	
	char s[5000];

    if (argc < 4) {
		fprintf(stderr,"usage: exec <relative_path> <version1.xml> <version2.xml> [ ... <versionN.xml> ] \n");
		exit(-1);
	}

    int filesNumber = argc - 2;
	char* relativePath = argv[1];
    fprintf(stderr, "\n\n[[[NUMBER OF VERSIONS: %d]]]\n\n", filesNumber);
	fprintf(stderr, "\n\n[[[RELATIVE PATH: %s]]]\n\n", relativePath);

	// test if relativePath ends with a "/" character
	char c1 = relativePath[strlen(relativePath)-1];
	char c2 = '/';
    if ( c1 != c2 ) {
		fprintf(stderr, "Relative path without \" / \" termination\n");
		exit(-1);
	}

	// the test files must be in this directory "relativePath" (to the current one)
	sprintf(s,"cd %s",relativePath);
	fprintf(stderr, "command=%s\n", s);
    int rinit=system(s);
	if (rinit!=0) {
		fprintf(stderr, "Failed to go to directory %s\n", relativePath);
		exit(-1);
	}
    
	// construct the random list of versions of xml document and load them in the repository

	std::vector<int> indexList;		// the coresponding real source files numbers for the list of versions 
	std::vector<std::string> versionsList;	// the list of the versions

	int versionsNumber = MIN_VERSIONS_NUMBER + (int) ( ((float)50)*rand()/(RAND_MAX+1.0) ); // the length of the versions list
	std::cout << "Number of versions : " << versionsNumber << endl;

	fprintf(stderr, "\n\n[[[USE XyLoad TO LOAD THE VERSIONS IN THE REPOSITORY]]]\n\n");

	int idx = 0;
	char filename[1000];
	for (int i=0; i<versionsNumber; i++) {
		idx = 2 + (int) ( ((float)filesNumber)*rand()/(RAND_MAX+1.0) );	// a random number between 0 and filesNumber-1 
		indexList.push_back(idx);				// add the index of file to the list
		sprintf(filename, "tmpf%02d.xml", i);
		std::string str(filename);
		versionsList.push_back(str);			// add the name of the new version file to the list 
		// create the new version file 
		sprintf(s, "cp %s%s %s%s", relativePath, argv[idx], relativePath, filename);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: copy command returns code %d\n", r);
			exit(-1);
		}

		//cout << "idx=" << idx << " argv[idx]=" << argv[idx] << endl;
        sprintf(s,"./XyLoad %s%s %sdelta.xml", relativePath, filename, relativePath);
		fprintf(stderr, "command=%s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "XyLoad: command failed return code is %d\n", r);
			exit(-1);
		}
	}

	std::cout << "Number of versions : " << versionsNumber << endl;
	// list the versions indexes
	std::vector<int>::iterator p; 
	std::cout << endl << "The order of versions:" << endl;
	for ( p = indexList.begin(); p != indexList.end(); p++ ) {
    	std::cout << *p << " ";
	}
	std::cout << endl;
	
	fprintf(stderr, "\n\n[[[USE XyRetrieve TO RETRIEVE THE VERSIONS FROM THE DELTA FILE]]]\n\n");
	
	for (int i=1; i < versionsNumber; i++) {
        sprintf(s, "./XyRetrieve %s delta.xml -v %2d", relativePath, i);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyRetrieve return code %d\n", r);
			exit(-1);
		}
        
		sprintf(s, "./XyCmpXML %sbackwardVersion.xml %s%s", relativePath, relativePath, versionsList[versionsNumber-i-1].c_str() );
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyCmpXML xml files return code %d\n", r);
			exit(-1);
		}


		sprintf(s, "./XyCmpXidmap %sbackwardVersion.xml.xidmap %s%s.xidmap", relativePath, relativePath, versionsList[versionsNumber-i-1].c_str() );
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyCmpXidmap xidmap files return code %d\n", r);
			exit(-1);
		}

		sprintf(s, "rm %sbackwardVersion.xml* ", relativePath);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: rm backwardVersion* files return code %d\n", r);
			exit(-1);
		}

		fprintf(stderr, "STEP %d OK\n", i);
	}

	sprintf(s, "rm %stmp* %sdelta.xml ", relativePath, relativePath);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: rm delta and tmp files return code %d\n", r);
			exit(-1);
	}

	std::cout << "Number of versions : " << versionsNumber << endl;
	// list the versions indexes
	std::cout << endl << "The order of versions:" << endl;
	for ( p = indexList.begin(); p != indexList.end(); p++ ) {
    	std::cout << *p << " ";
	}
	std::cout << endl;

	fprintf(stderr, "\n\nWORKS FINE!!!\n\n");
	return(0);
}
