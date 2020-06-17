#ifndef _OPT_H
#define _OPT_H

#include <iostream>
#include <string>
#include <vector>

#include <wordexp.h>

#include "getopt/getopt.hpp"

#define PROG_NAME "goproWhereWhen"
#define PROG_VER  "0.5"

//
// class to handle options and setup for main program
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//
class opts {
public:
	opts();
	~opts();
	void processOpts(int argc, const char** argv);
	void showHelp();
	bool expandPath(const char* inPath, std::string &expandedPath);

	// Flags and option values
	std::string logFileName;
	std::string sourceDir;
	bool sourceDirRecursive;
	std::vector<std::string> fileExtList;
	std::string fileExtRaw;
	std::string inFile;
	unsigned int timeBetweenSamples;

};
#endif
