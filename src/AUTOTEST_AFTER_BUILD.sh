#!/bin/bash

mkdir TEST_DIR
cd TEST_DIR

TEST_EXE=../bin/testProgram
TESTS_SRC=../bin

LD_LIBRARY_PATH=/xyleme/tools/icu-2.1/GCC3_Linux/lib:/xyleme/tools/xerces-5.2/GCC3_Linux/lib:/opt/gcc-3.2.3/lib
export LD_LIBRARY_PATH
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"

# create local copy of exec
cp $TESTS_SRC/XyCreateRandomDelta .
cp $TESTS_SRC/XyDeltaApply .
cp $TESTS_SRC/XyDiff .
cp $TESTS_SRC/XyCmpXML .
cp $TESTS_SRC/XyCmpXidmap .

# create local copy of test XML file
cp $TESTS_SRC/../sample-data/example1.xml ./example.xml

$TEST_EXE

EXIT_CODE=$?
echo "Exit code was [$EXIT_CODE]"
exit $EXIT_CODE
