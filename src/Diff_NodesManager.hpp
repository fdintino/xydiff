#ifndef DIFF_NODESMANAGER_HXX
#define DIFF_NODESMANAGER_HXX

#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "lookup2.hpp"
#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"


#include <stdio.h>
#include <map>
#include <math.h>
#include <fstream>

#include <sys/timeb.h>
#include <time.h>


/**** HEADERS ****/

class OutOfBound {
	public:
		OutOfBound(void) {
			std::cerr << "ERROR: Out of bound" << std::endl ;
		}
	} ;

class AtomicInfo {
	public:
		typedef enum { NOP, DELETED, INSERTED, STRONGMOVE, WEAKMOVE, UPDATE_OLD, UPDATE_NEW } NodeEvent_t ;
		AtomicInfo(void) ;
		
		int         myID ;
		int         myPosition ;
		int         myParent ;
		int         firstChild ;
		int         nextSibling ;
		
		NodeEvent_t myEvent ;
		int         myMatchID ;
		
		float       myWeight ;
		hash32      mySubtreeHash ; // identify the subtree rooted at node
		hash32      myOwnHash ;
			// identify node's label.
			// *must* be equal to match any pair of nodes
			// NULL if text node -> can match any text nodes
			// = hash(label, keyattrname, keyattrvalue) if ID attribute is here
			// So if KeyAttr is here, matching is either mandatory or forbidden
	
		bool        hasIdAttr ;
		bool        isUnimportant ;
	} ;

// Note that push_back must be used to add candidates (in postfix order)
// Thus the ordering of children will be somehow taken into consideration during lookup for the best candidate

class CandidatesSet {
	public:
		std::vector<int> v0node;
	};

typedef std::map<unsigned long, CandidatesSet> indexToCandidates ;
typedef indexToCandidates::iterator candidatesPointer ;

//typedef multimap<unsigned long, int>::const_iterator matchingRangeIterator ;
class UniqueIdHandler ;

class NodesManager {
  public:
		NodesManager(void);
		~NodesManager(void);
		void registerSourceDocument( XID_DOMDocument *sourceDoc );
		void registerResultDocument( XID_DOMDocument *resultDoc );
		void setUniqueIdHandler( class UniqueIdHandler *theUniqueIdHandler );
	
	  unsigned int sourceNumberOfNodes ;
		unsigned int resultNumberOfNodes ;

		std::vector<xercesc::DOMNode*> v0nodeByDID ;
		std::vector<xercesc::DOMNode*> v1nodeByDID ;
		//multimap<unsigned long, int> v0nodeByHash ;
		std::vector<indexToCandidates> listOfCandidatesByParentLevelByHash;
		
	  XID_DOMDocument *v0doc, *v1doc ;
		
		std::vector<AtomicInfo> v0node ;
		std::vector<AtomicInfo> v1node ;

		void PrintStats(void);
		
	private:
		int registerSubtree(xercesc::DOMNode *node, bool isSource);
		void computeCandidateIndexTables(int v0nodeID);
		class UniqueIdHandler *myUniqueIdHandler;

	public:

		//int getBestCandidate(pair<matchingRangeIterator, matchingRangeIterator> nodeRange, int v1nodeID ) ;
		int getBestCandidate(int v1nodeID, unsigned long selfkey) ;

		void MatchById( int v1nodeID );
		void topdownMatch( int v0rootID, int v1rootID ) ;

		int FullBottomUp(int v1nodeID);
		void Optimize(int v0nodeID) ;

	  bool v0Assigned( int id );
		bool v1Assigned( int id );

		void nodeAssign( int v0nodeID, int v1nodeID ) ;
		void recursiveAssign( int v0nodeID, int v1nodeID );
		void forceParentsAssign( int v0nodeID, int v1nodeID, int level );

	  void PrintAll(void);

		unsigned int sourceAssigned, resultAssigned ;

		void MarkOldTree( const int v0nodeID ) ;
		void MarkNewTree( const int v1nodeID ) ;
		void ComputeWeakMove( int v0nodeID ) ;
		void DetectUpdate(int v0nodeID) ;

	  bool operator() (int id1, int id2) const ;
	} ;


#endif
