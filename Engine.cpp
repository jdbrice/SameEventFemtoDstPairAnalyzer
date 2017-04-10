

// RooBarb
#include "XmlConfig.h"
#include "TaskEngine.h"
#include "SharedTreeAnalyzer.h"
using namespace jdb;

// STL
#include <iostream>
#include <exception>


#include "FemtoDstReader/FemtoDstReader.h"

#define LOGURU_IMPLEMENTATION 1
#include "vendor/loguru.h"

int main( int argc, char* argv[] ) {

	TaskFactory::registerTaskRunner<FemtoDstReader>( "FemtoDstReader" );

	TaskEngine engine( argc, argv );

	return 0;
}
