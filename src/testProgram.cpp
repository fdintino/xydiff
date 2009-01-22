#include <stdio.h>
#include <stdlib.h>
#include <cstring>

char s[5000];
int testlevel=20;

using namespace std;

void generateTestFiles(int argc, char **argv) {
	if (argc>1) {
		testlevel=(int)strtol(argv[1],NULL,10);
		if (testlevel<1) testlevel=10;
		}
	fprintf(stderr, "\n\n[[[RUNNING WITH %d LEVELS]]]\n\n", testlevel);
	
	fprintf(stderr, "\n\n[[[CREATE TEST DIRECTORY]]]\n\n");
	
	int rinit=system("rm -Rf ./testdir");
	if (rinit==0) rinit=system("mkdir ./testdir");
	if (rinit!=0) {
		fprintf(stderr, "Failed to create test directory in ./testdir\n");
		exit(-1);
		}
	
	fprintf(stderr, "\n\n[[[CREATE RANDOM DELTA AND REFERENCE FILES]]]\n\n");
	
	int r0=-1;
	int tryCount=0;
	while(r0!=0) {
		sprintf(s,"./XyCreateRandomDelta ./testdir/ex_delta%%03d.xml -start ./example.xml ./testdir/example%%03d.xml %d 0.1 0.2 0.1 >./log_createdelta.txt", testlevel);
		fprintf(stderr, "command=%s\n", s);
		r0=system(s);
		if (r0!=0) {
			tryCount++;
			if (tryCount<10) {
				fprintf(stderr, ">>> Random Generation Of Files Created An Empty Document.... Retrying...\n");
			}
			else {
				fprintf(stderr, "command failed return code is %d\n", r0);
				fprintf(stderr, "\n\nXyDiff program has not yet been tested: only the TEST sequence that has just been generated is not good enough for testing.\n\nPlease try again.\n");
				fprintf(stderr, "or see ./log_createdelta.txt for more details\n");
				exit(-1);
			}
		}
	}
}

void cleanTestFiles(void) {
	fprintf(stderr, "[[[REPLAY OPTION - USING EXISTING TEST FILES]]]\n\n");
	fprintf(stderr, "cleaning test files\n");
	const char * command;
	command = "rm log_*.txt";
	system(command);
	command = "rm example.xml.xidmap";
	system(command);
	command = "mv testdir REFtestdir";
	system(command);
	command = "mkdir testdir";
	system(command);
	command = "cp REFtestdir/*_expected* REFtestdir/ex_delta*.xml REFtestdir/example000.xml testdir";
	system(command);
	command = "rm -Rf REFtestdir";
	system(command);
}

int main(int argc, char *argv[]) {
	
	bool replay = false;
	if (argc>1) if (strcmp(argv[1], "-replay")==0) replay=true;
	if (argc>2) if (strcmp(argv[2], "-replay")==0) replay=true;
	
	if (!replay) generateTestFiles(argc, argv);
	else cleanTestFiles();
	
	fprintf(stderr, "\n\n[[[TEST APPLY DELTA AND COMPARE TO REFERENCES]]]\n\n");
	
	for(int i=0; i<testlevel; i++) {
		fprintf(stderr, "STEP %d\n", i);
		sprintf(s, "./XyDeltaApply ./testdir/ex_delta%03d.xml >./log_deltaApply%03d.txt", i, i);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyDeltaApply return code %d\n", r);
			exit(-1);
			}
		sprintf(s, "cmp ./testdir/example%03d.xml ./testdir/example%03d.xml_expected", i+1, i+1);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: cmp xml files return code %d\n", r);
			exit(-1);
			}
		sprintf(s, "./XyCmpXML ./testdir/example%03d.xml ./testdir/example%03d.xml_expected", i+1, i+1);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyCmpXML xml files return code %d\n", r);
			exit(-1);
			}
		sprintf(s, "cmp ./testdir/example%03d.xml.xidmap ./testdir/example%03d.xml_expected.xidmap", i+1, i+1);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: cmp xidmap files return code %d\n", r);
			exit(-1);
			}
		fprintf(stderr, "STEP %d OK\n\n", i);
		}

	fprintf(stderr, "\n\n[[[DIFF THE XML FILES]]]\n\n");

	for(int i=0; i<testlevel; i++) {
		fprintf(stderr, "DIFF %d->%d\n", i, i+1);
		sprintf(s, "rm -f ./testdir/example%03d.xml.xidmap", i+1);
		fprintf(stderr, "command= %s\n", s);
		if (system(s)!=0) exit(-1);
		sprintf(s, "./XyDiff ./testdir/example%03d.xml ./testdir/example%03d.xml >testdir/log_diffFrom%03dTo%03d.txt", i, i+1, i, i+1);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyComputeDelta return code %d\n", r);
			exit(-1);
			}
		fprintf(stderr, "STEP %d OK\n\n", i);
		}

	fprintf(stderr, "\n\n[[[DELETE ONE OF THE TWO XML FILES SEQUENCE]]]\n\n");

	for(int i=1; i<=testlevel; i++) {
		sprintf(s, "rm -f ./testdir/example%03d.xml ./testdir/example%03d.xml.xidmap", i, i);
		fprintf(stderr, "command= %s\n", s);
		if (system(s)!=0) exit(-1);
		}

	fprintf(stderr, "\n\n[[[RECONSTRUCT AND VERIFY XML FILES WITH RESULTS FROM XYDIFF]]]\n\n");
	for(int i=0; i<testlevel; i++) {
		fprintf(stderr, "STEP %d\n", i);
		sprintf(s, "./XyDeltaApply ./testdir/example%03d.xml.forwardDelta.xml >./testdir/log_diffDeltaApply%03d.txt", i, i);
		fprintf(stderr, "command= %s\n", s);
		int r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyDeltaApply return code %d\n", r);
			exit(-1);
			}
		sprintf(s, "cmp ./testdir/example%03d.xml ./testdir/example%03d.xml_expected", i+1, i+1);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: cmp xml files return code %d\n", r);
			exit(-1);
			}
		sprintf(s, "./XyCmpXML ./testdir/example%03d.xml ./testdir/example%03d.xml_expected", i+1, i+1);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyCmpXML files return code %d\n", r);
			exit(-1);
			}
		sprintf(s, "./XyCmpXidmap -n ./testdir/example%03d.xml.xidmap ./testdir/example%03d.xml_expected.xidmap", i+1, i+1);
		fprintf(stderr, "command= %s\n", s);
		r=system(s);
		if (r!=0) {
			fprintf(stderr, "ERROR: XyCmpXidmap files return code %d\n", r);
			exit(-1);
			}
		fprintf(stderr, "STEP %d OK\n\n", i);
		}

	fprintf(stderr, "\n\nWORKS FINE !!!\n\n");
	exit(0);
	}
