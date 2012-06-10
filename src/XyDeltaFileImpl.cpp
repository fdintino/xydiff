#include "xydiff/XyDelta_FileInterface.hpp"

#include "xydiff/XyLatinStr.hpp"
#include "xydiff/XyUTF8Str.hpp"
#include "xydiff/XID_map.hpp"
#include "xydiff/XID_DOMDocument.hpp"

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
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/framework/StdOutFormatTarget.hpp"


#include <stdio.h>
#include <map>
#include <queue>
#include <list>
#include <math.h>
#include <fstream>
#include "infra/general/hash_map.hpp"

XERCES_CPP_NAMESPACE_USE



/********************************************************************
 |*                                                                 *
 |*                                                                 *
 |* Single Command DeltaApply                                       *
 |*                                                                 *
 |*                                                                 *
 ********************************************************************/

void XyDelta::ApplyDelta(const char *incDeltaName, const char *incSourceName, const char *incDestinationName, bool applyAnnotations) {

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

//	if (f!=NULL) {
//		fclose(f);
//		THROW_AWAY(("destination file <%s> already exists",destinationName.c_str()));
//		}
		
	vddprintf(("Opening source file: %s\n",sourceName.c_str() ));
	XID_DOMDocument* sourceXML = new XID_DOMDocument(sourceName.c_str()) ;

	vddprintf(("Apply DELTA on Document\n"));
	DeltaApplyEngine appliqueDoc(sourceXML);
	appliqueDoc.setApplyAnnotations(applyAnnotations);
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
XID_DOMDocument* XidXyDiff(XID_DOMDocument* v0XML, const char *doc1name, XID_DOMDocument* v1XML, const char *doc2name, bool ignoreSpacesFlag=false, bool verbose=false);

// called by main() in execComputeDelta.cpp

void XyDelta::XyDiff(const char *incV0filename, const char *incV1filename, const char *deltafilename, bool ignoreSpacesFlag, bool verbose) {


	std::string v0filename = incV0filename ;
	std::string v1filename = incV1filename ;
	
	/* ---- [[ Phase 0: ]] Read and Parse documents ---- */
		
		
	vddprintf(("\n+++ Opening and Registration of XML Documents +++\n\n")) ;

	vddprintf(("Opening v0 file: %s\n", v0filename.c_str())) ;

	XID_DOMDocument* v0XML = new XID_DOMDocument( v0filename.c_str(), true, false) ;
		
	vddprintf(("Opening v1 file: %s\n", v1filename.c_str() )) ;

	std::string v1xidmapFilename = v1filename + ".xidmap" ;
	FILE *f=fopen(v1xidmapFilename.c_str(), "r");
	if (f!=NULL) {
		fclose(f);
		remove(v1xidmapFilename.c_str());
		// THROW_AWAY(("xidmap file already exists for file <%s>", v1filename.c_str()));
	}

	XID_DOMDocument* v1XML = new XID_DOMDocument( v1filename.c_str(), false ) ;
	v1XML->getXidMap().initFirstAvailableXID(v0XML->getXidMap().getFirstAvailableXID());
		

	/* --- DO IT ---- */


	XID_DOMDocument* delta = XidXyDiff(v0XML, v0filename.c_str(), v1XML, v1filename.c_str(), ignoreSpacesFlag, verbose);


	/* ---- [[ Phase Terminal: ]] Save Result in a file ---- */
	
#ifndef DONT_SAVE_RESULT
	vddprintf(("\n+++ Phase Inf-2: Save Result in a file +++\n\n")) ;


	{
	

	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
	DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMLSOutput *theOutput = ((DOMImplementationLS*)impl)->createLSOutput();

	const char *stdoutString = "stdout";
	if (strcmp(deltafilename, stdoutString) != 0) {	std::cout << "Outputting to file " << deltafilename << std::endl;
		LocalFileFormatTarget *myFormatTarget = new LocalFileFormatTarget(deltafilename);
		if (!myFormatTarget) {
			std::cerr << "Error: could not open <" << deltafilename << "> for output" << std::endl ;
			return;
		}
		theOutput->setByteStream(myFormatTarget);
	} else {
		std::cout << "Outputting to stdout" << std::endl;
		XMLFormatTarget *myFormatTarget = new StdOutFormatTarget();
		theOutput->setByteStream(myFormatTarget);
	}		
	
	try {
		if (theSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
			theSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
		if (theSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMXMLDeclaration, true)) 
		    theSerializer->getDomConfig()->setParameter(XMLUni::fgDOMXMLDeclaration, true);
		// do the serialization through DOMLSSerializer::write();
		// If output file is "stdout," send output to stdout and not a file.
	
		
		theSerializer->write((DOMDocument*)delta, theOutput);
      }
      catch (const XMLException& toCatch) {
          char* message = XMLString::transcode(toCatch.getMessage());
          std::cout << "Exception message is: \n"
               << message << "\n";
          XMLString::release(&message);
      }
      catch (const DOMException& toCatch) {
          char* message = XMLString::transcode(toCatch.msg);
          std::cout << "Exception message is: \n"
               << message << "\n";
          XMLString::release(&message);
      }
      catch (...) {
          std::cout << "Unexpected Exception \n" ;
      }

		theOutput->release();
	theSerializer->release();
	
	// End changes
	}

#endif
	
	/* ---- [[ Phase Post: ]] Clean-up uses ressources ---- */
		
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


	XID_DOMDocument* delta = XidXyDiff(v0XML, v0filename.c_str(), v1XML, v1filename.c_str(), true, false);


		/* ---- [[ Phase Terminal: ]] Save Result in a file ---- */
	
#ifndef DONT_SAVE_RESULT
		vddprintf(("\n+++ Phase Inf-2: Save Result in a file +++\n\n")) ;
       
	
		// Use DeltaManger: addDeltaElement and then SaveToDisk()
		// Also set a fromVersionId and toVersionId using DeltaManager
	
        DOMNode* deltaNode = delta->getDocumentElement()->getFirstChild();
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
	DOMNode* root = doc0->getDocumentElement();
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

