#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	
	char s[5000];
	int filesNumber=10;

    if (argc != 2) {
		fprintf(stderr,"usage: exec filesNumber \n");
		exit(-1);
	}

		
	filesNumber=(int)strtol(argv[1],NULL,10);
	if (filesNumber<1) {
		fprintf(stderr, "input error: wrong number of versions \n");
		exit(-1);
	}
	
	fprintf(stderr, "\n\n[[[NUMBER OF VERSIONS: %d]]]\n\n", filesNumber);
	
    int rinit=system("cd ./dirtest");
	if (rinit!=0) {
		fprintf(stderr, "Failed to go to directory ./dirtest\n");
		exit(-1);
	}
	
	fprintf(stderr, "\n\n[[[USE XyLoad TO LOAD THE VERSIONS IN THE REPOSITORY]]]\n\n");
	
	for (int i=0; i<filesNumber; i++) {
        sprintf(s,"./XyLoad ./dirtest/version%02d.xml ./dirtest/delta.xml", i+1);
		fprintf(stderr, "command=%s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "XyLoad: command failed return code is %d\n", r);
			exit(-1);
		}
	}
	
	fprintf(stderr, "\n\n[[[USE XyRetrieve TO RETRIEVE THE VERSIONS FROM THE DELTA FILE]]]\n\n");
	
	for (int i=1; i < filesNumber; i++) {
		sprintf(s, "./XyRetrieve ./dirtest/delta.xml -v %2d", i);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyRetrieve return code %d\n", r);
			exit(-1);
		}
#if 0
		sprintf(s, "cmp ./dirtest/backwardVersion.xml ./dirtest/version%02d.xml", filesNumber-i);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: cmp xml files return code %d\n", r);
			exit(-1);
		}
#endif
		sprintf(s, "./XyCmpXML ./dirtest/backwardVersion.xml ./dirtest/version%02d.xml", filesNumber-i);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyCmpXML xml files return code %d\n", r);
			exit(-1);
		}
		sprintf(s, "cmp ./dirtest/backwardVersion.xml.xidmap ./dirtest/version%02d.xml.xidmap", filesNumber-i);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: cmp xidmap files return code %d\n", r);
			exit(-1);
		}
		fprintf(stderr, "STEP %d OK\n", i);
	}

    
	fprintf(stderr, "\n\nWORKS FINE !!!\n\n");
	return(0);
}
