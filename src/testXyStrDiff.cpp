#include "xercesc/util/PlatformUtils.hpp"
#include "include/XyStrDiff.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>


XERCES_CPP_NAMESPACE_USE

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: testXyStrDiff <str1> <str2>\n");
		return(-1);
	}	

	XMLPlatformUtils::Initialize();
	
	char **strings = &argv[1];
	char *str1 = strings[0];
	char *str2 = strings[1];

	XyStrDiff *strdiff = new XyStrDiff(str1, str2);
	int distance = strdiff->LevenshteinDistance();
	std::cout << "Distance: " << distance << std::endl;
	
	return 0;
}
