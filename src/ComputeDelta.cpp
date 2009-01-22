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

#include <sys/timeb.h>
#include <sys/time.h>

//class DTDValidator;

/*******************************************************************
 *                                                                 *
 *                                                                 *
 * Time Profiling Functions                                        *
 *                                                                 *
 * Author: Gregory COBENA                                          *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 *******************************************************************/
 
unsigned long long int clocksComputeDelta           ;

unsigned long long int clocksReadDocuments          ;
unsigned long long int clocksRegisterSubtree        ;
unsigned long long int clocksTopDownMatch           ;
unsigned long long int clocksOptimize               ;
unsigned long long int clocksConstructDeltaDocument ;
unsigned long long int clocksSaveDelta              ;

/* 1 to 10 ms resolution functions */

// We can use: getitimer, setitimer

struct timeb timeStart ;
struct timeb timeLast ;

void TimeInit(void) {
  vddprintf(("TimeInit\n")) ;
	ftime ( &timeStart );
	timeLast = timeStart ;
	}

void TimeStep(void) {
  struct timeb timeEnd ;
	ftime( &timeEnd );
	int secLast = (int) (timeEnd.time-timeLast.time) ;
	long mLast = timeEnd.millitm - timeLast.millitm ;
	mLast += 1000* secLast ;
	int secStart = (int) (timeEnd.time-timeStart.time) ;
	long mStart = timeEnd.millitm - timeStart.millitm ;
	mStart += 1000* secStart ;
	
	vddprintf(("<!> time since previous step=%ldms, since timer init=%ldms\n", mLast, mStart));
	timeLast = timeEnd ;
	}

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
XID_DOMDocument* XidXyDiff(XID_DOMDocument* v0XML, const char *doc1name, XID_DOMDocument* v1XML, const char *doc2name, bool ignoreSpacesFlag=false, bool verbose=false, xercesc_3_0::DTDValidator *dtdValidator=NULL) {
#ifdef HW_PROF
	clocksRegisterSubtree        = 0 ;
	clocksTopDownMatch           = 0 ;
		clocksGetBestCandidate     = 0 ;
		clocksQueueOps             = 0 ;
		clocksPropagateAssign      = 0 ;
	clocksOptimize               = 0 ;
	clocksConstructDeltaDocument = 0 ;
		clocksCDDinitDomDelta      = 0 ;
		clocksCDDprepareVectors    = 0 ;
		clocksCDDsaveXidMap        = 0 ;
		clocksCDDcomputeWeakMove   = 0 ;
		clocksCDDdetectUpdate      = 0 ;
		clocksCDDaddAttributeOps   = 0 ;
		clocksConstructDOMDelta    = 0 ;
#endif

	/* ---- [[ Phase 1: ]] Compute signatures and weight for subtrees on both documents ---- */
	
	TRACE("PHASE 1: Register ID-Attributes");
	
	UniqueIdHandler myUniqueIdHandler ;
	if (dtdValidator!=NULL) {
		myUniqueIdHandler.RegisterAttrId( dtdValidator );
		}
		
	// Create object that manages meta-data for nodes of both documents
	class NodesManager xyMappingEngine;
		
	vddprintf(("\n+++ Phase 1: Compute signature and weight for subtrees on both documents +++\n\n")) ;
	unsigned long long start = rdtsc() ;
		
	xyMappingEngine.setUniqueIdHandler( &myUniqueIdHandler );
	TRACE("PHASE 1: Register version 1");
	xyMappingEngine.registerSourceDocument( v0XML );
	TRACE("PHASE 1: Register version 2");
	xyMappingEngine.registerResultDocument( v1XML );
		
	int v0rootID = xyMappingEngine.sourceNumberOfNodes ;
	int v1rootID = xyMappingEngine.resultNumberOfNodes ;
		
	clocksRegisterSubtree += rdtsc() - start ;
	TimeStep();
	
	/* ---- [[ Phase 2: ]] Apply Bottom-Up Lazy-Down Algorithm ---- */
		
	vddprintf(("\n+++ Phase 2: Apply BULD (Bottom-Up Lazy Down) matching algorithm starting from heaviest subtrees +++\n\n")) ;
	start = rdtsc() ;
		
	TRACE("PHASE 2: Match by ID-Attributes");
	vddprintf(("Match node which have ID attributes\n")) ;
	xyMappingEngine.MatchById( v1rootID );

	TRACE("PHASE 2: Full Bottom-up propagation pass #1");
	vddprintf(("Full Bottom-up propagation pass #1\n")) ;
	xyMappingEngine.FullBottomUp( v1rootID );

	TRACE("PHASE 2: Apply BULD Algorithm");
	vddprintf(("Top-down matching on new document\n")) ;
	xyMappingEngine.topdownMatch( v0rootID, v1rootID );
		
	clocksTopDownMatch += rdtsc() - start ;
	vddprintf(("\nAlgorithm Finished OK\n\n")) ;
		
	if (verbose) xyMappingEngine.PrintStats();
		
	/* ---- [[ Phase 3: ]] Peephole Optimization to Propagate Matchings ---- */
		
	// Due to complete re-writting, this phase has been disabled for the moment
		
	vddprintf(("\n+++ Phase 3: Peephole optimization to propagate matchings +++\n\n")) ;
	vddprintf(("\nskipping\n")) ;

	TRACE("PHASE 3: Optimize matchings");
	start = rdtsc() ;
	xyMappingEngine.Optimize( v0rootID );
	clocksOptimize += rdtsc() - start ;
	
#if 0
	TimeStep();
	vddprintf(("\n+++ Print Raw Assigns +++\n\n")) ;
	xyMappingEngine.PrintAll();
#endif

	/* ---- [[ Phase 4: ]] Construct the Delta ---- */
		
	TimeStep();
	vddprintf(("\n+++ Phase 4: Construct the Delta +++\n\n")) ;
	start = rdtsc() ;
		
	TRACE("PHASE 4: Construct result delta");

	class DeltaConstructor myDeltaConstructor(&xyMappingEngine, doc1name, v0XML, doc2name, v1XML, ignoreSpacesFlag) ;
	myDeltaConstructor.constructDeltaDocument() ;
		
	clocksConstructDeltaDocument += rdtsc() - start ;

	TRACE("PHASE 4: Return result delta");

	return myDeltaConstructor.getDeltaDocument();
}




