#include "xyleme_DOMPrint.hpp"

#include <string.h>
#include <stdlib.h>

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "xydiff/XID_map.hpp"
#include "xydiff/XyLatinStr.hpp"

XERCES_CPP_NAMESPACE_USE
using namespace std;

/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/**
 * $Log: xyleme_DOMPrint.cpp,v $
 * Revision 1.9  2004/09/08 07:32:08  cobena
 * fix some ambiguous << XyLatinStr(...) lines of code.
 * Some compilers require to explicit state: << (char*)... or << XyLatinStr(...).localForm()
 *
 * Revision 1.8  2004/06/30 14:43:50  cobena
 * make test more intensive (larger files, more diffs, ...)
 *
 * Revision 1.7  2003/11/05 20:38:05  preda
 * GCC3
 * double default value
 * missing includes
 * namespace std;
 *
 * Revision 1.6  2003/10/24 12:52:36  cobena
 * merge changes from V3.2.0
 * change is to manage Latin1 transcoding with a *real* Xerces transcoder
 * the class is now XyLatinStr
 *
<<<<<<< xyleme_DOMPrint.cpp
 * Revision 1.5  2003/10/24 11:06:38  cobena
 * remove unused code
 *
 * Revision 1.4  2003/10/08 15:30:58  cobena
 * replace memory leak with XMLString::transcode by XyStr
 * note that there is a bug in compiler or somewhere if two many casts occurs on
 * the same line.
 *
 * Also one should be careful to not do: const char *s = XyStr(...), since the
 * pointer has only a limited validity.
 *
 * Well, now it seems to work fine.
 *
=======
 * Revision 1.3.2.3  2003/10/24 12:42:58  cobena
 * small fixes
 *
 * Revision 1.3.2.2  2003/10/24 12:03:27  cobena
 * add XyLatinStr for real ISO-8859-1 transcoding
 *
 * Revision 1.3.2.1  2003/10/24 11:21:03  cobena
 * get version from HEAD
 *
 * Revision 1.5  2003/10/24 11:06:38  cobena
 * remove unused code
 *
 * Revision 1.4  2003/10/08 15:30:58  cobena
 * replace memory leak with XMLString::transcode by XyStr
 * note that there is a bug in compiler or somewhere if two many casts occurs on
 * the same line.
 *
 * Also one should be careful to not do: const char *s = XyStr(...), since the
 * pointer has only a limited validity.
 *
 * Well, now it seems to work fine.
 *
>>>>>>> 1.3.2.3
 * Revision 1.3  2003/07/23 10:41:06  cobena
 * remove some outputs
 *
 * Revision 1.2  2003/07/04 09:17:02  leninive
 * new version of XyDiff using new xercesc_3_0 Dom implementation
 *
 * Revision 1.1  2003/06/11 12:06:25  leninive
 * XyDiff : use .cc and .hh extensions to work with standard build
 *
 * Revision 1.6  2003/04/17 13:12:06  green
 * - globally replace ISO-8859-1 with ISO-8859-1
 *
 * Revision 1.5  2003/04/08 13:29:50  green
 * - sever lingering transcoder.hh dependency
 *
 * Revision 1.4  2003/03/03 10:03:43  cobena
 * -add XID debug print mode
 *
 * Revision 1.3  2002/04/17 13:03:25  cobena
 * add xidmap string to the delta
 * plus small fixes
 *
 * Revision 1.2  2002/04/16 15:44:46  cobena
 * makefile update to eb compliant with xyleme makefiles
 *
 * Revision 1.1  2002/04/16 15:29:10  cobena
 * cleanup printing functions to make upgrages easier using xercesC++ DOMPrint
 * example
 *
 * Revision 1.1.1.1  2000/10/11 09:24:44  ferran
 * ok
 *
 * Revision 1.5  2000/02/06 07:47:18  rahulj
 * Year 2K copyright swat.
 *
 * Revision 1.4  2000/01/29 00:39:08  andyh
 * Redo synchronization in DOMStringHandle allocator.  There
 * was a bug in the use of Compare and Swap.  Switched to mutexes.
 *
 * Changed a few plain deletes to delete [].
 *
 * Revision 1.3  1999/12/03 00:14:52  andyh
 * Removed transcoding stuff, replaced with DOMString::transcode.
 *
 * Tweaked xml encoding= declaration to say ISO-8859-1.  Still wrong,
 * but not as wrong as utf-8
 *
 * Revision 1.2  1999/11/12 02:14:05  rahulj
 * It now validates when the -v option is specified.
 *
 * Revision 1.1.1.1  1999/11/09 01:09:51  twl
 * Initial checkin
 *
 * Revision 1.7  1999/11/08 20:43:35  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */


// ---------------------------------------------------------------------------
//  This sample program which invokes the DOMParser to build a DOM tree for
//  the specified input file. It then walks the tree, and prints out the data
//  as an XML file.
//
//  The parameters are:
//
//      [-?]            - Show usage and exit
//      [-v]            - Do validation
//      [-NoEscape]     - Don't escape characters like '<' or '&'.
//      filename        - The path to the XML file to parse
//
//  These are non-case sensitive
//
//   Limitations:
//     1.  The encoding="xxx" clause in the XML header should reflect
//         the system local code page, but does not.
//     2.  Cases where the XML data contains characters that can not
//         be represented in the system local code page are not handled.
//
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  Local data
//
//  xmlFile
//      The path to the file to parser. Set via command line.
//
//  doValidation
//      Indicates whether validation should be done. The default is not
//      to validate, but -v overrides that.
//
//  doEscapes
//      Indicates whether special chars should be escaped in the output.
//      Defaults to doing escapes, -NoEscape overrides.
// ---------------------------------------------------------------------------
//static char*	xmlFile         = 0;
//static bool     doValidation    = false;
//static bool     doEscapes       = true;

static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
//void          outputContent(ostream& target, const DOMString &s);
void          usage();
//ostream& operator<<(ostream& target, const DOMString& toWrite);
ostream& operator<<(ostream& target, DOMNode& toWrite);


// ---------------------------------------------------------------------------
//
//  ostream << DOM_Node   
//
//                Stream out a DOM node, and, recursively, all of its children.
//                This function is the heart of writing a DOM tree out as
//                XML source.  Give it a document node and it will do the whole thing.
//
// ---------------------------------------------------------------------------

extern bool PrintWithXID;
extern XID_map *PrintXidMap;

ostream& operator<<(ostream& target, DOMNode &toWrite)
{
    if ((PrintWithXID)&&(PrintXidMap!=NULL)) {
        if ((toWrite.getNodeType()==DOMNode::TEXT_NODE)||(toWrite.getNodeType()==DOMNode::ELEMENT_NODE)) {
            target << "[XID=" << PrintXidMap->getXIDbyNode(&toWrite)<<"]";
	}
    }
    
    // Get the name and value out for convenience
    const XMLCh* nodeName = toWrite.getNodeName();
    const XMLCh* nodeValue = toWrite.getNodeValue();

	switch (toWrite.getNodeType())
    {
		case DOMNode::TEXT_NODE:
        {
          outputContent(target, nodeValue);
          
            break;
        }

        case DOMNode::PROCESSING_INSTRUCTION_NODE :
        {
            target  << "<?"
                    << XyLatinStr(nodeName).localForm()
                    << ' '
                    << XyLatinStr(nodeValue).localForm()
                    << "?>";
            break;
        }

        case DOMNode::DOCUMENT_NODE :
        {
            // Bug here:  we need to find a way to get the encoding name
            //   for the default code page on the system where the
            //   program is running, and plug that in for the encoding
            //   name.  
            target << "<?xml version='1.0' encoding='ISO-8859-1' ?>\n";
            DOMNode* child = toWrite.getFirstChild();
            while( child != 0)
            {
                target << *child << endl;
                child = child->getNextSibling();
            }

            break;
        }

        case DOMNode::ELEMENT_NODE :
        {
            // Output the element start tag.
            target << '<' << XyLatinStr(nodeName).localForm();

            // Output any attributes on this element
            DOMNamedNodeMap* attributes = toWrite.getAttributes();
            int attrCount = attributes->getLength();
            for (int i = 0; i < attrCount; i++)
            {
                DOMNode*  attribute = attributes->item(i);

                target  << ' ' << XyLatinStr(attribute->getNodeName()).localForm()
                        << "=\"";
                        //  Note that "<" must be escaped in attribute values.
                        outputContent(target, attribute->getNodeValue());
                        target << '"';
            }

            //
            //  Test for the presence of children, which includes both
            //  text content and nested elements.
            //
            DOMNode* child = toWrite.getFirstChild();
            if (child != 0)
            {
                // There are children. Close start-tag, and output children.
                target << ">";
                while( child != 0)
                {
                    target << *child;
                    child = child->getNextSibling();
                }

                // Done with children.  Output the end tag.
                target << "</" << XyLatinStr(nodeName).localForm() << ">";
            }
            else
            {
                //
                //  There were no children.  Output the short form close of the
                //  element start tag, making it an empty-element tag.
                //
                target << "/>";
            }
            break;
        }

        case DOMNode::ENTITY_REFERENCE_NODE:
        {
            DOMNode* child;
            for (child = toWrite.getFirstChild(); child != 0; child = child->getNextSibling())
                target << *child;
            break;
        }

        case DOMNode::CDATA_SECTION_NODE:
        {
            target << "<![CDATA[" << XyLatinStr(nodeValue).localForm() << "]]>";
            break;
        }

        case DOMNode::COMMENT_NODE:
        {
            target << "<!--" << XyLatinStr(nodeValue).localForm() << "-->";
            break;
        }

        default:
            cerr << "Unrecognized node type = "
                 << (long)toWrite.getNodeType() << endl;
    }
	return target;
}


// ---------------------------------------------------------------------------
//
//  outputContent  - Write document content from a DOMString to a C++ ostream.
//                   Escape the XML special characters (<, &, etc.) unless this
//                   is suppressed by the command line option.
//
// ---------------------------------------------------------------------------

void outputContent(ostream& target, const XMLCh* toWrite)
{
	XyLatinStr s(toWrite);
	for(unsigned int i=0; i<s.localFormSize(); i++) {
		char c = s.localForm()[i];
		switch (c) {
			case '&' : target << "&amp;"  ; break;
			case '<' : target << "&lt;"   ; break;
			case '>' : target << "&gt;"   ; break;
			case '\'': target << "&apos;" ; break;
			case '\"': target << "&quot;" ; break;
			default:   target << c ;
		}
	}
}
