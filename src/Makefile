EXES = testXyTranscode XyDiff XyDebugXid XyGenerateXidTaggedDocument XyCreateRandomDelta\
	XyDeltaApply XyCmpXML xydeltaCost mmdeltaCost testProgram testProgramLibs \
	xmlParseInfo XyLoad XyRetrieve XyCmpXidmap testLR testLoadAndRetrieve \
	perfXyTranscode testSeveralDiff

LIBS = XyDelta

COMMON_OBJECTS = convertUTF.o StringPusher.o \
                 ComputeDelta.o \
                 Diff_DeltaConstructor.o \
                 Diff_NodesManager.o \
                 Diff_UniqueIdHandler.o \
                 DeltaApply.o \
		 DeltaException.o \
                 DeltaManager.o \
		 DeltaReverse.o \
                 DeltaSortOperations.o \
                 easy_css.o \
                 lcss.o \
                 lookup2.o \
		 Tools.o \
		 XID_map.o \
		 XID_DOMDocument.o \
                 XyDeltaFileImpl.o \
                 XyDeltaDomImpl.o \
		 XyInt.o \
		 XyLatinStr.o \
		 XyLatinDocument.o \
                 XyStr.o \
		 XyUTF8Str.o \
		 XyUTF8Document.o


XyCmpXidmap_LOC_OBJS = $(COMMON_OBJECTS) XyCmpXidmap.o xyleme_DOMPrint.o

XyDelta_LOC_OBJS  = $(COMMON_OBJECTS)

XyDiff_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o execComputeDelta.o

XyDebugXid_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o testXidDomDocument.o
XyGenerateXidTaggedDocument_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o generateXidTaggedDocument.o
XyCreateRandomDelta_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o CreateRandomScript.o
XyDeltaApply_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o execDeltaApply.o
XyCmpXML_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o compareXML.o
xmlParseInfo_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o parseInfo.o

xydeltaCost_LOC_OBJS =$(COMMON_OBJECTS) xyleme_DOMPrint.o xydeltaCost.o
mmdeltaCost_LOC_OBJS =$(COMMON_OBJECTS) xyleme_DOMPrint.o mmdeltaCost.o

XyLoad_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o XyLoad.o

XyRetrieve_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o XyRetrieve.o

testProgram_LOC_OBJS = testProgram.o
testXyTranscode_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o testXyTranscode.o 
perfXyTranscode_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o perfXyTranscode.o 
testProgramLibs_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o testProgramLibs.o
testSeveralDiff_LOC_OBJS = $(COMMON_OBJECTS) xyleme_DOMPrint.o testSeveralDiff.o

testLR_LOC_OBJS = testLR.o

testLoadAndRetrieve_LOC_OBJS = testLoadAndRetrieve.o

XyDiff_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyDebugXid_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyGenerateXidTaggedDocument_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyCreateRandomDelta_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyDeltaApply_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyCmpXML_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyCmpXidmap_OTHER_LIBS = $(GEN_OTHER_LIBS)
xydeltaCost_OTHER_LIBS = $(GEN_OTHER_LIBS)
mmdeltaCost_OTHER_LIBS = $(GEN_OTHER_LIBS)
testProgram_OTHER_LIBS = $(GEN_OTHER_LIBS)
xmlParseInfo_OTHER_LIBS = $(GEN_OTHER_LIBS)
XyLoad_OTHER_LIBS =$(GEN_OTHER_LIBS)
XyRetrieve_OTHER_LIBS = $(GEN_OTHER_LIBS)
testLR_OTHER_LIBS = $(GEN_OTHER_LIBS)
testLoadAndRetrieve_OTHER_LIBS = $(GEN_OTHER_LIBS)
testXyTranscode_OTHER_LIBS = $(CONFIG_OTHER_LIBS)
perfXyTranscode_OTHER_LIBS = $(GEN_OTHER_LIBS)
testProgramLibs_OTHER_LIBS = $(CONFIG_OTHER_LIBS)
testSeveralDiff_OTHER_LIBS = $(CONFIG_OTHER_LIBS)

XyDiff_XY_LIBS = $(CONFIG_XY_LIBS)
XyDebugXid_XY_LIBS = $(CONFIG_XY_LIBS)
XyGenerateXidTaggedDocument_XY_LIBS = $(CONFIG_XY_LIBS)
XyCreateRandomDelta_XY_LIBS = $(CONFIG_XY_LIBS)
XyDeltaApply_XY_LIBS = $(CONFIG_XY_LIBS)
XyCmpXML_XY_LIBS = $(CONFIG_XY_LIBS)
XyCmpXidmap_XY_LIBS = $(CONFIG_XY_LIBS)
xydeltaCost_XY_LIBS = $(CONFIG_XY_LIBS)
mmdeltaCost_XY_LIBS = $(CONFIG_XY_LIBS)
testProgram_XY_LIBS = $(CONFIG_XY_LIBS)
xmlParseInfo_XY_LIBS = $(CONFIG_XY_LIBS)
XyLoad_XY_LIBS =$(CONFIG_XY_LIBS)
XyRetrieve_XY_LIBS = $(CONFIG_XY_LIBS)
testLR_XY_LIBS = $(CONFIG_XY_LIBS)
testLoadAndRetrieve_XY_LIBS = $(CONFIG_XY_LIBS)
testXyTranscode_XY_LIBS = $(CONFIG_XY_LIBS)
perfXyTranscode_XY_LIBS = $(CONFIG_XY_LIBS)
testProgramLibs_XY_LIBS = $(CONFIG_XY_LIBS)
testSeveralDiff_XY_LIBS = $(CONFIG_XY_LIBS)

# XyDiff XyDebugXid XyGenerateXidTaggedDocument XyCreateRandomDelta\
#XyDeltaApply XyCmpXML xydeltaCost mmdeltaCost testProgram \
#xmlParseInfo XyLoad XyRetrieve testLR 

#_EXT_OBJS = 

#_XY_LIBS = 

#_OTHER_LIBS =


include Make.xydiff
