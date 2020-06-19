#ifndef _GOPROMETA_H
#define _GOPROMETA_H
//
// class-based wrapper to the gpmf parser project from GoPro
// and pruned down to my specific use case around GPS data extraction
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//

#include <iostream>
#include <iomanip>
#include <vector>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>

#include "gpmf-parser/GPMF_parser.h"
#include "gpmf-parser/GPMF_mp4reader.h"

class TD {
public:
	TD();
	~TD();
	friend std::ostream& operator<<(std::ostream& os, const TD& dt);
	friend bool operator<(TD& lhs, TD& rhs);
	void readGPMeta(char* gp);
	int getSeconds();
	time_t getTime();
	std::string getDateOnly();
	bool isValid() { return bIsSet; };
	void setToCurrentTime();		// get current time and set values from there.
protected:
	void calcTime();
	int year, month, day, hour, minute, second;
	time_t theTime;
	bool bIsSet;
};

class GPSSample {
public:
	GPSSample(TD &_t, double _lat, double _lon, double _ele);
	~GPSSample();
	friend std::ostream& operator<<(std::ostream& os, const GPSSample& gs);
	friend bool operator<(GPSSample& lhs, GPSSample& rhs);
	time_t getTime() { return t.getTime(); };
	std::string getDateOnly();
	double getLat() { return lat; };
	double getLon() { return lon; };
	double getEle() { return ele; };
	TD getTD() { return t; };
protected:
	TD t;
	double lat, lon, ele;
};

class GoProMeta {
public:
	GoProMeta();
	~GoProMeta();
	void setSecondsBetweenSamples(unsigned int newtiming);
	bool openFile(const char* filename);
	bool processFile();
	void getOutputPoints(std::vector<GPSSample> &samps);

protected:
	bool processGPS5();
	bool processGPSU();
	void recordSample(double* ptr);
	void recordSampleIfAppropriate(double* ptr);

	size_t mp4;
	GPMF_stream metadata_stream, *ms;
	double metadatalength;
	uint32_t *payload;
	unsigned int secondsBetweenSamples;
	uint32_t numPayloads;
	TD currentTime;
	time_t nextSampleTime;
	std::vector<GPSSample> GPSSamples;
};

#endif
