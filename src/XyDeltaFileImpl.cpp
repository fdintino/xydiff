#include "include/XyDelta_FileInterface.hpp"

#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"

#include "DeltaApply.hpp"
#include "DOMPrint.hpp"
#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "Diff_NodesManager.hpp"
#include "Diff_DeltaConstructor.hpp"
#include "Diff_UniqueIdHandler.hpp"
#include "DeltaException.hpp"
#include "DeltaManager.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMLSOutput.hpp"
#include "xercesc/dom/DOMLSSerializer.hpp"
#include "xercesc/dom/DOMLSOutput.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/validators/DTD/DTDValidator.hpp"


#include <stdio.h>
#include <map>
#include <queue>
#include <list>
#include <math.h>
#include <fstream>
#include "infra/general/hash_map.hpp"

#include <sys/timeb.h>
#include <sys/time.h>

extern unsigned long long int clocksComputeDelta           ;

extern unsigned long long int clocksReadDocuments          ;
extern unsigned long long int clocksRegisterSubtree        ;
extern unsigned long long int clocksTopDownMatch           ;
extern unsigned long long int clocksOptimize               ;
extern unsigned long long int clocksConstructDeltaDocument ;
extern unsigned long long int clocksSaveDelta              ;

void TimeInit(void);
void TimeStep(void);


/********************************************************************
 |*                                                                 *
 |*                                                                 *
 |* Single Command DeltaApply                                       *
 |*                                                                 *
 |*                                                                 *
 ********************************************************************/

void XyDelta::ApplyDelta(const char *incDeltaName, const char *incSourceName, const char *incDestinationName) {

	std::string deltaName = incDeltaName ;
	std::string sourceName ;
	std::string destinationName ;

	vddprintf(("Opening delta file <%s>\n", deltaName.c_str()));
                std::cout << "delta file " << deltaName << " parsing start" << std::endl;
	XID_DOMDocument* deltaXML = new XID_DOMDocument(deltaName.c_str(), false) ;

        std::cout << "delta file " << deltaName << " parse ok" << std::endl;

	if (incSourceName) sourceName = incSourceName;
	else sourceName = DeltaApplyEngine::getSourceURI( deltaXML );
	
        std::cout << "sourceName " << sourceName << std::endl;

	if (incDestinationName) destinationName = incDestinationName;
	else destinationName = DeltaApplyEngine::getDestinationURI( deltaXML );

        std::cout << "destinationName " << destinationName << std::endl;

	FILE *f=fopen(destinationName.c_str(), "r");
	if (f!=NULL) {
		fclose(f);
		THROW_AWAY(("destination file <%s> already exists",destinationName.c_str()));
		}
		
	vddprintf(("Opening source file: %s\n",sourceName.c_str() ));
	XID_DOMDocument* sourceXML = new XID_DOMDocument(sourceName.c_str()) ;

	vddprintf(("Apply DELTA on Document\n"));
	DeltaApplyEngine appliqueDoc(sourceXML);
	appliqueDoc.ApplyDelta(deltaXML) ;

	vddprintf(("Saving Result to: %s\n", destinationName.c_str() ));
	appliqueDoc.getResultDocument()->SaveAs(destinationName.c_str());
	vddprintf(("Terminated.\n"));
}


// called from XyRetrieve.cxx 
void XyDelta::ApplyDelta(const char *incDeltaName, unsigned int backwardNumber) {
	
	std::string sourceName ;
	std::string destinationName = "./dirtest/backwardVersion.xml";
    
	// Select a deltaElement from the deltaFile using DeltaMananger

    DeltaManager deltaMgr = DeltaManager(incDeltaName) ;
    XID_DOMDocument *deltaXML = new XID_DOMDocument(deltaMgr.getDeltaDocument());

	if (deltaXML->getDocumentElement()->getChildNodes()->getLength()-1 < backwardNumber) {
		throw VersionManagerException("Data error", "DeltaApply", "incorrect specified backwardNumber of document to recover");
	}

	sourceName = deltaMgr.getCurrentVersionFileName();

//	if (incDestinationName) destinationName = incDestinationName;
//	else destinationName = DeltaApplyEngine::getDestinationURI( deltaXML );

	FILE *f=fopen(destinationName.c_str(), "r");
	if (f!=NULL) {
		fclose(f);
		THROW_AWAY(("destination file <%s> already exists",destinationName.c_str()));
	}
		
	vddprintf(("Opening source file: %s\n",sourceName.c_str() ));
	XID_DOMDocument* sourceXML = new XID_DOMDocument(sourceName.c_str()) ;

	if (backwardNumber <= 0) {
		sourceXML->SaveAs(destinationName.c_str());
	}
	else {
		vddprintf(("Apply DELTA on Document\n"));
		DeltaApplyEngine appliqueDoc(sourceXML);
    
		// ApplyDeltaElement() because now you select a deltaElement using DeltaManager
		appliqueDoc.ApplyDelta(deltaXML, backwardNumber) ; 

		vddprintf(("Saving Result to: %s\n", destinationName.c_str() ));
		appliqueDoc.getResultDocument()->SaveAs(destinationName.c_str());
		vddprintf(("Terminated.\n"));
	}
 }

// From ComputeDelta.cpp
XID_DOMDocument* XidXyDiff(XID_DOMDocument* v0XML, const char *doc1name, XID_DOMDocument* v1XML, const char *doc2name, bool ignoreSpacesFlag=false, bool verbose=false, xercesc_3_0::DTDValidator *dtdValidator=NULL);

// called by main() in execComputeDelta.cpp

void XyDelta::XyDiff(const char *incV0filename, const char *incV1filename, const char *deltafilename, bool ignoreSpacesFlag, bool verbose) {

	std::string v0filename = incV0filename ;
	std::string v1filename = incV1filename ;
	
#ifdef HW_PROF
	clocksComputeDelta           = 0 ;
	
	clocksReadDocuments          = 0 ;
	clocksSaveDelta              = 0 ;
#endif

	TimeInit();
	
	/* ---- [[ Phase 0: ]] Read and Parse documents ---- */
		
	unsigned long long int start000 = rdtsc() ;
	unsigned long long int start = rdtsc() ;
		
	vddprintf(("\n+++ Opening and Registration of XML Documents +++\n\n")) ;

	vddprintf(("Opening v0 file: %s\n", v0filename.c_str())) ;

	XID_DOMDocument* v0XML = new XID_DOMDocument( v0filename.c_str(), true, false) ;
		
	vddprintf(("Opening v1 file: %s\n", v1filename.c_str() )) ;

	std::string v1xidmapFilename = v1filename + ".xidmap" ;
	FILE *f=fopen(v1xidmapFilename.c_str(), "r");
	if (f!=NULL) {
		fclose(f);
		THROW_AWAY(("xidmap file already exists for file <%s>", v1filename.c_str()));
		}

	XID_DOMDocument* v1XML = new XID_DOMDocument( v1filename.c_str(), false ) ;
	v1XML->getXidMap().initFirstAvailableXID(v0XML->getXidMap().getFirstAvailableXID());
		
	clocksReadDocuments += rdtsc() - start ;
	TimeStep();

	/* --- DO IT ---- */


	XID_DOMDocument* delta = XidXyDiff(v0XML, v0filename.c_str(), v1XML, v1filename.c_str(), ignoreSpacesFlag, verbose, NULL);


	/* ---- [[ Phase Terminal: ]] Save Result in a file ---- */
	
#ifndef DONT_SAVE_RESULT
	vddprintf(("\n+++ Phase Inf-2: Save Result in a file +++\n\n")) ;
	start = rdtsc() ;


	{
	std::ofstream flux(deltafilename); 
	
	// Old:flux << *delta << std::endl;
	// Start changes
	XMLCh tempStr[100];
	xercesc_3_0::XMLString::transcode("LS", tempStr, 99);
	xercesc_3_0::DOMImplementation *impl = xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(tempStr);
	xercesc_3_0::DOMLSSerializer* theSerializer = ((xercesc_3_0::DOMImplementationLS*)impl)->createLSSerializer();

	if (theSerializer->getDomConfig()->canSetParameter(xercesc_3_0::XMLUni::fgDOMWRTFormatPrettyPrint, false))
		theSerializer->getDomConfig()->setParameter(xercesc_3_0::XMLUni::fgDOMWRTFormatPrettyPrint, false);
	
	
	try {
          // do the serialization through DOMLSSerializer::writeToString();
		XMLCh * serializedString = theSerializer->writeToString((xercesc_3_0::DOMNode*)delta->getDocumentElement());
		flux << xercesc_3_0::XMLString::transcode(serializedString) << std::endl;
      }
      catch (const xercesc_3_0::XMLException& toCatch) {
          char* message = xercesc_3_0::XMLString::transcode(toCatch.getMessage());
          std::cout << "Exception message is: \n"
               << message << "\n";
          xercesc_3_0::XMLString::release(&message);
      }
      catch (const xercesc_3_0::DOMException& toCatch) {
          char* message = xercesc_3_0::XMLString::transcode(toCatch.msg);
          std::cout << "Exception message is: \n"
               << message << "\n";
          xercesc_3_0::XMLString::release(&message);
      }
      catch (...) {
          std::cout << "Unexpected Exception \n" ;
      }

	theSerializer->release();
	
	// End changes
	}

	clocksSaveDelta += rdtsc() - start ;
#endif
	
	/* ---- [[ Phase Post: ]] Clean-up uses ressources ---- */
		
	TimeStep();
	
	vddprintf(("\n+++ Freeing Ressources +++\n\n")) ;

	vddprintf(("closing v1XML\n")) ;
	v1XML->release();
	v1XML=NULL;
	vddprintf(("closing v0XML\n")) ;
	v0XML->release();
	v0XML=NULL;
	vddprintf(("closing delta document\n")) ;
	delta->release();
	delta=NULL;
	vddprintf(("closing stack\n")) ;

	/* ---- [[ END ]] Program terminated ---- */

	clocksComputeDelta += rdtsc() - start000 ;
	TimeStep() ;

#ifdef HW_PROF
	if (verbose) {
		printf("computeDelta                 : %5dms\n", (int)(clocksComputeDelta           /(CLOCKRATE/1000)) );
		printf("  (0) Read documents              : %5dms\n", (int)(clocksReadDocuments          /(CLOCKRATE/1000)) );
		printf("  (1) RegisterSubtree             : %5dms\n", (int)(clocksRegisterSubtree        /(CLOCKRATE/1000)) );
		printf("  (2) top-down Match              : %5dms\n", (int)(clocksTopDownMatch           /(CLOCKRATE/1000)) );
		printf("    getBestCandidate                   : %5dms\n", (int)(clocksGetBestCandidate       /(CLOCKRATE/1000)) );
		printf("    clocksQueueOps                     : %5dms\n", (int)(clocksQueueOps               /(CLOCKRATE/1000)) );
		printf("    clocksPropagateAssign              : %5dms\n", (int)(clocksPropagateAssign        /(CLOCKRATE/1000)) );
		printf("  (3) Optimize                    : %5dms\n", (int)(clocksOptimize               /(CLOCKRATE/1000)) );
		printf("  (4) ConstructDeltaDocument      : %5dms\n", (int)(clocksConstructDeltaDocument /(CLOCKRATE/1000)) );
		printf("    init DOM delta                     : %5dms\n", (int)(clocksCDDinitDomDelta        /(CLOCKRATE/1000)) );
		printf("    prepare vectors                    : %5dms\n", (int)(clocksCDDprepareVectors      /(CLOCKRATE/1000)) );
		printf("    save XidMap                        : %5dms\n", (int)(clocksCDDsaveXidMap          /(CLOCKRATE/1000)) );
		printf("    compute weak move                  : %5dms\n", (int)(clocksCDDcomputeWeakMove     /(CLOCKRATE/1000)) );
		printf("    detect update                      : %5dms\n", (int)(clocksCDDdetectUpdate        /(CLOCKRATE/1000)) );
		printf("    add DOM attribute operations       : %5dms\n", (int)(clocksCDDaddAttributeOps     /(CLOCKRATE/1000)) );
		printf("    construct DOM delta                : %5dms\n", (int)(clocksConstructDOMDelta      /(CLOCKRATE/1000)) );
		printf("  (5) Save delta                  : %5dms\n", (int)(clocksSaveDelta              /(CLOCKRATE/1000)) );
		}
#endif

	return ;
}



void XyDelta::XyLoadAndDiff(const char *versionFile , const char *deltaFile){

	if ( !existsFile(deltaFile) ) {
		// create the delta document and save it ; initialize the metainfo for the delta document
		DeltaManager dm = DeltaManager(deltaFile, versionFile);
		dm.SaveToDisk();
	}
	else {
		// compute the new delta element and add it to the delta file
		// also update the metainfo of delta file
		DeltaManager dm = DeltaManager(deltaFile);
       
		std::string v0filename = dm.getCurrentVersionFileName() ;
		std::string v1filename = versionFile ;
	
	
		/* ---- [[ Phase 0: ]] Read and Parse documents ---- */
    
		vddprintf(("\n+++ Opening and Registration of XML Documents +++\n\n")) ;
	
		vddprintf(("Opening v0 file: %s\n", v0filename.c_str())) ;
	
		XID_DOMDocument* v0XML = new XID_DOMDocument( v0filename.c_str(), true, false) ;
	
		vddprintf(("Opening v1 file: %s\n", v1filename.c_str() )) ;
	
		std::string v1xidmapFilename = v1filename + ".xidmap" ;
		FILE *f=fopen(v1xidmapFilename.c_str(), "r");
		if (f!=NULL) {
			fclose(f);
			THROW_AWAY(("xidmap file already exists for file <%s>", v1filename.c_str()));
		}
	
		XID_DOMDocument *v1XML = new XID_DOMDocument( v1filename.c_str(), false ) ;
		v1XML->getXidMap().initFirstAvailableXID(v0XML->getXidMap().getFirstAvailableXID());

	   
		/* --- DO IT ---- */


	XID_DOMDocument* delta = XidXyDiff(v0XML, v0filename.c_str(), v1XML, v1filename.c_str(), false, false, NULL);


		/* ---- [[ Phase Terminal: ]] Save Result in a file ---- */
	
#ifndef DONT_SAVE_RESULT
		vddprintf(("\n+++ Phase Inf-2: Save Result in a file +++\n\n")) ;
       
	
		// Use DeltaManger: addDeltaElement and then SaveToDisk()
		// Also set a fromVersionId and toVersionId using DeltaManager
	
        xercesc_3_0::DOMNode* deltaNode = delta->getDocumentElement()->getFirstChild();
		if ( dm.addDeltaElement(deltaNode, versionFile) == 0 ) {
			dm.SaveToDisk();
			dm.listAllDeltas();
		}
		else {
			throw VersionManagerException("Data Error", "ComputeDelta", "addDeltaElement");
		}
#endif
	
		/* ---- [[ Phase Post: ]] Clean-up uses ressources ---- */
	
		vddprintf(("\n+++ Freeing Ressources +++\n\n")) ;
	
		XID_DOMDocument* emptyDoc ;
	
		vddprintf(("closing v1XML\n")) ;
		v1XML = emptyDoc ;
	
		vddprintf(("closing v0XML\n")) ;
		v0XML = emptyDoc ;
	
		vddprintf(("closing stack\n")) ;
	}
	return;
}



/*******************************************************************
 *                                                                 *
 * Function Diff for Spin Project                                  *
 *                                                                 *
 * Author: Gregory COBENA                                          *
 *                                                                 *
 * Read files V0 and V1                                            *
 * Writes V1.xidmap (and V0.xidmap if it doesn't exists)           *
 * Writes XID tagged (on attributes) versions of V0 and V1         *
 * Writes the Delta from V0 to V1                                  *
 *                                                                 *
 *                                                                 *
 *******************************************************************/

int SpinProject::RunDiff(const char *openFileV0, const char *openFileV1, const char *saveFileV0, const char *saveFileV1, const char *saveDeltaV0V1) {
	if ((openFileV0==NULL)||(openFileV1==NULL)) {
  	fprintf(stderr, "SpinProject::RunDiff ERROR - missing input argument\n");
    return -1;
    }
	if ((saveFileV0==NULL)||(saveFileV1==NULL)||(saveDeltaV0V1==NULL)) {
  	fprintf(stderr, "SpinProject::RunDiff ERROR - missing output argument\n");
    return -1;
    }
  
  XyDelta::XyDiff(openFileV0, openFileV1, saveDeltaV0V1);
  
	XID_DOMDocument *doc0 = new XID_DOMDocument(openFileV0);
	xercesc_3_0::DOMNode* root = doc0->getDocumentElement();
	if (root!=NULL) Restricted::XidTagSubtree(doc0, root);
	doc0->SaveAs(saveFileV0, false);                                    

	XID_DOMDocument *doc1 = new XID_DOMDocument(openFileV1);
	root = doc1->getDocumentElement();
	if (root!=NULL) Restricted::XidTagSubtree(doc1, root);
	doc1->SaveAs(saveFileV1, false);                                    
        delete doc0;
        delete doc1;

  return 0;
  }

