#ifndef DELTAMANAGER_HXX
#define DELTAMANAGER_HXX

// A Delta Document contains all deltas corresponding to versions of some document.
// It is represented as follows:
//
// The root node is <unit_delta>
//    Below are <t> nodes corresponding to all deltas from version i to j of the document.
//    For instance <t fromVersionId="14" toVersionId="15" fromDate="..." toDate="..." >
//       Below each <t> nodes are the operations corresponding to the delta: <d> (delete), <i> (insert), <ai>,<au>,<ad> for attributes insert, update and delete
//
//

#include "xercesc/dom/DOMNode.hpp"
#include "XyDiff/include/XID_DOMDocument.hpp"
//#include "RepositoryContext/VersionManager.hpp"

#include <vector>
#include <string>

class DeltaHeader {
	public:
		DeltaHeader(xercesc_2_2::DOMNode *deltaElement);
		
		typedef int VersionId_t;

		VersionId_t getFromVersionId();
		VersionId_t getToVersionId();
		std::string      getFromDate();
		std::string      getToDate();
	private:		
		VersionId_t fromVersionId;
		VersionId_t toVersionId;
		std::string      fromDate;
		std::string      toDate;
} ;

class DeltaManager {
	public:
		DeltaManager(const char *deltaFileName); // loads the entire delta document from the file
		DeltaManager(const char *deltaFileName, const char *newVersionFileName ); // creates a new delta document
		
		typedef int VersionId_t ;

		int addDeltaElement(xercesc_2_2::DOMNode *deltaElement, const char *versionFile); // add a delta. The version ID's are found inside the delta. Return 0 if ok, negative code if not.
		xercesc_2_2::DOMNode* getDeltaElement(DeltaHeader::VersionId_t fromVersionId, DeltaHeader::VersionId_t toVersionId = -1); // finds the delta from <i> to <j>. If <j> is not specified, <j> will be considered to be j=i+1

		const std::vector<DeltaHeader> & getDeltaList();
		xercesc_2_2::DOMDocument* getDeltaDocument(); // get the entire deltadocument
		std::string getCurrentVersionFileName(); 
		
		void listAllDeltas();
		void listAllDocumentVersions();

		void setFileName(const char *filename); // change the file name (in order to specify where to save, or to do SaveAs)
		void SaveToDisk(); // save changes to disk if constructed from a file
		
		
	private:
		XID_DOMDocument* deltaDocument;
		std::string deltaFileName;
		std::string newVersionFileName;
		std::string currentVersionFileName;
        int firstAvailableVersionId;
		std::vector<DeltaHeader> headerList;
} ;

#endif
