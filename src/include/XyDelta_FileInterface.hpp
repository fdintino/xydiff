#ifndef COMPUTEDELTA_HXX
#define COMPUTEDELTA_HXX

#include <stdio.h>

namespace XyDelta {
	void XyDiff(const char *incV0filename, const char *incV1filename, const char *deltafilename, bool ignoreSpacesFlag=false, const bool verbose=false);
	void XyLoadAndDiff(const char *versionFile , const char *deltaFile);
	void ApplyDelta(const char *incDeltaName, const char *incSourceName=NULL, const char *incDestinationName=NULL);
	void ApplyDelta(const char *incDeltaName, unsigned int backwardNumber = 0);
} ;
  
namespace SpinProject {
	int RunDiff(const char *openFileV0, const char *openFileV1, const char *saveFileV0, const char *saveFileV1, const char *saveDeltaV0V1);
}


#endif
