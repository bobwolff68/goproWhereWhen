# goproWhereWhen
 Utility for pulling out all the GPS location and time information and exporting to CSV or GPX files en-batch

 System for extracting GoPro GPMETA data from MP4 files

 Initial focus in 2020 was post-processing video taken on motorcycle adventure
 trips to know where I was when certain video files were taken to enable looking
 back at a bit of video and being able to pinpoint a location "post-ride"

# Data gathered
 GoPro starting with the Hero 5 has had GPS and other telemetry data buried in the MP4 file.
 There is a rich set of data which can be harvested that includes GPS and time information.

 Target is to harvest GPS lattitude, longitude, and elevation as well as UTC time information.

# Flexibility
 System allows for a variety of sampling rates as well as batch file processing flexibility.
 Command line switches will allow post-processing large amounts of data into an organized output method.

 Export of data can be had as CSV file(s) or GPX files to be imported elsewhere

# Libraries utilized
 GoPro's gpmf-parser is the most important item as that body of work handles much of the heavy lifting
 to get the project off the ground and ready to extract information. I did fork their project to do 
 early experiments and prove that this would be possible. - https://github.com/bobwolff68/gpmf-parser

 Additionally, a simple XML writer library found on codeproject called 'xmlwriter' was used to facilitate
 being able to export the GPS data into the GPX format. 
 Also improved by fixing warnings and adding the ability to indent or not indent through
 the use of 'useIndentation(bool)' function during the stream (turn on and off dynamically)
 Author: Oboltus - https://www.codeproject.com/Articles/5588/Simple-C-class-for-XML-writing

 Getopt - https://github.com/r-lyeh-archived/getopt -- Oddly, on Mac I was unable to get the simpler
 getarg() method to function even in the sample (seems a compilation issue) but the OOP map based version
 works fine so that'll be what I use.

 GPX format is simple XML for these purposes and the file 'gpx-sample.xml' is similar to how this will output to that form.

# Options
** --help
** --version
 --logfile=
** --infile=
** --sourcedir=
** --recursive
 --destdir= | --output= (stdout if none specified)
** --fileext=  (default is .MP4 and .mp4 - list shall be comma separated without spaces, without '*' and without '.' globbing/regex. It is simply a list of file-endings. For instance, --fileext=mp4,mov,mpeg,mpg ... and note upper/lower case does not matter.
 --exportcsv
 --exportgpx
 --grouping=[dailysegmented|dailycombined|allcombined|individual] (default: dailysegmented)
     dailysegmented: filename is date, each trkseg name is filename (any/all points within a given day are in the single combined file ; In csv, there's a column for filename to allow differentiation/grouping)
     dailycombined: filename is date, trkseg name is date also (any/all points within a given day are in the single combined file) - 
     allcombined: filename is ??
     individual: each file is the original filename. Should the files be in $destdir/ and have the exact same heirarchy followed as the source file list??
 --maxsamples (For output file, limit the total samples in each file)
** --timebetweensamples
 --dryrun (dont actually process the files - just list them and show what would have been)


# Conclusion
 Author: Robert M. Wolff - bob dawt wolff 68 aht gmail.com
