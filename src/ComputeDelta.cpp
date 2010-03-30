#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"

#include "DOMPrint.hpp"
#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "Diff_NodesManager.hpp"
#include "Diff_DeltaConstructor.hpp"
#include "Diff_UniqueIdHandler.hpp"
#include "DeltaException.hpp"
#include "DeltaManager.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/validators/DTD/DTDValidator.hpp"

#include "infra/general/Log.hpp"

#include <stdio.h>
#include <map>
#include <queue>
#include <list>
#include <math.h>
#include <fstream>
#include "infra/general/hash_map.hpp"

//class DTDValidator;



XERCES_CPP_NAMESPACE_USE
/*******************************************************************
 *                                                                 *
 *                                                                 *
 * Main Functions                                                  *
 *                                                                 *
 * Author: Gregory COBENA                                          *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 *******************************************************************/

// apelat din XyDiff() de mai jos
XID_DOMDocument* XidXyDiff(XID_DOMDocument* v0XML, const char *doc1name, XID_DOMDocument* v1XML, const char *doc2name, bool ignoreSpacesFlag=false, bool verbose=false) {

	/* ---- [[ Phase 1: ]] Compute signatures and weight for subtrees on both documents ---- */
	
	TRACE("PHASE 1: Register ID-Attributes");
	
	UniqueIdHandler myUniqueIdHandler ;
		
	// Create object that manages meta-data for nodes of both documents
	class NodesManager xyMappingEngine;
		
	vddprintf(("\n+++ Phase 1: Compute signature and weight for subtrees on both documents +++\n\n")) ;
		
	xyMappingEngine.setUniqueIdHandler( &myUniqueIdHandler );
	TRACE("PHASE 1: Register version 1");
	xyMappingEngine.registerSourceDocument( v0XML );
	TRACE("PHASE 1: Register version 2");
	xyMappingEngine.registerResultDocument( v1XML );
		
	int v0rootID = xyMappingEngine.sourceNumberOfNodes ;
	int v1rootID = xyMappingEngine.resultNumberOfNodes ;
			
	/* ---- [[ Phase 2: ]] Apply Bottom-Up Lazy-Down Algorithm ---- */
		
	vddprintf(("\n+++ Phase 2: Apply BULD (Bottom-Up Lazy Down) matching algorithm starting from heaviest subtrees +++\n\n")) ;
		
	TRACE("PHASE 2: Match by ID-Attributes");
	vddprintf(("Match node which have ID attributes\n")) ;
	xyMappingEngine.MatchById( v1rootID );

	TRACE("PHASE 2: Full Bottom-up propagation pass #1");
	vddprintf(("Full Bottom-up propagation pass #1\n")) ;
	xyMappingEngine.FullBottomUp( v1rootID );

	TRACE("PHASE 2: Apply BULD Algorithm");
	vddprintf(("Top-down matching on new document\n")) ;
	xyMappingEngine.topdownMatch( v0rootID, v1rootID );
		
	vddprintf(("\nAlgorithm Finished OK\n\n")) ;
		
	if (verbose) xyMappingEngine.PrintStats();
		
	/* ---- [[ Phase 3: ]] Peephole Optimization to Propagate Matchings ---- */
		
	// Due to complete re-writting, this phase has been disabled for the moment
		
	vddprintf(("\n+++ Phase 3: Peephole optimization to propagate matchings +++\n\n")) ;
	vddprintf(("\nskipping\n")) ;

	TRACE("PHASE 3: Optimize matchings");
	xyMappingEngine.Optimize( v0rootID );
	
	/* ---- [[ Phase 4: ]] Construct the Delta ---- */
		
	vddprintf(("\n+++ Phase 4: Construct the Delta +++\n\n")) ;
		
	TRACE("PHASE 4: Construct result delta");

	class DeltaConstructor myDeltaConstructor(&xyMappingEngine, doc1name, v0XML, doc2name, v1XML, ignoreSpacesFlag) ;
	myDeltaConstructor.constructDeltaDocument() ;
		
	TRACE("PHASE 4: Return result delta");

	return myDeltaConstructor.getDeltaDocument();
}




