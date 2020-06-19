#ifndef _GPXOUT_H
#define _GPXOUT_H
//
// Ingests GPSSample vectors and outputs GPX Track files
// Has abilities to combine and trim and segregate samples
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

// std::min and std::max
//#include <algorithm>

#include "goprometa.h"
#include "xmlwriter/xmlwriter.h"

class SamplesHandler {
public:
	SamplesHandler();
	~SamplesHandler();
	bool AddSampleSet(const char* keyname, std::vector<GPSSample> samples);
	void ExportDataGroupDailySegmented(std::map<std::string, std::map<std::string, std::vector<GPSSample>>> &outGroups);
protected:
	std::map<std::string, std::vector<GPSSample>> trackGroups;	// All sample groups mapped by filename individually
};

class GPXExporter {
public:
	GPXExporter(SamplesHandler* _pHandler);
	~GPXExporter();
	bool ExportDailySegmented(const char* destdir="");
protected:
	SamplesHandler* pHandler;
};

#endif

