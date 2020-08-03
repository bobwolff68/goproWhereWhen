//
// Ingests GPSSample vectors and outputs GPX Track files
// Has abilities to combine and trim and segregate samples
//
// Author: Robert Wolff
// Copyright 2020 Robert Wolff with MIT license
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <libgen.h>

#include "exporters.h"

using namespace	xmlw;

std::string rightStr(std::string input, size_t rKeep) {
	// Take rightmost 'chars' from input
	// Watch out for input being smaller than chars
	return input.substr(input.size() > rKeep ? input.size()-rKeep : 0, std::min(rKeep, input.size()));
}

SamplesHandler::SamplesHandler() {

}

SamplesHandler::~SamplesHandler() {

}

bool SamplesHandler::AddSampleSet(const char* keyname, std::vector<GPSSample> samples) {
	if (trackGroups.find(keyname) != trackGroups.end()) {
		std::cerr << "ERROR: keyname in use - SamplesHandler cannot add SampleSet named: " << keyname << std::endl;
		return false;
	}

	trackGroups[keyname] = samples;
	return true;
}

void SamplesHandler::makeUniqueBasenames(std::map<std::string, std::vector<GPSSample>> &inbound) {
	std::map<std::string, std::vector<GPSSample>> outbound;
	std::string nameInQuestion;

	// Iterate the inbound list.
	// Get the basename() for each entry and compare that against the outbound list
	// For any non-unique item, create a unique version.
	// Then put it into the outbound list.
	//
	// For any item that's found to be unique, simply copy it to the outbound list
	for (auto in: inbound) {
		unsigned int ver=1;

		nameInQuestion = basename((char*)in.first.c_str()); // Hopefully it's NOT a troubled name...

		if (outbound.find(nameInQuestion) == outbound.end()) {
			// It's unique. Didn't find it in the list so we're good to simply add it.
			outbound[nameInQuestion] = in.second;
		}
		else {
			// Must create unique version of troubledName
			// 
			do {
				std::string candidate;

				candidate = nameInQuestion + "-" + std::to_string(ver);

				// Check this one for uniqueness. If unique, add it and break;
				if (outbound.find(candidate) == outbound.end()) {
					outbound[candidate] = in.second;
					break;
				}
			} while(1);

		}
	}

	// In the end - copy back to the original.
	inbound = outbound;
}

void SamplesHandler::ExportDataGroupDailySegmented(
	std::map<std::string, std::map<std::string, std::vector<GPSSample>>> &outGroups) {
	// iterate the full map
	  // get the start date for entry
	  // check if the output has an entry for this date
	  //   No? create the date entry map[date]
	  // add this filename of samples into the map[date].map[filename]
	  // Need to make sure duplicate filenames are checked for and added with uniqueness to the final map

	makeUniqueBasenames(trackGroups); // Clean up trackGroups and strip path from keys

	for (auto inmap: trackGroups) {
		std::string fname = inmap.first;
		std::vector<GPSSample> samples = inmap.second;
		std::string trackDate;
		std::map<std::string, std::vector<GPSSample>> blank;

		// Shouldn't happen, but validate there's an entry here before accessing it.
		if (samples.size() == 0)
			continue;

		// Date from the first entry of the iterated trackGroups items.
		trackDate = samples[0].getDateOnly();

		// Quick status update of what's being iterated. #samples, datename, filename
//		std::cout << "Iterating: " << samples.size() << " samples on " << trackDate << " in: ..." << rightStr(fname, 35) << std::endl;

		if (outGroups.find(trackDate.c_str()) == outGroups.end()) {
			// No entry for this date yet.
			// Note there cannot be a collision of filename as it is a full-path filename

			// Without an entry yet, we'll insert a 'blank' entry and then insert our 'inmap'
			outGroups[trackDate] = blank;
			outGroups[trackDate].insert(inmap);
		}
		else {
			// date entry already exists. Add an entry directly to its existing map
			// No need to be worried about uniqueness in sub-list since all keys
			// are unique in the original map (by definition) and should already be basenames

			outGroups[trackDate].insert(inmap);
		}

	}

	std::cout << "outGroups[] has " << outGroups.size() << " unique date entries in it:" << std::endl;
	for (auto m: outGroups) {
		std::cout << " " << m.first << " has " << m.second.size() << " sets of samples." << std::endl;
	}
}

GPXExporter::GPXExporter(SamplesHandler* _pHandler) {
	pHandler = _pHandler;
}

GPXExporter::~GPXExporter() {

}

void tagGPXStartup(XmlStream &xml) {
	TD time;
	time.setToCurrentTime();

	// Assumes xml is already associated with a file that's open.
	xml << useIndentation(true) << prolog()
		<< tag("gpx")
		<< attr("xmlns") << "http://www.topografix.com/GPX/1/1"
		<< attr("creator") << "goproWhereWhen"
		<< attr("version") << "1.1"
		<< attr("xmlns:xsi") << "http://www.w3.org/2001/XMLSchema-instance"
		<< attr("xsi:schemaLocation") << "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd";

	xml << tag("metadata")
			<< tag("link") << attr("href") << "http://www.ridebdr.com"
				<< tag("text") << chardata() << "Robert Wolff - ADV Fan"
				<< endtag()
			<< endtag()
			<< tag("time") << chardata() << time
			<< endtag()
		<< endtag();	// metadata

}

bool GPXExporter::ExportDailySegmented(const char* destdir) {
	std::string prefix(destdir); // destdir can be empty or null - it's ok.
	std::map<std::string, std::map<std::string, std::vector<GPSSample>>> outGroups;

	pHandler->ExportDataGroupDailySegmented(outGroups);

/*
<gpx>
  <metadata>...</metadata>
  <trk>
    <name>Filename-Foldername or Foldername/Filename</name>
    <trkseg>
      <trkpt lat="47.644548" lon="-122.326897">
        <ele>4.46</ele>
        <time>2009-10-17T18:37:26Z</time>
      </trkpt>
*/
	for (auto dateGroup: outGroups) {
		// Each dateGroup is a file to export
		// Inside each entry, each sub-entry is a 'trk->name' with trkseg->trkpt
		std::string fnameDate;
		std::ofstream f;

		if (prefix != "")
			fnameDate = prefix + "/" + dateGroup.first + ".gpx";
		else
			fnameDate = dateGroup.first+".gpx";
		f.open(fnameDate);

		XmlStream xml(f);

		tagGPXStartup(xml);

		// Now iterate the entry for segments/tracks based on filename
		// Will need to shorten the filename and ensure uniqueness too
		for (auto segs: dateGroup.second) {
		    xml << tag("trk");
		      xml << tag("name")
		    	  << chardata() << segs.first << endtag();
		      xml << tag("trkseg");

		    // Now iterate the samples found here as trkpt
		    for (auto point: segs.second) {
		    	xml << tag("trkpt")
		    		<< attr("lat") << point.getLat()
		    		<< attr("lon") << point.getLon();

		    		xml << tag("ele")  << chardata() << point.getEle() << endtag();
		    		xml << tag("time") << chardata() << point.getTD() << endtag();
		    	xml << endtag(); // Finish trkpt
		    }

		    xml << endtag(); // Finish the trkseg
		    xml << endtag(); // Finish trk
		}

		xml << endtag(); // Finish gpx

		f.close();
	}

	return true;
}
