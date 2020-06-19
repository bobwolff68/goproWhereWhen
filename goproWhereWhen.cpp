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
// Copyright Robert Wolff 2020 with MIT license
//

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <libgen.h>

#include "opts.h"
#include "goprometa.h"
#include "exporters.h"

using namespace std;

opts options;
SamplesHandler sHandler;

// Utils
extern	void getAllFilesFromPath(const char* inPath, bool bRecurse, vector<string>& files);
extern	bool validateFileExts(const char* endingList, vector<string> &destList);
extern	void pruneFilesList(vector<string> &filesInOut, vector<string> toKeep);

int main(int argc, const char** argv)
{
  vector<string> files;
  options.processOpts(argc, argv);

//#define UNITTEST
#ifdef UNITTEST
  unittest_ExtValidation();
  unittest_PruneFiles();
  exit(1);
#endif

//  exit(1);

  if (options.fileExtRaw != "") {
  	// Need to validate the input and place it into the options vector
  	if (!validateFileExts(options.fileExtRaw.c_str(), options.fileExtList)) {
  		// Invalid.
  		cout << "ERROR: Option '--fileext' expects to get a comma separated list of" << endl;
  		cout << "  file extensions. No '*' or '.' - just command separated extensions." << endl;
  		cout << "  Upper/Lower case does not matter. Extension comparison will be insensitive." << endl;
  		cout << "  Example: --fileext=MP4,MOV,MPEG" << endl;
  		exit(-2);
  	}

  	// Now we have a good extensions list.
  	// These will be applied to 'sourcedir' when searching.
  }

  if (options.sourceDir != "") {
  	getAllFilesFromPath(options.sourceDir.c_str(), options.sourceDirRecursive, files);

  	// Now let's apply the fileext results to this list to prune it down to our work items.
  	pruneFilesList(files, options.fileExtList);

  	std::cerr << "Final Files List:" << std::endl;
  	for (auto f: files)
	  	std::cout << f << std::endl;
  }

  if (options.inFile != "") {
  	files.clear();
  	files.push_back(options.inFile);
  }

  vector<GPSSample> samples;
  // At this stage, we have a list of input files that need processed.
  // It's time to decide how to rip through that list and what to do with the results.
  for (auto f: files) {
  	GoProMeta *pGPM = new GoProMeta();
  	samples.clear();

  	std::cerr << "Processing: " << f << std::endl;

  	pGPM->setSecondsBetweenSamples(options.timeBetweenSamples);

  	if (!pGPM->openFile(f.c_str())) {
//  		std::cerr << "ERROR: Could not open file: " << f << std::endl;
  		delete pGPM;
  		continue;
//  		exit(-6);
  	}

  	if (!pGPM->processFile()) {
  		std::cerr << "ERROR: Could not process file properly: " << f << std::endl;
  		delete pGPM;
  		continue;
//  		exit(-7);
  	}

  	pGPM->getOutputPoints(samples);
  	std::cerr << "For file " << f << " Number of samples is: " << samples.size() << std::endl;
  	if (samples.size())
  		std::cerr << " First entry: " << samples[0] << std::endl;

	// Insert samples into SamplesHandler for safe keeping
	if (!sHandler.AddSampleSet(basename((char*)f.c_str()), samples)) {
		std::cerr << "Could not add samples for: " << f << std::endl;
		// continuing...
	}

  	delete pGPM;
  }

  // Now that we have all the files processed, what shall we do with them?

  // Let's try 'exporting' in groups.
//	std::map<std::string, std::map<std::string, std::vector<GPSSample>>> groupedDates;
//	sHandler.ExportDataGroupDailySegmented(groupedDates);

	GPXExporter gpxOut(&sHandler);
	gpxOut.ExportDailySegmented();

  return 0;
}
