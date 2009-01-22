//#define VERBOSE

#include "Diff_NodesManager.hpp"
#include "Diff_UniqueIdHandler.hpp"
#include "DeltaException.hpp"
#include "lookup2.hpp"
#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"


#include <stdio.h>
#include <map>
#include <queue>
#include <list>
#include <math.h>
#include <fstream>
#include "infra/general/hash_map.hpp"

#include <sys/timeb.h>
#include <sys/time.h>

#define MIN_CANDIDATEPARENT_LEVEL (3)

XERCES_CPP_NAMESPACE_USE

using namespace std;

AtomicInfo::AtomicInfo() {
	myID       = -1 ;
	myPosition = -1 ;
	myParent   = 0 ;
	firstChild = 0 ;
	nextSibling= 0 ;
	myEvent    = AtomicInfo::NOP ;
	myMatchID  = 0 ;
	myWeight   = -995000 ;
	hasIdAttr  = false ;
	isUnimportant=false ;
	}

unsigned long long int   clocksGetBestCandidate     ;
unsigned long long int   clocksQueueOps             ;
unsigned long long int   clocksPropagateAssign      ;

int statsNodeAlreadyAssigned       =0 ;
int statsRecursiveAssignFailed     =0 ;
int statsCantMatchDifferentOwnHash =0 ;

/*******************************************************************
 *                                                                 *
 * Manages nodes assignements v0 <-> v1 of the XML document        *
 *                                                                 *
 * Author: COBENA Gregory                                          *
 *                                                                 *
 *******************************************************************/

void NodesManager::PrintStats(void) {
	printf("NodesManager destructor\n");
	printf("Stats: NodeAlreadyAssigned       = %d\n", statsNodeAlreadyAssigned      );
	printf("Stats: RecursiveAssignFailed     = %d\n", statsRecursiveAssignFailed    );
	printf("Stats: CantMatchDifferentOwnHash = %d\n", statsCantMatchDifferentOwnHash);
	}

NodesManager::~NodesManager(void) {
	}
	
#define NULL_ID (0)

void NodesManager::PrintAll(void) {
	printf("XML document Source(v0): %d assigned nodes over total of %d\n", sourceAssigned, sourceNumberOfNodes) ;
	printf("XML document Source(v1): %d assigned nodes over total of %d\n", resultAssigned, resultNumberOfNodes) ;
	
	vddprintf(("List of v0->v1 assignements\n")) ;
	for(unsigned long i=1; i<sourceNumberOfNodes; i++) {
		if (!v0Assigned(i)) vddprintf(("%4d not matched(delete)\n", (int)i )) ;
		else vddprintf(("%4d->%4d\n", (int)i, (int) v0node[i].myMatchID ));
		}
		
	vddprintf(("List of v1->v0 assignements\n")) ;
	for(unsigned long i=1; i<resultNumberOfNodes; i++) {
	  if (!v1Assigned(i)) vddprintf(("%4d not matched(insert)", (int)i )) ;
		else vddprintf(("%4d->%4d\n", (int)i, (int) v1node[i].myMatchID ));
		}
  }
	
bool NodesManager::v0Assigned( int id ) {
  if ((id<1)||((unsigned int)id>sourceNumberOfNodes)) throw OutOfBound() ;
	return (v0node[id].myMatchID!=NULL_ID);
	}
bool NodesManager::v1Assigned( int id ) {
  if ((id<1)||((unsigned int)id>resultNumberOfNodes)) throw OutOfBound() ;
	return (v1node[id].myMatchID!=NULL_ID);
	}

void NodesManager::nodeAssign( int v0nodeID, int v1nodeID ) {
	vddprintf(("Matching old(%d) with new(%d)\n", v0nodeID, v1nodeID ));
	
	if (v0Assigned( v0nodeID ) || v1Assigned( v1nodeID )) {
		statsNodeAlreadyAssigned++ ;
		return ;
		}
		
	if (v0node[v0nodeID].myOwnHash.value==v1node[v1nodeID].myOwnHash.value) {
		v0node[ v0nodeID ].myMatchID = v1nodeID ;
		sourceAssigned++ ;
		v1node[ v1nodeID ].myMatchID = v0nodeID ;
		resultAssigned++ ;
		}
	else {
		statsCantMatchDifferentOwnHash++ ;
		}
	}

void NodesManager::recursiveAssign( int v0nodeID, int v1nodeID ) {

	if ((!v0nodeID)||(!v1nodeID)) {
		fprintf(stderr, "recursiveAssign::bad arguments (%d, %d)\n", v0nodeID, v1nodeID);
		return;
		}
	
	nodeAssign(v0nodeID, v1nodeID);

	int v0child = v0node[v0nodeID].firstChild ;
	int v1child = v1node[v1nodeID].firstChild ;
	
	while(v0child) {
		if (!v1child) {
			statsRecursiveAssignFailed++;
			fprintf(stderr, "recursiveAssign::expected child in v1 not found\n");
			return;
			}
		recursiveAssign(v0child, v1child);
		v0child = v0node[v0child].nextSibling ;
		v1child = v1node[v1child].nextSibling ;
		}
	if (v1child) {
		statsRecursiveAssignFailed++;
		fprintf(stderr, "recursiveAssign::expected child in v0 not found\n");
		return;
		}
  }

void NodesManager::forceParentsAssign( int v0nodeID, int v1nodeID, int level ) {

	if ((!v0nodeID)||(!v1nodeID)) {
		fprintf(stderr, "forceParentsAssign::bad arguments (%d, %d)\n", v0nodeID, v1nodeID);
		return;
		}

	int v0ascendant = v0nodeID ;
	int v1ascendant = v1nodeID ;

	for(int i=0; i<(level-1); i++) {
		v0ascendant = v0node[v0ascendant].myParent ;
		v1ascendant = v1node[v1ascendant].myParent ;

		if ((v0ascendant==0)||(v1ascendant==0)) return;
		
		if (v0Assigned( v0ascendant )) {
		  vddprintf(("forceParentsAssign stopped at level %d because v0 ascendant is already assigned\n",i ));
			return ;
			}
		if (v1Assigned( v1ascendant )) {
		  vddprintf(("forceParentsAssign stopped at level %d because v1 ascendant is already assigned\n",i ));
			return ;
			}
			
		if (v0node[v0ascendant].myOwnHash.value==v1node[v1ascendant].myOwnHash.value) {
		  nodeAssign( v0ascendant, v1ascendant );
			}
		else {
		  vddprintf(("forceParentsAssign stopped because relatives (%d, %d) do not have the same label\n", v0ascendant, v1ascendant )) ;
			return ;
			}
		}
  }

/*******************************************************************
 *                                                                 *
 * Registration of v0 and v1 nodes                                 *
 *                                                                 *
 * What we want is :                                               *
 * - a direct DiffID <-> DOM_Node mapping                          *
 * - a path from v1 DOM_Node to its hash value                     *
 * - a path from an hash value to v0 nodes with the same hash      *
 * - a notion of 'weight' for a subtree in v1                      *
 *                                                                 *
 *                                                                 *
 * Author: COBENA Gregory                                          *
 *******************************************************************/

NodesManager::NodesManager( void ) {
  sourceNumberOfNodes = 0 ;
	resultNumberOfNodes = 0 ;

	v0node.resize( 1 );
	v1node.resize( 1 );
	
	sourceAssigned = 0 ;
	resultAssigned = 0 ;
	
	myUniqueIdHandler = NULL ;
	}

void NodesManager::setUniqueIdHandler( class UniqueIdHandler *theUniqueIdHandler ) {
	myUniqueIdHandler = theUniqueIdHandler ;
}

void NodesManager::registerSourceDocument( XID_DOMDocument *sourceDoc ) {
	if (sourceNumberOfNodes) THROW_AWAY(("source document has already been registered"));
	DOMElement* v0root = sourceDoc->getDocumentElement() ;
	v0nodeByDID.resize( 1 ); // DiffID 0 is not allowed
	(void) registerSubtree((DOMNode*) v0root, true);
	vddprintf(("Source Document has %d nodes\n", (int)v0nodeByDID.size()-1));
	v0doc = sourceDoc ;
	vddprintf(("Register precomputed index to access candidates by their ancestors (up to level %d)\n", (int)MIN_CANDIDATEPARENT_LEVEL));
	listOfCandidatesByParentLevelByHash.resize(1+MIN_CANDIDATEPARENT_LEVEL);
	computeCandidateIndexTables(sourceNumberOfNodes);
	vddprintf(("index ok\n"));
}

void NodesManager::registerResultDocument( XID_DOMDocument *resultDoc ) {
	if (!sourceNumberOfNodes) THROW_AWAY(("source document must be registered before target document"));
	DOMElement* v1root = resultDoc->getDocumentElement() ;
	v1nodeByDID.resize( 1 ); // DiffID 0 is not allowed
	(void) registerSubtree((DOMNode*) v1root, false);
	vddprintf(("Result Document has %d nodes\n", (int)v1nodeByDID.size()-1));
	v1doc = resultDoc ;
}
	
int NodesManager::registerSubtree(DOMNode *node, bool isSource) {

	if (node==NULL) throw VersionManagerException("runtime-error", "NodesManager::registerSubtree", "node is NULL"); 

	class AtomicInfo myAtomicInfo ;
	myAtomicInfo.myWeight = 1.0 ;
	std::vector<int> childList ;
	
	switch( node->getNodeType() ) {
		case DOMNode::ELEMENT_NODE: {
			unsigned int attLength = node->getAttributes()->getLength();
			for(unsigned int i=0; ((i<attLength)&&(!myAtomicInfo.hasIdAttr)); i++) {
				DOMNode* attr = node->getAttributes()->item( i );
				std::string keyId = UniqueIdHandler::UniqueKey_from_TagAttr(node->getNodeName(),attr->getNodeName());
				if (myUniqueIdHandler->isIdAttr(keyId)) {
					vddprintf(("found unique key %s\n", keyId.c_str()));
					myAtomicInfo.hasIdAttr = true ;
					hash32 keyIdH(keyId.c_str());
					hash32 attrValH(attr->getNodeValue());
					myAtomicInfo.myOwnHash.value = (keyIdH.value<<1)+(attrValH.value<<2) ;
					}
				}

			if (!myAtomicInfo.hasIdAttr) {
				myAtomicInfo.myOwnHash = hash32(node->getNodeName());
				}
			myAtomicInfo.mySubtreeHash = myAtomicInfo.myOwnHash ;
			
			if (node->hasChildNodes()) {
				DOMNode* child = node->getFirstChild() ;
				while(child!=NULL) {
			  		int childID = registerSubtree(child, isSource);
					if (childID>0) {
						childList.push_back( childID );
						unsigned long buffer[2];
						buffer[0] = myAtomicInfo.mySubtreeHash.value;
						buffer[1] = (isSource)? v0node[childID].mySubtreeHash.value : v1node[childID].mySubtreeHash.value;
						hash32 subtreehash((unsigned char*)buffer, sizeof(buffer));
						myAtomicInfo.mySubtreeHash.value = subtreehash.value ;
						myAtomicInfo.myWeight += (isSource)? v0node[childID].myWeight : v1node[childID].myWeight ;
						child=child->getNextSibling();
						}
					else {
						fprintf(stderr, "XyDiff::registerSubtree() warning - a child node has been ignored\n");
						}
					}
				}
			}
			break ;
			
		case DOMNode::TEXT_NODE:
		case DOMNode::CDATA_SECTION_NODE:
                  myAtomicInfo.mySubtreeHash   = hash32(node->getNodeValue());
			myAtomicInfo.myOwnHash.value = 0x0 ;
			if (XMLString::stringLen(node->getNodeValue())>0) myAtomicInfo.myWeight+=(float)log((double)XMLString::stringLen(node->getNodeValue()));

			/* For text nodes containing only whitespaces and \13 chars, 
			 * set the flag 'isUnimportant'
			 */ 
			
			if (!XID_DOMDocument::isRealData(node)) myAtomicInfo.isUnimportant=true;
			
			break ;
			
		case DOMNode::COMMENT_NODE:
			/* Nota: Like other XML Diff tools, we ignore comment nodes */
			break ;
			
		case DOMNode::ENTITY_REFERENCE_NODE:
			cerr << "- Entity Found in the Document -" << endl ;
			throw VersionManagerException("runtime-error", "registerSubtree", "Entity not supported - Please apply Entities before the Diff"); 
		default: {
			char n[10];
			sprintf(n, "%d", node->getNodeType());
			std::string s = "Unknown node type "+std::string(n) ;
			XyLatinStr context(node->getNodeValue());
			fprintf(stderr, "Warning: ignoring unsupported node in registerSubtree() - NodeType=%d, NodeValue=%s\n", (int)node->getNodeType(), (const char*)context);
			return 0;
			//throw VersionManagerException("runtime-error", "registerSubtree", s); 
			}
		}

	if ( isSource ) myAtomicInfo.myID = ++sourceNumberOfNodes ;
	else            myAtomicInfo.myID = ++resultNumberOfNodes ;

	// Register Id Attr values

	if ((myAtomicInfo.hasIdAttr)&&(isSource)) {
		myUniqueIdHandler->v0nodeByIdAttrHash[myAtomicInfo.myOwnHash.value]=myAtomicInfo.myID ;
		}

	// Register list of children
	
	unsigned int nbChildren = childList.size() ;
	for(unsigned int i=0; i<nbChildren; i++) {
		class AtomicInfo *childAtomicInfo = (isSource) ? & v0node[childList[i]] : & v1node[childList[i]] ;
		childAtomicInfo->myParent         = myAtomicInfo.myID ;
		childAtomicInfo->myPosition       = (i+1) ;
		if (i<(nbChildren-1)) childAtomicInfo->nextSibling = childList[i+1] ;
		}
	if (nbChildren) myAtomicInfo.firstChild = childList[0] ;

	// Store node in array

	if ( isSource ) v0node.push_back( myAtomicInfo );
	else            v1node.push_back( myAtomicInfo );

	// Link node's ID to DOM_Node
	
	if ( isSource ) { // Source Document : v0
		v0nodeByDID.push_back( node );
		vddprintf(("v0: node %4d, SubtreeHash=0x%08x OwnHash=0x%08x\n", myAtomicInfo.myID, (unsigned int)myAtomicInfo.mySubtreeHash.value, (unsigned int)myAtomicInfo.myOwnHash.value ));
		}
	else { // Result Document : v1
		v1nodeByDID.push_back( node );
		vddprintf(("v1: node %4d, weight=%2.1f, SubtreeHash=0x%08x OwnHash=0x%08x\n", myAtomicInfo.myID, myAtomicInfo.myWeight, (unsigned int)myAtomicInfo.mySubtreeHash.value, (unsigned int)myAtomicInfo.myOwnHash.value ));
		}

	// Return value e.g. Postfix count
	
	return myAtomicInfo.myID ;
	}

void NodesManager::computeCandidateIndexTables(int v0nodeID) {
	class AtomicInfo & myAtomicInfo = v0node[ v0nodeID ] ;

	listOfCandidatesByParentLevelByHash[0][myAtomicInfo.mySubtreeHash.value].v0node.push_back(myAtomicInfo.myID);
		
	int relativeLevel=1;
	int relativeId=myAtomicInfo.myParent;
	while((relativeLevel<=MIN_CANDIDATEPARENT_LEVEL)&&(relativeId>0)) {
		unsigned long buf[2];
		buf[1]=relativeId;
		buf[0]=myAtomicInfo.mySubtreeHash.value;
		hash32 tablekey((unsigned char*)buf, 8);
		indexToCandidates &theIndex = listOfCandidatesByParentLevelByHash[relativeLevel]; 
		theIndex[tablekey.value].v0node.push_back(myAtomicInfo.myID);
		
		relativeLevel++;
		relativeId=v0node[relativeId].myParent;
		}
	
	int childID = myAtomicInfo.firstChild ;
	while(childID) {
		computeCandidateIndexTables( childID );
		childID = v0node[childID].nextSibling ;
		}

	}

/*******************************************************************
 *                                                                 *
 *                                                                 *
 * Top-Down matching algorithm                                     *
 *                                                                 *
 * Author: COBENA Gregory                                          *
 *                                                                 *
 * The algorithm relies on two features:                           *
 * 1) Order the nodes to match in top-down or weight-relatedorder  *
 * 2) Find the best node among a certain number of nodes with      *
 * the same content                                                *
 *                                                                 *
 *******************************************************************/

// From a number of old nodes that have the exact same signature, one has to choose which one
// will be considered 'matching' the new node
// Basically, the best is the old node somehow related to new node: parents are matching for example
// If none has this property, and if hash_matching is *meaningfull* ( text length > ??? ) we may consider returning any matching node
// Maybe on a second level parents ?

//int NodesManager::getBestCandidate(pair<matchingRangeIterator, matchingRangeIterator> nodeRange, int v1nodeID ) {
int NodesManager::getBestCandidate(int v1nodeID, unsigned long selfkey) {

	candidatesPointer fullListPointer=listOfCandidatesByParentLevelByHash[0].find(selfkey);
	if (fullListPointer==listOfCandidatesByParentLevelByHash[0].end()) return 0;
	if (fullListPointer->second.v0node.size()==0) return 0;

	// nodeRange.first==nodeRange.second) return 0;

	// first pass : finds a node which parent matches v1node parent (usefull because documents roots always match or parent may be matched thanks to its unique label)
	
	int candidateRelativeLevel = 1 ;
	int v1nodeRelative = v1nodeID ;
	
  /* The relative weight correspond to the ratio of the weight of the subtree over the weight of the entire document */
	int v1rootID = resultNumberOfNodes ;
	float relativeWeight = v1node[ v1nodeID ].myWeight / v1node[ v1rootID ].myWeight;
	int maxLevelPath = MIN_CANDIDATEPARENT_LEVEL + (int) (5.0*log((double)resultNumberOfNodes)*relativeWeight) ;
	
	/* Try to attach subtree to existing match among ancesters
	 * up to maximum level of ancester, depending on subtree weight
	 */
	
	vddprintf(("maxLevel=%d\n", maxLevelPath));
	while ( candidateRelativeLevel <= maxLevelPath ) {

		/* get info for ancester at corresponding level */
		
		vddprintf(("    pass parentLevel=%d ", candidateRelativeLevel)) ;
		fflush(stdout);
		v1nodeRelative = v1node[v1nodeRelative].myParent ;
		if (v1nodeRelative==0) {
			vddprintf(("but node doesn't not have ancesters up to this level\n")) ;
			return 0 ;
			}
		class AtomicInfo &v1nodeRelativeInfo=v1node[v1nodeRelative];
		if (v1nodeRelativeInfo.myMatchID<=0) {
			vddprintf(("but v1 relative at this level has no match\n"));
			}
		else {		
			
			/* For the lower levels, use precomputed index tables to acces candidates given the parent */
			
			if (candidateRelativeLevel<=MIN_CANDIDATEPARENT_LEVEL) {
				vddprintf(("using pre-computed index for relative level %d\n", candidateRelativeLevel));
				unsigned long buf[2];
				buf[1]=v1nodeRelativeInfo.myMatchID;
				buf[0]=selfkey;
				hash32 tablekey((unsigned char*)buf, 8);
				indexToCandidates &theIndex = listOfCandidatesByParentLevelByHash[candidateRelativeLevel]; 
				candidatesPointer theList = theIndex.find(tablekey.value);
				if (theList!=theIndex.end()) {
					for(unsigned long i=0; i<theList->second.v0node.size(); i++) {
						int c=theList->second.v0node[i];
						if (!v0Assigned(c)) {
							if (candidateRelativeLevel>1) {
						  	vddprintf(("    level>1 so forcing parents matching in the hierarchie\n")) ;
								forceParentsAssign(c, v1nodeID, candidateRelativeLevel );
								}
							return c;
							}
						}
					vddprintf(("but there is no available subtree with this content and this ancester\n"));
					}
				else {
					vddprintf(("but there is not subtree with this content and this ancester\n"));
					}
				}
			
			/* For higher levels, try every candidate and this if its ancestor is a match for us */

			else {
				candidatesPointer theList = listOfCandidatesByParentLevelByHash[0].find(selfkey);
				if (theList==listOfCandidatesByParentLevelByHash[0].end()) return 0;
				if (theList->second.v0node.size()>50) {
					printf("warning, it seems that there are too many candidates(%d)\n", theList->second.v0node.size());
					}
				
				for(unsigned long i=0; i<theList->second.v0node.size(); i++) {
					int candidate = theList->second.v0node[i];
					if ( !v0Assigned( candidate )) { // Node still not assigned
			  		vddprintf(("(%d)", candidate )) ;

					  // get its relative
						int candidateRelative = candidate ;
						int i ;
						for(i=0; i<candidateRelativeLevel; i++) {
				  		candidateRelative = v0node[candidateRelative].myParent ;
							if (candidateRelative==0) i=candidateRelativeLevel+1 ;
							}
				
						// if relative is ok at required level, test matching
						if (i==candidateRelativeLevel) {
							if (v0node[candidateRelative].myMatchID==v1nodeRelative) {
			      		vddprintf((" taken because some relatives ( level= %d ) are matching\n", candidateRelativeLevel ));
				    		if (candidateRelativeLevel>1) {
						  		vddprintf(("    level>1 so forcing parents matching in the hierarchie\n")) ;
						  		forceParentsAssign( candidate, v1nodeID, candidateRelativeLevel );
									}
								return candidate ;
				    		}
							}
							
			  		} // else: candidate was already assigned
					} //try next candidate
				} //end MIN(Precomputed)<relativelevel<MAX
						
			fflush(stdout);
		  } //end ancestor is matched
    candidateRelativeLevel++;
		} // next level

	return 0 ;
	} ;

/*******************************************************************
 *                                                                 *
 * Function Object that compare two nodes to decide which one      *
 * should be tried first.                                          *
 *                                                                 *
 * if F(x,y) is true, that means <y> will be placed first          *
 * (highest priority)                                              *
 * In our algorithm, the priority is given to the largest subtree  *
 * If weights are equal, priority is given to the lowest postfix   *
 * (e.g. postfix order)                                            *
 *                                                                 *
 *******************************************************************/

class cmpNodesOrder {
	public:
		cmpNodesOrder(class NodesManager *IncNodesManager) {
			myNodesManager = IncNodesManager;
			} ;
		bool operator() (int id1, int id2) const {
			if ( myNodesManager->v1node[ id1 ].myWeight == myNodesManager->v1node[ id2 ].myWeight ) {
				return ( id1 > id2 ) ;
				}
			return ( myNodesManager->v1node[ id1 ].myWeight < myNodesManager->v1node[ id2 ].myWeight );
			}
	private:
		class NodesManager *myNodesManager ;
	};

void NodesManager::topdownMatch( int v0rootID, int v1rootID ) {
	
#ifndef DISABLE_PRIORITY_FIFO
	std::priority_queue<int, std::vector<int>, cmpNodesOrder> toMatch(cmpNodesOrder(this)) ;
#else
	std::queue<int> toMatch ;
#endif

	toMatch.push( v1rootID );

	while( toMatch.size() > 0 ) {

		// get node to investigate
		unsigned long long int localStart = rdtsc() ;
#ifndef DISABLE_PRIORITY_FIFO	
		int nodeID = toMatch.top() ;
#else
		int nodeID = toMatch.front() ;
#endif
		toMatch.pop();
		clocksQueueOps += rdtsc() - localStart ;
		
		hash32 v1hash = v1node[nodeID].mySubtreeHash ;
	  vddprintf(("Trying new node %d, hash=%08x\n", nodeID, (unsigned int)v1hash.value ));

		int matcher = 0;

	  // consistency check: has it already been done ???
		
		if (v1Assigned( nodeID )) {
			vddprintf(( "skipping Full Subtree check because subtree node is already assigned.\n" )) ;
			}
		else {

	  	// Document roots *always* match
		// a 'renameRoot' operation will be added later if necessary
			if (nodeID==v1rootID) {
				v0node[ v0rootID ].myMatchID = v1rootID ;
				sourceAssigned++ ;
				v1node[ v1rootID ].myMatchID = v0rootID ;
				resultAssigned++ ;
			} else {
				unsigned long long int localStart = rdtsc() ;
				matcher = getBestCandidate(nodeID, v1hash.value);
				clocksGetBestCandidate += rdtsc() - localStart ;
				}
#if 0
				candidatesPointer i=listOfCandidatesByHash.find(v1hash.value);
				//if (i!=v0nodeByHash.end())
				if (i!=listOfCandidatesByHash.end()) {
		  		unsigned long long int localStart = rdtsc() ;
					//matcher = getBestCandidate( v0nodeByHash.equal_range( v1hash.value ), nodeID );
					matcher = getBestCandidate(i, nodeID, v1hash.value);
					clocksGetBestCandidate += rdtsc() - localStart ;
			  	}
#endif
			}
			
		if (matcher) {
		  unsigned long long int localStart = rdtsc() ;
			recursiveAssign( matcher, nodeID ) ;
			clocksPropagateAssign += rdtsc() - localStart ;
			}
			
		// If not found, children will have to be investigated

	  else {
	    // put children in the vector so they'll be taken care of later
	
	    vddprintf(( "Subtree rooted at (%d) not fully matched, programming children\n", nodeID ));
	
			int childID = v1node[nodeID].firstChild ;
			while(childID) {
				unsigned long long int localStart = rdtsc() ;
				toMatch.push( childID ) ;
				clocksQueueOps += rdtsc() - localStart ;
				childID = v1node[childID].nextSibling ;
				}
			}
			
	  // Next node to investigate
		}
	
	return ;
	}

/*******************************************************************
 *                                                                 *
 * Match by ID                                                     *
 *                                                                 *
 * This is first step: use identification of DTD to match nodes    *
 *                                                                 *
 *******************************************************************/

int NodesManager::FullBottomUp(int v1nodeID) {
	
	vddprintf(("fullBottomUp call on v1 node %d\n", v1nodeID));

	class AtomicInfo & myV1AtomicInfo = v1node[v1nodeID];
	std::map<int, float> weightByCandidate ;
	
	// Apply to children

	int childID = myV1AtomicInfo.firstChild ;
	while(childID) {
		class AtomicInfo &childInfo = v1node[childID];
		int childMatch = FullBottomUp( childID );
		if (childMatch) {
			class AtomicInfo & childMatchInfo = v0node[childMatch];
			int v0childParent = childMatchInfo.myParent ;
			if (v0childParent) {
				if (weightByCandidate.find(v0childParent)==weightByCandidate.end()) weightByCandidate[v0childParent]=childInfo.myWeight;
				else weightByCandidate[v0childParent]+=childInfo.myWeight;
				}
			}
		childID = childInfo.nextSibling ;
		}
	
	// Do self
	
	if (myV1AtomicInfo.myMatchID) {
		vddprintf(("v1 node %d already has a match, returning %d\n", v1nodeID, myV1AtomicInfo.myMatchID));
		return myV1AtomicInfo.myMatchID ;
		}
	if (weightByCandidate.begin()==weightByCandidate.end()) {
		vddprintf(("v1 node %d has no matched children, returning match %d\n", v1nodeID, myV1AtomicInfo.myMatchID));
		return myV1AtomicInfo.myMatchID ;
		}
	
		
	// Find parent corresponding to largest part of children
	vddprintf(("v0 parents of v0 nodes matching v1 children of v1 node %d are:\n", v1nodeID));
	float max=-1.0;
	int bestMatch=0;
	for(std::map<int, float>::iterator i=weightByCandidate.begin(); i!=weightByCandidate.end(); i++) {
		vddprintf(("v0 node %d with total weight among children of %.1f\n", i->first, i->second));
		if (i->second>max) {
			bestMatch = i->first ;
			max = i->second ;
			}
		}
	
	vddprintf(("best parent is v0 node %d with total weight among children of %.1f\n", bestMatch, max));
	
	nodeAssign(bestMatch, v1nodeID);
		
	vddprintf(("returning match %d\n", myV1AtomicInfo.myMatchID));
	return myV1AtomicInfo.myMatchID ;
	}

void NodesManager::MatchById( int v1nodeID ) {
	class AtomicInfo & myV1AtomicInfo = v1node[ v1nodeID ] ;

	// Test
	
	if (myV1AtomicInfo.hasIdAttr) {
		if (myUniqueIdHandler->v0nodeByIdAttrHash.find(myV1AtomicInfo.myOwnHash.value)!=myUniqueIdHandler->v0nodeByIdAttrHash.end()) {
			int v0nodeID = myUniqueIdHandler->v0nodeByIdAttrHash[myV1AtomicInfo.myOwnHash.value] ;
			vddprintf(("v1 node %d has an IdAttribute and we found corresponding node %d in v0\n", v1nodeID, v0nodeID));
			nodeAssign(v0nodeID, v1nodeID);
			}
		}

	// Apply recursivly on children

	int childID = myV1AtomicInfo.firstChild ;
	while(childID) {
		MatchById( childID );
		childID = v1node[childID].nextSibling ;
		}
	}

/*******************************************************************
 *                                                                 *
 *                                                                 *
 * Optimization Phase                                              *
 *                                                                 *
 * The goal of this phase is to use matchings, and if they give us *
 * obvious results for children                                    *
 *                                                                 *
 *******************************************************************/

void NodesManager::Optimize(int v0nodeID) {
	
	class AtomicInfo & myAtomicInfo = v0node[ v0nodeID ] ;

	// If node is matched, we can try to do some work
	
	// Get Free nodes in v0
	
	std::map<std::string, int> v0freeChildren ;
	if (v0Assigned( v0nodeID )) {
		// printf("For parent v0(%d)\n", v0nodeID);
		int childID = myAtomicInfo.firstChild ;
		while(childID) {
			if (!v0Assigned(childID)) {
				DOMNode* child = v0nodeByDID[ childID ] ;
				if (child->getNodeType()==DOMNode::ELEMENT_NODE) {
					XyUTF8Str t(child->getNodeName());
					std::string tag = (const char*)t;
					// printf("Children <%s> is free!\n", tag.c_str());
					if (v0freeChildren.find(tag)!=v0freeChildren.end()) {
						v0freeChildren[tag]=-1 ;
						// printf("but many have the same name...\n");
						}
					else v0freeChildren[tag]=childID ;
					}
				}
			childID = v0node[childID].nextSibling ;
			}

		// Look for similar nodes in v1

		int v1nodeID = myAtomicInfo.myMatchID;
		class AtomicInfo & v1AtomicInfo = v1node[ v1nodeID ];
		
		std::map<std::string, int> v1freeChildren ;
		if (v1Assigned( v1nodeID )) {
			// printf("For parent v1(%d)\n", v1nodeID);
			int childID = v1AtomicInfo.firstChild ;
			while(childID) {
				if (!v1Assigned(childID)) {
					DOMNode* child = v1nodeByDID[ childID ] ;
					if (child->getNodeType()==DOMNode::ELEMENT_NODE) {
						XyUTF8Str t(child->getNodeName());
						std::string tag = (const char*)t;
						// printf("v1 children <%s> is free!\n", tag.c_str());
						if (v1freeChildren.find(tag)!=v1freeChildren.end()) {
							v1freeChildren[tag]=-1 ;
							// printf("but many have the same name...\n");
							}
						else v1freeChildren[tag]=childID ;
						}
					}
				childID = v1node[childID].nextSibling ;
				}
			}

		// Now match unique children
		std::map<std::string, int>::iterator i ;
		for(i=v0freeChildren.begin(); i!=v0freeChildren.end(); i++) {
			if ((i->second>0)&&(v1freeChildren.find(i->first)!=v1freeChildren.end())) {
				int v1ID = v1freeChildren[i->first];
				if (v1ID>0) {
					vddprintf(("matching v0(%d) with v1(%d)\n", i->second, v1ID));
					nodeAssign(i->second, v1ID);
					}
				}
			}

		// End-if - Assigned(v0nodeID)
		}
	
	// Apply recursivly on children

	int childID = myAtomicInfo.firstChild ;
	while(childID) {
		Optimize( childID );
		childID = v0node[childID].nextSibling ;
		}

	}


/*******************************************************************
 *                                                                 *
 *                                                                 *
 * Create the Script that constructs v1 from v0.                   *
 *                                                                 *
 * This Script will then be used to create the XML-delta document  *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 *******************************************************************/

/* ---- Make script for DELETE operations ---- */

void NodesManager::MarkOldTree( const int v0nodeID ) {

	class AtomicInfo & myAtomicInfo = v0node[ v0nodeID ] ;
	
	// Apply to Children

	int childID = myAtomicInfo.firstChild ;
	while(childID) {
		MarkOldTree( childID );
		childID = v0node[childID].nextSibling ;
		}

	// Test if node is DELETED
	
	if (!v0Assigned( v0nodeID )) {
		vddprintf(("delete node %d (child pos %d for parent %d)\n", myAtomicInfo.myID, myAtomicInfo.myPosition, myAtomicInfo.myParent));
		myAtomicInfo.myEvent = AtomicInfo::DELETED ;
		}
		
	else if (myAtomicInfo.myParent) { // e.g. if not ROOT
	
		// Test if node is a STRONG MOVE
		
		if ( (!v0Assigned( myAtomicInfo.myParent )) || (v0node[ myAtomicInfo.myParent ].myMatchID != v1node[myAtomicInfo.myMatchID].myParent )) {
			vddprintf(("strong move (from) node %d (child pos %d for parent %d)\n", myAtomicInfo.myID, myAtomicInfo.myPosition, myAtomicInfo.myParent));
			myAtomicInfo.myEvent = AtomicInfo::STRONGMOVE ;
			}
		}
		
	vddprintf(("nothing for node %d (child pos %d for parent %d)\n", myAtomicInfo.myID, myAtomicInfo.myPosition, myAtomicInfo.myParent));
	}

void NodesManager::MarkNewTree( const int v1nodeID ) {

	class AtomicInfo & myAtomicInfo = v1node[ v1nodeID ] ;
	
	int childID = myAtomicInfo.firstChild ;
	while(childID) {
		MarkNewTree( childID );
		childID = v1node[childID].nextSibling ;
		}

	// Test if node is assigned
	
	if ( v1Assigned( v1nodeID )) {

		// Get corresponding XID

		int v0nodeID =myAtomicInfo.myMatchID ;
		
		// Set --- XID ---
		DOMNode *oldnode = v0nodeByDID[ v0nodeID ] ;
		DOMNode *node = v1nodeByDID[ v1nodeID ] ;

		v1doc->getXidMap().registerNode( node, v0doc->getXidMap().getXIDbyNode( oldnode ) );
		
		vddprintf(( "getting xid=%d for node %d\n",(int)v0doc->getXidMap().getXIDbyNode( oldnode ), v0nodeID )) ;
	
		// Test if it is a STRONGMOVE
	
		if (myAtomicInfo.myParent) { //e.g. not root
			if (  (!v1Assigned( myAtomicInfo.myParent )) || (v1node[ myAtomicInfo.myParent ].myMatchID != v0node[myAtomicInfo.myMatchID].myParent )) {
				vddprintf(("strong move (to) node %d (child pos %d for parent %d)\n", myAtomicInfo.myID, myAtomicInfo.myPosition, myAtomicInfo.myParent));
				myAtomicInfo.myEvent = AtomicInfo::STRONGMOVE ;
				}
			}
		
		}
	else { // Node is INSERTED
		myAtomicInfo.myEvent = AtomicInfo::INSERTED ;
		DOMNode* node = v1nodeByDID[ v1nodeID ] ;
		v1doc->getXidMap().registerNode( node, v0doc->getXidMap().allocateNewXID() ); // WORK HERE
		vddprintf(("insert node %d with xid=%d at pos %d for parent %d\n", myAtomicInfo.myID, (int)v1doc->getXidMap().getXIDbyNode( node ), myAtomicInfo.myPosition, myAtomicInfo.myParent )) ;
		}

	}

/***********************************************
 *                                             *
 *         ++++ Compute Weak Move ++++         *
 *                                             *
 ***********************************************/

void NodesManager::ComputeWeakMove( int v0nodeID ) {
	
	class AtomicInfo & myAtomicInfo = v0node[ v0nodeID ] ;
	
	// Apply to Children
	
	int v0childID = myAtomicInfo.firstChild ;
	while(v0childID) {
		ComputeWeakMove( v0childID );
		v0childID = v0node[v0childID].nextSibling ;
		}
		
	if (!myAtomicInfo.firstChild) {
		vddprintf(("old node %d has no children, skipping\n", v0nodeID ));
		return ;
		}
		
	if (!v0Assigned( v0nodeID )) {
		vddprintf(("old node %d not assigned, skipping\n", v0nodeID ));
		return ;
		}
	
	// Apply to Self
	
	int v1nodeID = myAtomicInfo.myMatchID ;

	// Set Index to children of v0 node that are remaining on this node
	// Set 0 for others
	
	std::vector<wSequence> oldChildValue ;
	oldChildValue.push_back(wSequence(-1, 999000.0)) ;
	unsigned int index=1 ;
	v0childID = myAtomicInfo.firstChild ;
	while(v0childID) {
		class AtomicInfo & childInfo = v0node[v0childID] ;
		if (childInfo.myEvent==AtomicInfo::NOP) oldChildValue.push_back( wSequence(index++, childInfo.myWeight) ) ;
		else oldChildValue.push_back( wSequence(0, 998000.0) ) ;
		v0childID = childInfo.nextSibling ;
		}
	vddprintf(("Index for children of v0 node %d are listed here(0: not concerned by weak-move): ", v0nodeID ));
	for(unsigned int i=1; i<oldChildValue.size(); i++) vddprintf(("(%d, %.2f)  ", oldChildValue[i].data, oldChildValue[i].weight));
	vddprintf(("\n"));

	std::vector<wSequence> originalSequence ;
	originalSequence.push_back(wSequence(-1, 997000.0));
	for(unsigned int i=1; i<oldChildValue.size(); i++) if (oldChildValue[i].data) originalSequence.push_back( oldChildValue[i] );
	
	// Construct Sequence with new orders of children, given their 'stable index'
	
	std::vector<wSequence> finalSequence ;
	finalSequence.push_back(wSequence(-1, 996000.0));
	int v1childID = v1node[v1nodeID].firstChild ;
	while(v1childID) {
		class AtomicInfo & childInfo = v1node[v1childID] ;
		if (childInfo.myEvent==AtomicInfo::NOP) {
			int originPos = v0node[ childInfo.myMatchID ].myPosition;
			finalSequence.push_back( wSequence(oldChildValue[originPos].data, childInfo.myWeight) );
			}
		v1childID = childInfo.nextSibling ;
		}
		
	vddprintf(("final sequence is: "));
	for(unsigned int i=1; i<finalSequence.size(); i++) vddprintf(("%d ", finalSequence[i].data));
	vddprintf(("\n"));

	// Resolution : may be replaced by Longest Common Subsequence algorithms
	// Find move operations converting originalSequence into finalSequence

#if 1
	if ( originalSequence.size() < 100 ) {
		vddprintf(("using lcss algorithm...\n"));
		lcss( originalSequence, finalSequence );
		}
	else {
		vddprintf(("sequence too long, using quick subsequence finder.\n"));
		easy_css( originalSequence, finalSequence ) ;
		}
#else
		easy_css( originalSequence, finalSequence ) ;
#endif
	
	vddprintf(("using common subsequence: "));
	for(unsigned int i=1; i<finalSequence.size(); i++) if (finalSequence[i].data) vddprintf(("%d ", finalSequence[i].data));
	vddprintf(("\n"));
	
	// Children that have been marked with 0 will be 'moved'

	index=1 ;
	v0childID=myAtomicInfo.firstChild ;
	while(v0childID) {
		if (v0node[v0childID].myEvent==AtomicInfo::NOP) {
			if (originalSequence[index].data==0) {
				vddprintf(("  v0 node %d is moved to another position\n", v0childID ));
				v0node[ v0childID ].myEvent = AtomicInfo::WEAKMOVE ;
				v1node[ v0node[v0childID].myMatchID ].myEvent = AtomicInfo::WEAKMOVE;
				}
			index++ ;
			}
		v0childID = v0node[v0childID].nextSibling ;
		}
	}


/******************************************************
 *                                                    *
 *         ++++ Detect Update Operations ++++         *
 *                                                    *
 ******************************************************/

// This is another specific case:
// If children is a single text node, and is not assigned, then consider its value has been 'updated'
	
// Node that even if a text node is updated, a new XID will be allocated
// for the entry corresponding to the 'insert' part of 'UPDATE'
// The reason for this is that the minimal requirement for an XID
// is that the value of the root node of the subtree identified by the XID
// is always the same

void NodesManager::DetectUpdate(int v0nodeID) {
	
	class AtomicInfo & myAtomicInfo = v0node[ v0nodeID ] ;

	if (v0Assigned(v0nodeID)) {
		int v1nodeID = myAtomicInfo.myMatchID;
		class AtomicInfo & v1AtomicInfo = v1node[ v1nodeID ];
		
		int child0 = myAtomicInfo.firstChild ;
		class AtomicInfo & myChild0Info = v0node[ child0 ] ;
		if ((child0)&&(!v0Assigned(child0))&&(myChild0Info.nextSibling==0)) {
			int child1 = v1AtomicInfo.firstChild ;
			class AtomicInfo & myChild1Info = v1node[ child1 ] ;
			if ((child1)&&(!v1Assigned(child1))&&(myChild1Info.nextSibling==0)) {
				DOMNode* domChild0 = v0nodeByDID[ child0 ] ;
				DOMNode* domChild1 = v1nodeByDID[ child1 ] ;
				if ((domChild0->getNodeType()==DOMNode::TEXT_NODE)&&(domChild1->getNodeType()==DOMNode::TEXT_NODE)) {
					if ((myChild0Info.myEvent==AtomicInfo::DELETED)&&(myChild1Info.myEvent==AtomicInfo::INSERTED)) {
						// ---
						myChild0Info.myEvent = AtomicInfo::UPDATE_OLD ;
						myChild1Info.myEvent = AtomicInfo::UPDATE_NEW ;
						// ---
						}
					else throw VersionManagerException("Internal Error", "Optimize()", "Update handler state is	inconsistant");
					}
				}
			}
		}	

	// Apply recursivly on children

	int childID = myAtomicInfo.firstChild ;
	while(childID) {
		DetectUpdate( childID );
		childID = v0node[childID].nextSibling ;
		}

	}
