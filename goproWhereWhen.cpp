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
#include <stdio.h>
#include <stdlib.h>

//#define GETOPT_BUILD_DEMO
#include "getopt/getopt.hpp"

#if 1
int main(int argc, char** argv)
{
	// - No initialization required: (argc, argv) pair automatically retrieved.
	// - First argument is default option value, then all option indentifiers follow.
    bool help = getarg( false, "-h", "--help", "-?" );
    int version = getarg( 0, "-v", "--version", "--show-version" );
    int depth = getarg( 1, "-d", "--depth", "--max-depth");
    std::string file = getarg( "", "-f", "--file" );
    // [...]
    std::cout << help << ',' << file << ',' << version << std::endl;

  return 0;
}

#endif
