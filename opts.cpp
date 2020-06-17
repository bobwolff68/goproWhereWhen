//
// class to handle options and setup for main program
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//

#include "opts.h"

opts::opts() {
		logFileName="";
		sourceDir="";
		sourceDirRecursive = false;
		fileExtRaw = "MP4";
		fileExtList.clear();
		inFile="";
	};

opts::~opts() {};

bool opts::expandPath(const char* inPath, std::string &expandedPath) {
  int status;
  wordexp_t exp_result;
  status = wordexp(inPath, &exp_result, 0);  
  expandedPath = exp_result.we_wordv[0];
  wordfree(&exp_result);

  return status==0;
}

void opts::processOpts(int argc, const char** argv) {
	    struct getopt args( argc, argv );

	    if( args.has("-h") || args.has("--help") || args.has("-?") || args.size() == 1 ) {
	        showHelp();
	        exit(1);
	    }

	    if( args.has("--version") ) {
	        std::cout << args["0"] << PROG_NAME << " Version " << PROG_VER << " Compiled on " << __DATE__ << std::endl;
	        exit(2);
	    }

	    if ( args.has("--showargs") ) {
	   	    std::cout << "---" << std::endl;
		    std::cout << args.cmdline() << std::endl;
 			std::cout << args.size() << " MAP: " << args.str() << std::endl;
 			exit(3);
	    }

	    if( args.has("--infile") && (args.has("--sourcedir") || args.has("--fileext")) ) {
	        std::cout << "ERROR: --infile is mutually exclusive from --sourcedir and --fileext" << std::endl;
	        exit(-3);
	    }

	    if( args.has("--sourcedir") ) {
	    	if (!expandPath(args["--sourcedir"].c_str(), sourceDir)) {
	    		std::cout << "ERROR: Expansion of path failed: " << args["--sourcedir"] << std::endl;
	    		exit(-4);
	    	}
	    }

	    if( args.has("--fileext") ) {
	        fileExtRaw = args["--fileext"];
	    }

	    if( args.has("--infile") ) {
	        if (!expandPath(args["--infile"].c_str(), inFile)) {
	    		std::cout << "ERROR: Expansion of path failed: " << args["--infile"] << std::endl;
	    		exit(-5);
	        }
	    }

	    if( args.has("--recursive") ) {
	        sourceDirRecursive=true;
	    }

#if 0
    if( args.has("-d") || args.has("--depth") || args.has("--max-depth") ) {
        std::string arg = args["-d"];
        if( arg.empty() ) arg = args["--depth"];
        if( arg.empty() ) arg = args["--max-depth"];
        int depth = atoi( arg.c_str() );
        std::cout << "provided depth: " << depth << std::endl;
    }
#endif

};

void opts::showHelp() {
		std::cout << "goproWhereWhen Usage:" << std::endl
			<< " --help : Print this message." << std::endl
			<< " --version : Print product version." << std::endl
			<< " --sourcedir=<directory> : Process MP4 files from this location." << std::endl
			<< " --fileext=extlist [no '*' or '.' ... just extensions separated by commas (ie mp4,mov,mpeg)]" << std::endl
			<< " --infile=filename [process a singular input file - mutually exclusive from --sourcedir and --fileext]" << std::endl
			<< " --recursive : Process sourcedir and all directories under it. (default: false)" << std::endl
			<< std::endl;
	};

