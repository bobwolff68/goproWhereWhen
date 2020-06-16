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
 being able to export the GPS data into the GPX format. Author: Oboltus - https://www.codeproject.com/Articles/5588/Simple-C-class-for-XML-writing

 Getopt - https://github.com/r-lyeh-archived/getopt -- Oddly, on Mac I was unable to get the simpler
 getarg() method to function even in the sample (seems a compilation issue) but the OOP map based version
 works fine so that'll be what I use.

 GPX format is simple XML for these purposes and the file 'gpx-sample.xml' is similar to how this will output to that form.

# Conclusion
 Author: Robert M. Wolff - bob dawt wolff 68 aht gmail.com
