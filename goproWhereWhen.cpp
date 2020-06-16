//
// System for extracting GoPro GPMETA data from MP4 files
//
// Target is GPS lat, lon, and elevation as well as GPSU UTC time information
//
// System allows for a variety of sampling rates as well as batch file processing
// flexibility.
//
// Export of data can be had as CSV file(s) or GPX files to be imported elsewhere
//
// Initial focus in 2020 was post-processing video taken on motorcycle adventure
// trips to know where I was when certain video files were taken to enable looking
// back at a bit of video and being able to pinpoint a location "post-ride"
//
// Author: Robert M. Wolff - bob dawt wolff 68 aht gmail.com
//

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <set>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <dirent.h>

#include "getopt/getopt.hpp"

using namespace std;

#define PROG_NAME "goproWhereWhen"
#define PROG_VER  "0.5"

class opts {
public:
	opts() {
		logFileName="";
		sourceDir="";
		sourceDirRecursive = false;
	};
	~opts() {};
	void processOpts(int argc, const char** argv) {
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

	    if( args.has("--sourcedir") ) {
	        sourceDir = args["--sourcedir"];
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

	void showHelp() {
		cout << "goproWhereWhen Usage:" << endl
			<< " --help : Print this message." << endl
			<< " --version : Print product version." << endl
			<< " --sourcedir=<directory> : Process MP4 files from this location." << endl
			<< " --recursive : Process sourcedir and all directories under it. (default: false)" << endl
			<< endl;
	};

	// Flags and option values
	string logFileName;
	string sourceDir;
	bool sourceDirRecursive;

};

opts options;

const char* addToPath(const char* start, const char* add) {
	string path(start);

	if (path.back() != '/')
		path += '/';

	path += add;

	return path.c_str();
}

void getAllFilesFromPath(const char* inPath, bool bRecurse, vector<string>& files) {
  	vector<string> dirs;
	DIR *dir;
	struct dirent *dirItem;

	files.clear();
	dirs.clear();
  	dirs.push_back(inPath);

  	while (!dirs.empty()) {
//  		cout << "TOP: dirs.size()=" << dirs.size() << endl;
  		string curPath = dirs.back();
  		dirs.pop_back();
  		cout << "Processing dir: " << curPath << endl;

  		dir = opendir(curPath.c_str());
  		// dir stream is now open and ready for business.
  		if (!dir) {
  			cout << "ERROR: Directory could not be opened: " << curPath << endl;
  			exit(-1);
  		}

  		dirItem = readdir(dir);
  		while (dirItem) {
  			// Will receive all file and dir entries including . and ..

  			if (!(!strcmp(dirItem->d_name, ".") || !strcmp(dirItem->d_name, "..")))
  			{
	  			if (dirItem->d_type == DT_REG) {
		  			cout << "FILE: " << dirItem->d_name << endl;
		  			files.push_back(addToPath(curPath.c_str(), dirItem->d_name));
	  			}
	  			// Only add directories in if we're in recursive mode.
	  			else if (bRecurse && dirItem->d_type == DT_DIR) {
		  			cout << " DIR: " << dirItem->d_name << endl;
		  			dirs.push_back(addToPath(curPath.c_str(), dirItem->d_name));
	  			}
	  		}

	  		// Next entry please.
  			dirItem = readdir(dir);
  		}

  		// Done with this directory. Close it.
		closedir(dir);
//  		cout << "BOTTOM: dirs.size()=" << dirs.size() << endl;

  	}
}

int main(int argc, const char** argv)
{
  vector<string> files;
  options.processOpts(argc, argv);

  if (options.sourceDir != "") {
  	getAllFilesFromPath(options.sourceDir.c_str(), options.sourceDirRecursive, files);

  	cout << "All Files found:" << endl;
  	for (auto it: files)
  		cout << " " << it << endl;
  }

  return 0;
}

