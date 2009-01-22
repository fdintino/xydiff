/*---------------------------------------------------------------------------
 *
 * Simulates changes and diff on an XML page
 *
 * Author: Gregory Cobena
 *
 *----------------------------------------------------------------------------
 */

// ---------------------------------------------------------------------------
//  Includes and Defines
// ---------------------------------------------------------------------------

#include "include/XyLatinStr.hpp"

#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"
#include "Tools.hpp"

#include "DeltaException.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMText.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/util/XMLString.hpp"

#include <stdio.h>
#include <fstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

/* Globals */

int sequenceSize ;
double deleteProb ;
double updateProb ;
double insertProb ;
std::string xmlfilespattern;
std::string deltafilespattern;
std::string initfile;	
bool startFlag=false;

double frand(void);

// This integer are incremented whenever the corresponding operations is used
// Their value is use to create original text content (e.g. %d ) for 'update' and 'insert' operations

long deleted    = 0;
long updated    = 0;
long inserted   = 0;
long moved      = 0;

class ChangeSimulator {
  public:
		ChangeSimulator(XID_DOMDocument *sourceDoc,
		                const char *sourceFilename,
										const char *targetFilename,
										double _deleteProb,
										double _updateProb,
										double _insertProb);
	
		void registerSubTree(   DOMNode *root ) ;
		void unregisterSubTree( DOMNode *root ) ;
	
	  void DELETE(DOMNode *node);
	  void UPDATE(DOMNode *node);
	  void INSERT(DOMNode *node);
		
		std::vector<DOMNode*> elementNodes ;
		std::vector<DOMNode*> textNodes ;
		
		XID_DOMDocument* resultingDocument ;
		XID_DOMDocument* deletedData ;
		XID_DOMDocument* deltaDoc ;
		
		double deleteProb;
		double updateProb;
		double insertProb;
		
	private:
		DOMElement* deltaElement ;
	} ;

/*
 *     -+-+-+-+- add / remove Nodes from Simulator Tables -+-+-+-+-
 *
 */

ChangeSimulator::ChangeSimulator(XID_DOMDocument *sourceDoc,
		                const char *sourceFilename,
										const char *targetFilename,
										double _deleteProb,
										double _updateProb,
										double _insertProb) {
	
	deleteProb = _deleteProb;
	updateProb = _updateProb;
	insertProb = _insertProb;
 
	std::cout << "Work process uses a copy of the original document" << std::endl ;

	resultingDocument = XID_DOMDocument::copy( sourceDoc ) ;

	std::cout << "Creating Document for deleted nodes" << std::endl ;

	deletedData = XID_DOMDocument::createDocument() ;
	DOMElement* deletedDataElement = deletedData->createElement(XMLString::transcode("deletedNodesRoot"));
	//DOMNode* deletedDataRoot       = 
        deletedData->appendChild( deletedDataElement );
	deletedData->initEmptyXidmapStartingAt(-1);
		
	std::cout << "Creating Delta Document" << std::endl ;

	deltaDoc         = XID_DOMDocument::createDocument() ;
	DOMElement* elem = deltaDoc->createElement(XMLString::transcode("unit_delta"));
	deltaDoc->appendChild( elem );
	deltaElement     = deltaDoc->createElement(XMLString::transcode("t"));
	deltaElement->setAttribute(XMLString::transcode("from"), XyLatinStr(sourceFilename));
	deltaElement->setAttribute(XMLString::transcode("to"), XyLatinStr(targetFilename));
	deltaDoc->getDocumentElement()->appendChild(deltaElement);

	std::cout << "Register document nodes as candidate for changes" << std::endl ;

	DOMNode* docRoot = resultingDocument->getDocumentElement();
	if (docRoot==NULL) THROW_AWAY(("source document is empty"));
	registerSubTree( docRoot ) ;
	}

void ChangeSimulator::registerSubTree( DOMNode *node ) {

  if (node->hasChildNodes()) {

    DOMNode* child = node->getFirstChild();
		while( child != NULL ) {
			registerSubTree( child ) ;
			child = child->getNextSibling() ;
			}
    }
	
	if  (node->getNodeType()==DOMNode::ELEMENT_NODE) elementNodes.push_back(node);
	else if(node->getNodeType()==DOMNode::TEXT_NODE) textNodes.push_back(node);
	}
	

std::vector<DOMNode*> &erase(std::vector<DOMNode*> &t, DOMNode *node) {
	unsigned long count=0;
	for(std::vector<DOMNode*>::iterator i=t.begin(); i!=t.end();) {
		if (*i==node) {
			i=t.erase(i);
			count++;
			if (count>1) THROW_AWAY(("node already found in table"));
			}
		else i++;
		}
	if (count<1) THROW_AWAY(("node not found in table"));
	return t;
	}

void ChangeSimulator::unregisterSubTree( DOMNode *node ) {

  if (node->hasChildNodes()) {

    DOMNode *child = node->getFirstChild();
		while( child != NULL ) {
			unregisterSubTree( child ) ;
			child = child->getNextSibling() ;
			}
    }
	
	if  (node->getNodeType()==DOMNode::ELEMENT_NODE) {
		elementNodes=erase(elementNodes, node);
		}
  else if(node->getNodeType()==DOMNode::TEXT_NODE) {
		textNodes=erase(textNodes, node);
		}

  }


/********************************
 *                              *
 *  Create a DELETE Operation   *
 *                              *
 ********************************/
		

void ChangeSimulator::DELETE( DOMNode *node ) {
	XID_t nodeXID = resultingDocument->getXidMap().getXIDbyNode( node );

  std::cout << "...delete() xid=" << nodeXID << std::endl ;

	DOMNode* parent = node->getParentNode();
	
	// Can't have two TEXT brothers - So delete one of them
	
	DOMNode* next = node->getNextSibling() ;
	DOMNode* previous = node->getPreviousSibling() ;
	if ((next!=NULL)&&(previous!=NULL)) {
		if ((next->getNodeType()==DOMNode::TEXT_NODE)&&(previous->getNodeType()==DOMNode::TEXT_NODE)) {
		  DELETE( previous );
			}
	  }
	
	// Copy of deleted node in the Garbage
	
	DOMNode* deletedNode = deletedData->importNode( node, true );
	deletedData->getXidMap().mapSubtree( resultingDocument->getXidMap().String( node ).c_str(), deletedNode );
	DOMElement* NodeElement = deletedData->createElement(XMLString::transcode("DeletedData"));
	NodeElement->appendChild( deletedNode );
	deletedData->getDocumentElement()->appendChild( NodeElement );

	// Create corresponding delta operation
	
	char parXID_str[10] ;
	sprintf(parXID_str, "%d", (int)resultingDocument->getXidMap().getXIDbyNode(parent) );
	char pos_str[10];
	sprintf(pos_str,    "%d", getPosition(parent, node) );
	DOMElement* op = deltaDoc->createElement(XMLString::transcode("d"));
	op->setAttribute(XMLString::transcode("par"), XyLatinStr(parXID_str) );
	op->setAttribute(XMLString::transcode("pos"), XyLatinStr(pos_str) );
	op->setAttribute(XMLString::transcode("xm") , XyLatinStr((resultingDocument->getXidMap().String( node ).c_str()) ));
	DOMNode* contentNode = deltaDoc->importNode( node, true );
	op->appendChild( contentNode ) ;
	deltaElement->appendChild(op);
  
	// Apply operation on Document
	
	resultingDocument->getXidMap().removeSubtree( node );
	parent->removeChild( node );

	// Apply it on (=remove nodes from) ChangeSimulator data
	
	unregisterSubTree( node );

	// Counter
	
	deleted++ ;
	}


/********************************
 *                              *
 *  Create an UPDATE Operation  *
 *                              *
 ********************************/
	

void ChangeSimulator::UPDATE( DOMNode *node ) {

	DOMNode* parent = node->getParentNode();
	int node_position = getPosition(parent, node);
	if (node_position!=1) return;

	XID_t nodeXID = resultingDocument->getXidMap().getXIDbyNode( node );
	std::cout << "...update() xid=" << nodeXID << std::endl ;

	char text[50] ;
	sprintf(text, "UpdatedValue_%d", (int)updated );
	DOMNode* newText = resultingDocument->createTextNode(XyLatinStr(text));
	
	// Create corresponding delta operation
	
	DOMElement* dElem = deltaDoc->createElement(XMLString::transcode("d"));
	DOMElement* iElem = deltaDoc->createElement(XMLString::transcode("i"));
	char oldXidStr[10];
	char newXidStr[10];
	sprintf(oldXidStr, "(%d)", (int)nodeXID);
	sprintf(newXidStr, "(%d)", (int)resultingDocument->getXidMap().allocateNewXID());
	dElem->setAttribute(XMLString::transcode("xm"), XyLatinStr(oldXidStr));
	iElem->setAttribute(XMLString::transcode("xm"), XyLatinStr(newXidStr));

	char parXID_str[10] ;
	sprintf(parXID_str, "%d", (int)resultingDocument->getXidMap().getXIDbyNode(parent) );
	char pos_str[10];
	sprintf(pos_str,    "%d", node_position );
	if (node_position!=1) THROW_AWAY(("update operations are only valid for single text node attached to a label"));
	dElem->setAttribute(XMLString::transcode("par"), XyLatinStr(parXID_str) );
	dElem->setAttribute(XMLString::transcode("pos"), XyLatinStr(pos_str)    );
	dElem->setAttribute(XMLString::transcode("update"), XMLString::transcode("yes"));
	iElem->setAttribute(XMLString::transcode("par"), XyLatinStr(parXID_str) );
	iElem->setAttribute(XMLString::transcode("pos"), XyLatinStr(pos_str)    );
	iElem->setAttribute(XMLString::transcode("update"), XMLString::transcode("yes"));

	DOMNode* oldValue = deltaDoc->importNode(node, true);
	dElem->appendChild(oldValue);
	DOMNode* newValue = deltaDoc->importNode(newText, true);
	iElem->appendChild(newValue);
	
	deltaElement->appendChild(dElem);
	deltaElement->appendChild(iElem);

	// Apply operation on Document

	parent->replaceChild(newText, node);
	resultingDocument->getXidMap().removeSubtree(node);
	resultingDocument->getXidMap().mapSubtree(newXidStr, newText);

	// Note that parent node can't be used to create an insertion anymore

	elementNodes=erase(elementNodes, parent);

	// Counter

	updated++ ;
	}


/********************************
 *                              *
 *  Create an INSERT Operation  *
 *                              *
 ********************************/
	

void ChangeSimulator::INSERT( DOMNode *node ) {
	XID_t nodeXID = resultingDocument->getXidMap().getXIDbyNode( node );
	
	std::cout << "...insert() parentXid=" << nodeXID << std::endl ;
  
	// ---- Choose random position and random type : if parent has <n> children, position can be 1...n+1
	
	unsigned int position = 1 + random()%( 1 + node->getChildNodes()->getLength() );
	bool move=false ;
	
	// ---- Choose random type

	DOMNode::NodeType type ;
	if (random()%2) {
	  type=DOMNode::ELEMENT_NODE;
		}
	else {
	  type=DOMNode::TEXT_NODE ;
		}
	
	// ---- Change type if surrounding brother are TEXT NODES too

	if ((type==DOMNode::TEXT_NODE)&&(node->hasChildNodes())) {
	  if (position==1) {
		  if (node->getFirstChild()->getNodeType()==DOMNode::TEXT_NODE) {
		    type = DOMNode::ELEMENT_NODE ;
				}
			}
    else {
		  // find the one going to be its previous brother
			DOMNode* previous = node->getFirstChild() ;
	    for(unsigned int i=2; i<position; i++) previous = previous->getNextSibling() ;
		  if (previous->getNodeType()==DOMNode::TEXT_NODE) {
		    type = DOMNode::ELEMENT_NODE ;
			  }
      else {
			// will it have a 'next' brother ?
		    if ((position !=1+node->getChildNodes()->getLength())&&(previous->getNextSibling()->getNodeType()==DOMNode::TEXT_NODE)) {
		      type = DOMNode::ELEMENT_NODE ;
					}
				}
			}
		}
	
	// Create Content to Insert
	// 2 possibilities: either take the content of a deleted node, or create 'original' content
	
	// Create the new Node
	
  DOMNode* insertedNode = 0;

	// Try to transform (delete,insert) into (move)
	if (frand()<0.5) {
          std::cout << " transform (delete,insert) into (move)" << std::endl;
		
		DOMNode* dataNode = deletedData->getDocumentElement()->getLastChild() ;
		while ( (dataNode!=NULL)
		      &&(dataNode->getFirstChild()->getNodeType()!=type))
		  {
			dataNode = dataNode->getPreviousSibling() ;
			}

		if (dataNode!=NULL) {
			DOMNode* content = dataNode->getFirstChild() ;
			insertedNode = resultingDocument->importNode( content, true );
			resultingDocument->getXidMap().mapSubtree( deletedData->getXidMap().String( content ).c_str(), insertedNode );
			deletedData->getDocumentElement()->removeChild( dataNode );
			moved++ ;
			move=true;
			}
		}
	
	// Else create Original Content

	if (insertedNode==NULL) {
          std::cout << "create original content " << std::endl;
		if (type==DOMNode::ELEMENT_NODE) {
			DOMNode* copyFrom = node->getLastChild() ;
			while ((copyFrom!=NULL)&&(copyFrom->getNodeType()!=DOMNode::ELEMENT_NODE)) {
				copyFrom=copyFrom->getPreviousSibling() ;
				}
			if (copyFrom==NULL) {
				DOMNode* uncle = node->getNextSibling() ;
				if (uncle!=NULL) uncle = uncle->getFirstChild() ;
				if (uncle!=NULL) {
					// printf("I am gonna be like my cousin!\n");
					copyFrom = uncle ;
					while ((copyFrom!=NULL)&&(copyFrom->getNodeType()!=DOMNode::ELEMENT_NODE)) {
						copyFrom=copyFrom->getNextSibling() ;
						}
					}
				if (copyFrom==NULL) {
					// printf("sorry\n");
					copyFrom = node ;
					}
				}
			else {
				// printf("I am gonna be like my brother!\n");
                          std::cout << "I am gonna be like my brother!" << std::endl;
                        }
			insertedNode = resultingDocument->importNode( copyFrom, false );
                        std::cout << "resultingDocument->importNode( copyFrom, false );" << std::endl;
			}
		else { // Text content is supposed to be original
	  	char text[50] ;
			sprintf(text, "InsertedText_%d", (int)inserted );
			insertedNode = resultingDocument->createTextNode( XyLatinStr(text) ) ;
                        std::cout << "resultingDocument->createTextNode( XyLatinStr(text) );" << std::endl;
			}
		resultingDocument->getXidMap().registerNode( insertedNode, resultingDocument->getXidMap().allocateNewXID() );
		}

	/* ---- Create corresponding delta operation ---- */
	
	DOMElement* elem      = deltaDoc->createElement(XMLString::transcode("i"));
	
	char parent_str[10];
	sprintf(parent_str,   "%d", (int)nodeXID);
	char position_str[10];
	sprintf(position_str, "%d", position );

	elem->setAttribute(    XMLString::transcode("par"), XyLatinStr(parent_str) );
	elem->setAttribute(    XMLString::transcode("pos"), XyLatinStr(position_str)  );
        if (move== true)
          std::cout << "move == true" << std::endl;
        else
          std::cout << "move != true" << std::endl;
        
        if (insertedNode->hasChildNodes())
          std::cout << "insertedNode->hasChildNodes ok" << std::endl;
        else
          std::cout << "insertedNode->hasChildNodes not ok" << std::endl;
        
	std::string myXm = resultingDocument->getXidMap().String(insertedNode) ;

	if (move) {
		char xidStr[10];
		sprintf(xidStr,"(%d)", (int)resultingDocument->getXidMap().getXIDbyNode(insertedNode));
		elem->setAttribute(XMLString::transcode("move"),XMLString::transcode("yes"));
		DOMNode* deleteOp=deltaElement->getFirstChild();
		int count=0;
		while(deleteOp!=NULL) {
                  std::cout << "deleteOp!=NULL" << std::endl;
                  if ( XMLString::equals(deleteOp->getNodeName(), XMLString::transcode("d"))) {
                    if ( XMLString::equals(deleteOp->getAttributes()->getNamedItem(XMLString::transcode("xm"))->getNodeValue(), XyLatinStr(myXm.c_str()))) {
					printf("   found corresponding delete operation\n");

                                        std::cout << "getNodeValue()) = " << XyLatinStr(deleteOp->getAttributes()->getNamedItem(XMLString::transcode("xm"))->getNodeValue()).localForm() << std::endl;
                                        std::cout << "myXm = " << myXm << std::endl;

					count++;
					DOMElement *deleteElem=(DOMElement*)deleteOp;
					deleteElem->setAttribute(XMLString::transcode("move"), XMLString::transcode("yes"));
					deleteOp->getAttributes()->getNamedItem(XMLString::transcode("xm"))->setNodeValue(XyLatinStr(xidStr));
					}
                                else
                                  {
                                    std::cout << "not found corresponding delete operation" << std::endl;
                                    std::cout << "failed : " << XyLatinStr(deleteOp->getAttributes()->getNamedItem(XMLString::transcode("xm"))->getNodeValue()).localForm() << std::endl;
                                    std::cout << " with " << myXm.c_str() << std::endl;
                                  }
				}
                        else
                          {
                            std::cout << "deleteOp->getNodeName() = " << XyLatinStr(deleteOp->getNodeName()).localForm() << std::endl;
                          }
			deleteOp=deleteOp->getNextSibling();
			}
                std::cout << "deleteOp==NULL" << std::endl;
		if (count!=1) THROW_AWAY(("we've found %d delete operations when looking for one!", count));
		elem->setAttribute(    XMLString::transcode("xm"), XyLatinStr(xidStr) );
		}
	else {
		// For real insert: describe the Xidmap for the subtree
		elem->setAttribute(    XMLString::transcode("xm"), XyLatinStr(myXm.c_str()) );
		}
		
	DOMNode* contentNode  = deltaDoc->importNode(insertedNode, true );
	elem->appendChild (contentNode) ;
	deltaElement->appendChild( elem );

  /* ---- Update ResultingDocument ---- */
	
	//DOM_Node newNode = resultingDocument.importNode( insertedNode, true );
	std::cout << "   insert data xidmap="<<resultingDocument->getXidMap().String(insertedNode)<<std::endl;
	//resultingDocument.getXidMap().mapSubtree(resultingDocument.getXidMap().String( insertedNode ).c_str(), newNode);

  if (position == 1+node->getChildNodes()->getLength()) {
		//node.appendChild( newNode );
		node->appendChild(insertedNode);
		}
	else {
	  DOMNode* next = node->getFirstChild();
		for(unsigned int i=1; i<position; i++) next=next->getNextSibling();
		//node.insertBefore( newNode, next );
		node->insertBefore(insertedNode, next);
		}

	//registerSubTree( newNode );
	registerSubTree(insertedNode);

  // statistics
	
	inserted++;
	
	}


double frand(void) {
  double x = rand() ;
	double y = RAND_MAX ;
	return (x/y) ;
	}


/***************************************************************************
 *                                                                         *
 *                                                                         *
 *      -+-+-+-+-+-+-+- Atomic Create Random Script -+-+-+-+-+-+-+-        *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


XID_DOMDocument* AtomicCreateRandomScript(XID_DOMDocument *sourceDoc, const char *sourceFilename, const char *targetFilename, const char *deltaFilename) {
    
	std::cout << "Constructing changeSimulator" << std::endl ;
		
	ChangeSimulator *changeSimulator = new ChangeSimulator(sourceDoc, sourceFilename, targetFilename, deleteProb, updateProb, insertProb) ;

	int elemsize = changeSimulator->elementNodes.size();
	int textsize = changeSimulator->textNodes.size();
	int originaldocsize = textsize+elemsize;

	printf("Number of Element nodes= %d\n", elemsize);
	printf("Number of Text nodes   = %d\n", textsize);
	printf("Document size          = %d\n", originaldocsize);

	/* Do DELETE Operations */
		
	printf("create 'delete' operations...\n");
	for(unsigned int i=0; i< changeSimulator->textNodes.size() ; i++) {
		DOMNode* node = changeSimulator->textNodes[i] ;
	  if (frand()<deleteProb) changeSimulator->DELETE(node);
		}
	for(unsigned int i=0; i< changeSimulator->elementNodes.size() ; i++) {
		DOMNode* node = changeSimulator->elementNodes[i];
		long size = changeSimulator->resultingDocument->getSubtreeNodeCount ( node ) ;
		if (frand()<(deleteProb/(double)size)) changeSimulator->DELETE( node );
		if (changeSimulator->elementNodes.size()<10) THROW_AWAY(("The sequence of documents that has just been created is not good enough for testing. Please try again."));
		}



        std::cout << "AtomicCreateRandomScript nb deleted = " << deleted << std::endl;
			
	/* Correct Probabilities */
	printf("correct probabilities according to the new size of the document...\n");
	if ( changeSimulator->textNodes.size() ) updateProb=updateProb * textsize / changeSimulator->textNodes.size() ;
	if ( changeSimulator->elementNodes.size() ) insertProb=insertProb * elemsize / changeSimulator->elementNodes.size() ;
		
  /* Do UPDATE Operations */

	printf("create 'update' operations...\n");
	for(unsigned int i=0; i< changeSimulator->textNodes.size() ; i++) {
	  if (frand()<updateProb) {
			DOMNode* node = changeSimulator->textNodes[i] ;
			changeSimulator->UPDATE ( node );
			}
		}

        std::cout << "AtomicCreateRandomScript nb updated = " << updated << std::endl;
			
	/* Do INSERT Operations */
			
	printf("create 'insert' operations...\n");
	for(unsigned int i=0; i< changeSimulator->elementNodes.size() ; i++) {
		if (frand()<insertProb) {
			DOMNode* node = changeSimulator->elementNodes[i] ;
			changeSimulator->INSERT( node );
			}
		}

        std::cout << "AtomicCreateRandomScript nb inserted = " << inserted << std::endl;
			
  /* ---- End Random changes */

	unsigned int total = changeSimulator->textNodes.size() + changeSimulator->elementNodes.size() ;
	printf("Result: %d nodes ( %d text nodes, %d element nodes)\n", total, changeSimulator->textNodes.size(), changeSimulator->elementNodes.size());

  if (total==0) THROW_AWAY(("bad luck the whole document has been deleted! maybe try to reduce the delete probability parameter?"));

	printf("Save delta document\n");
	changeSimulator->deltaDoc->SaveAs(deltaFilename, false );

	XID_DOMDocument* resultDoc = changeSimulator->resultingDocument ;
	std::cout << "delete ChangeSimulator" << std::endl ;
	delete changeSimulator ;
	
	std::cout << "Returning resulting Document" << std::endl ;
	return ( resultDoc );		  
	}



/***************************************************************************
 *                                                                         *
 *                                                                         *
 *              -+-+-+-+-+-+-+- Main Program -+-+-+-+-+-+-+-               *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


int main(int argc, char* argv[])
{

  bool help=false;
	for(int i=1;i<argc;i++) if (strcmp(argv[i],"--help")==0) help=true;
	if ((argc<3)||help) {
	  std::cerr << "error: Bad Arguments" << std::endl ;
		std::cerr << "usage: exec deltafilespattern [-start initfile] xmlfilespattern <n> [alpha beta gamma]\n" ;
		cerr << "Simulates random changes on an sequence XML document\n" ;
		cerr << "        <n>   : number of simulations\n" ;
		cerr << "        alpha : <delete> probability per node\n" ;
		cerr << "        beta  : <update> probability per node\n" ;
		cerr << "        gamma : <insert> probability per node\n" ;
		cerr << "example: exec example_delta_%02d.xml -start example1.xml example1_%02d.xml -10 0.05 0.1 0.1\n\n" ;
		return(0);
		}
		
	/* --- Read Command Line arguments --- */
	
	int pos=0;
	
	deltafilespattern=argv[pos+1];
	pos++;
	
	if (strcmp(argv[pos+1], "-start")==0) {
		initfile=argv[pos+2];
		pos+=2;
		startFlag=true;
		}

	xmlfilespattern=argv[pos+1] ;
	sequenceSize = strtol(argv[pos+2], NULL, 10);
	if (sequenceSize<=0) {
		fprintf(stderr, "error: second argument -the number of simulations- must be at least 1 (yours was %d)\n", sequenceSize);
		return(0);
		}
	pos+=2;	
	
	if (argc>pos+1) {
		if (argc==4+pos) {
			deleteProb = atof(argv[pos+1]);
			updateProb = atof(argv[pos+2]);
			insertProb = atof(argv[pos+3]);
			}
		else {
			fprintf(stderr, "error in [alpha beta gamma] arguments\n");
			return(0);
			}
		}
	else {
		deleteProb = 0.1 ;
		updateProb = 0.1 ;
		insertProb = 0.1 ;
		}
		
	/* ---- Init Xerces ---- */
		
	try {
		XMLPlatformUtils::Initialize();
		}

  catch(const XMLException& toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
		     << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
    }

	// Randomize
	int randint = (int) time(NULL);
	srandom(randint);

	try {
	
	  long round = 0 ;
		// Open Original XML File
    char filename[1000];
		sprintf(filename, xmlfilespattern.c_str(), round);
	
		if (startFlag) {
			XID_DOMDocument *d = new XID_DOMDocument(initfile.c_str());
			d->SaveAs(filename, true);
                        delete d;
			}

		XID_DOMDocument *first = new XID_DOMDocument(filename); 

    // Do Simulations
	
		while(round<sequenceSize) {
	  	char file1[1000];
			char file2[1000];
			char delta[1000];
			sprintf(file1, xmlfilespattern.c_str(), round);
			sprintf(file2, xmlfilespattern.c_str(), round+1);
			sprintf(delta, deltafilespattern.c_str(), round);
			
		  fprintf(stderr, "### Construct delta transforming <%s> into <%s>\n", file1, file2);
			fprintf(stdout, "\n\n### Construct delta transforming <%s> into <%s>\n\n\n", file1, file2);

		  printf("deleteProb=%.2f, updateProb=%.2f, insertProb=%.2f\n", deleteProb, updateProb, insertProb);
			XID_DOMDocument* second ;
			second = AtomicCreateRandomScript(first, file1, file2, delta);

			std::string resultSave = std::string(file2)+"_expected" ;
			printf("Save resulting document <%s>", resultSave.c_str());
			second->SaveAs(resultSave.c_str(), true);
		
			std::cout << "Let new version be actual version" << std::endl ;

                        delete first;
	    first = second;
			round++;
		  } // End of loop 
                delete first;
    }
	catch( const VersionManagerException &e ) {
	  std::cerr << e << std::endl ;
		}
	catch( const DeltaException &e ) {
	  std::cerr << "DeltaException catched. Exit program..." << std::endl ;
		exit(-1);
		}
	catch( const DOMException &e ) {
	  std::cerr << "DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOMException, message=" << e.msg << std::endl ;
		exit(-1);
		}	

  std::cout << "Finished." << std::endl ;
	}



