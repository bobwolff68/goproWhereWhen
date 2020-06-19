//
// class-based wrapper to the gpmf parser project from GoPro
// and pruned down to my specific use case around GPS data extraction
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//

#include "goprometa.h"

// Support for osstringstream
#include <sstream>

const unsigned int DEFAULT_TIMING = 5;

GoProMeta::GoProMeta() {
	payload = NULL;
	mp4 = 0;
	ms = &metadata_stream;
	secondsBetweenSamples = DEFAULT_TIMING;
	metadatalength = 0.0;
	numPayloads = 0;
	GPSSamples.clear();
	nextSampleTime = 0;
}

GoProMeta::~GoProMeta() {
	if (payload)
		FreePayload(payload);

	if (mp4)
		CloseSource(mp4);
}

//
// input of 0 literally will mean that every sample should be kept in the output database
//
void GoProMeta::setSecondsBetweenSamples(unsigned int newtiming) {
	secondsBetweenSamples = newtiming;
}

bool GoProMeta::openFile(const char* filename) {

	if (!filename)
		return false;

	mp4 = OpenMP4Source((char*)filename, MOV_GPMF_TRAK_TYPE, MOV_GPMF_TRAK_SUBTYPE);
	if (mp4 == 0)
	{
		std::cerr << "error: " << filename << " is an invalid MP4/MOV or it has no GPMF data" << std::endl;
		return false;
	}

	metadatalength = GetDuration(mp4);
	if (metadatalength > 0.0)
	{
		numPayloads = GetNumberPayloads(mp4);
	}
	else
		return false;

	return true;
}

bool GoProMeta::processFile() {

  if (mp4==0 || metadatalength==0.0 || numPayloads==0) {
  	std::cerr << "ERROR: missing critical source objects in processFile: " 
  		<< mp4 << " " << metadatalength << " " << numPayloads << std::endl;
  	return false;
  }

  for (uint32_t index = 0; index < numPayloads ; index++) {
	uint32_t payloadsize = GetPayloadSize(mp4, index);
	int32_t ret;

	payload = GetPayload(mp4, payload, index);
	if (payload == NULL) {
		std::cerr << "ERROR: Could not find payload on index:" << index << std::endl;
		return false;
	}

	ret = GPMF_Init(ms, payload, payloadsize);
	if (ret != GPMF_OK) {
		std::cerr << "ERROR: Could not GPMF_Init with payloadsize: " << payloadsize << std::endl;
		return false;
	}

	// Now we've got the sample and we've begun parsing it for GPMF.
	// Iterate through the sample and process anything we're interested in.
	do
	{
		switch(GPMF_Key(ms))
		{
		case STR2FOURCC("GPS5"):
		  // GPS data samples
		  if (!processGPS5()) {
		  	std::cerr << "ERROR: Failed to process GPS5 data." << std::endl;
		  	return false;
		  }
		  break;
	  
		case STR2FOURCC("GPSU"): 
		  // GPS UTC time
		  if (!processGPSU()) {
		  	std::cerr << "ERROR: Failed to process GPSU data." << std::endl;
		  	return false;
		  }
		  break;
	  
		default: // if you don’t know the Key you can skip to the next
		  break;
		}
	} while (GPMF_OK == GPMF_Next(ms, GPMF_RECURSE_LEVELS)); // Scan through all GPMF data

	// RMW: I believe this is unnecessary as GPMF_Init() calls GPMF_ResetState(),
	//      but I'll leave it in per the example in case there's some intertwined case I can't see.
	GPMF_ResetState(ms);
  }

  return true;
}

bool GoProMeta::processGPS5() {
	uint32_t samples = GPMF_Repeat(ms);
	uint32_t elements = GPMF_ElementsInStruct(ms);
	uint32_t buffersize = samples * elements * sizeof(double);

	double *ptr, *tmpbuffer = (double *)malloc(buffersize);

	if (!tmpbuffer)
		return false;

	if (samples)
	{
		// Go get the data and scale it to
		GPMF_ScaledData(ms, tmpbuffer, buffersize, 0, samples, GPMF_TYPE_DOUBLE);  //Output scaled data as floats

		ptr = tmpbuffer;

		// For sampling -- if secondsBetweenSamples is ZERO, we will record all samples.
		// However, otherwise we need to see if it is time to sample and if it is,
		//   we'll ONLY sample the initial one in the batch.
		// It is my understanding that this 'batch' of samples represents the one-second
		// group of 18 in the 18 Hz GPS sampling. A study of the real world data bears this out.
		if (secondsBetweenSamples) {
			recordSampleIfAppropriate(ptr);
		}
		else {
			// Record every sample
			for (uint32_t i = 0; i < samples; i++)
			{
				recordSample(ptr);

				ptr += elements; 		// Advance the pointer to the next sample.
			}
		}

		free(tmpbuffer);
	}

	return true;
}

bool GoProMeta::processGPSU() {
	char* pUTC;

   	pUTC = (char*)GPMF_RawData(ms);
   	if (!pUTC)
   		return false;

   	// UTC Time format yymmddhhmmss.sss 
   	currentTime.readGPMeta(pUTC);

//   	std::cout << "GPSU Decoded: DateTime: " << currentTime << std::endl;
   	return true;
}

void GoProMeta::recordSample(double* ptr) {
	double dLat = *ptr;
	double dLon = *(ptr+1);
	double dEle = *(ptr+2);

//	std::cout << "recording sample: " << currentTime << " " << dLat << " " << dLon << " " << dEle << std::endl;
	GPSSamples.emplace_back(currentTime, dLat, dLon, dEle);
}

void GoProMeta::recordSampleIfAppropriate(double* ptr) {
	// Let's make sure we don't record any GPS points until a GPSU valid time comes in.
	if (currentTime.isValid() && currentTime.getTime() >= nextSampleTime) {
		recordSample(ptr);

		// Bump forward our next time to take a snapshot
		nextSampleTime = currentTime.getTime() + secondsBetweenSamples;
	}
}

void GoProMeta::getOutputPoints(std::vector<GPSSample> &samps) {
	samps = GPSSamples; // Simply copy the big vector out to the caller.
}


GPSSample::GPSSample(TD &_t, double _lat, double _lon, double _ele) {
	t = _t;
	lat = _lat;
	lon = _lon;
	ele = _ele;
}

GPSSample::~GPSSample() {};

std::string GPSSample::getDateOnly() {
	return t.getDateOnly();
}

std::ostream& operator<<(std::ostream& os, const GPSSample& gs)
{
	os << "time: " << gs.t << ", " << "lat: " << gs.lat << ", lon: " << gs.lon << ", ele: " << gs.ele;
	return os;
}

bool operator<(GPSSample& lhs, GPSSample& rhs)
{
	return (lhs.getTime() < rhs.getTime());
}

TD::TD() {
			year = 2000;
			month = day = 1;
			hour = 12;
			minute = second = 0;
			theTime = 0;
			calcTime();
			bIsSet=false;
}

TD::~TD() {

}

void TD::calcTime() {
  struct tm t;

  // Load the structure with our time.
  t.tm_sec=second;
  t.tm_min=minute;
  t.tm_hour=hour;
  t.tm_mday=day;
  t.tm_mon=month;
  t.tm_year=year-1900;
  t.tm_wday=0;
  t.tm_yday=0;
  t.tm_isdst=0;  

  theTime = mktime(&t);
}

time_t TD::getTime() {
  if (theTime==0)
  	calcTime();

  return theTime;
}

std::string TD::getDateOnly() {
	std::ostringstream os;

	os << year << "-" << month << "-" << day;
	return std::string(os.str()); 
}

std::ostream& operator<<(std::ostream& os, const TD& dt)
{
    os  << dt.year << '-' 
    	<< std::setfill('0') << std::setw(2) << dt.month << '-' 
    	<< std::setfill('0') << std::setw(2) << dt.day << 'T'
    	<< std::setfill('0') << std::setw(2) << dt.hour << ':'
    	<< std::setfill('0') << std::setw(2) << dt.minute << ':'
    	<< std::setfill('0') << std::setw(2) << dt.second << 'Z';
    return os;
}

void TD::readGPMeta(char* gp) {
   	// UTC Time format yymmddhhmmss.sss 
   	year  = 2000 + 10*(gp[0]-'0') + (gp[1]-'0');
   	month = 10*(gp[2]-'0') + (gp[3]-'0');
   	day   = 10*(gp[4]-'0') + (gp[5]-'0');
   	hour  = 10*(gp[6]-'0') + (gp[7]-'0');
   	minute= 10*(gp[8]-'0') + (gp[9]-'0');
   	second= 10*(gp[10]-'0')+ (gp[11]-'0');	

   	bIsSet=true;
   	calcTime();	
}

int TD::getSeconds() { return second; };

bool operator<(TD& lhs, TD& rhs)
{
	return (lhs.getTime() < rhs.getTime());
}
