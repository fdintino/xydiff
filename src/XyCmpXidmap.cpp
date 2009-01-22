#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "XyDiff/include/XID_map.hpp"

using namespace std;

std::string getPureXidmap(const char *filename) {
	
	char s[5000];
	
	FILE *fd;
	if ((fd = fopen(filename, "r"))==NULL) {
		fprintf(stderr, "error: the file %s doesn't exist\n", filename);
		exit(-1);
	}
	fscanf(fd, "%s", s);
	fclose(fd);
	std::string xidmap(s);
	unsigned int pos = xidmap.find('|',0);
	if (pos == std::string::npos) {
		fprintf(stderr, "error: not a valid xidmap file");
		exit(-1);
	}
	//cout << xidmap << endl << "pos=" << pos << endl;
	std::string pureXidmap = xidmap.substr(0,pos) + ")";
	return pureXidmap;
}

int main(int argc, char *argv[]) {
	
	bool argsOk = false;
	if (argc==3) argsOk=true;
	if ((argc==4)&&(strcmp(argv[1], "-n")==0)) argsOk=true;
	if (!argsOk) {
		fprintf(stderr, "usage: XyCmpXidmap [-n] file1.xidmap file2.xidmap \n");
		fprintf(stderr, "             -n : only compare the number of nodes, not the values of the identifiers\n");
		exit(-1);
	}

	const char *file1 = argv[argc-2];
	const char *file2 = argv[argc-1];
	
	std::string xidmap1 = getPureXidmap(file1);
	std::string xidmap2 = getPureXidmap(file2);

	if (argc==3) {
		if (xidmap1==xidmap2) {
			std::cout << "The xidmaps from files <" << file1 << "> and <" << file2 << "> are identical" << endl ;
			return 0;
		} else {
			std::cout << "The xidmaps from files <" << file1 << "> and <" << file2 << ">  are different" << endl;
			std::cout << xidmap1 << endl << xidmap2 << endl;
			exit(-1);
		}
	} else {
		XidMap_Parser p1(xidmap1.c_str());
		XidMap_Parser p2(xidmap2.c_str());
		int n1=0;
		while (!p1.isListEmpty()) {
			(void)p1.getNextXID();
			n1++;
		}
		int n2=0;
		while (!p2.isListEmpty()) {
			(void)p2.getNextXID();
			n2++;
		}
		if (n1==n2) {
			std::cout << "The xidmaps from files <" << file1 << "> and <" << file2 << "> have the same number of nodes ("<<n1<<")"<< endl ;
			return 0;
		} else {
			std::cout << "The xidmaps from files <" << file1 << "> and <" << file2 << "> do not have the same number of nodes ("<<n1<<" and "<<n2<<")"<< endl ;
			exit(-1);
		}
	}
	std::cerr << "Internal error - unreachable code" << endl;
	return(-1);
}
