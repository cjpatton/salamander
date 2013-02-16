Christopher Patton
John Muir Institute for the Environment
University of California, Davis

This directory contains the source code for the Salamander image processing 
project. The purpose of this code is to detect and track a target in a raw
video stream. This applies to surveillance footage of animals in their natural 
habitats, eg. mice, rats, or salamanders. 

Files
-----
detect.cxx            -- top level program
filter.cxx            -- apply filters to a series of images
threshold.cxx         -- for collecting statistics
binary_threshold.cxx  -- binthresh
binary_morphology.cxx -- binmorph
CMakeLists.txt        -- for cmake 
salamander.[cxx/h]    -- library implementation of the image processing
ex                    -- some example footage for trying these programs


Installation
------------
This code is written in C++ for the Insight Segmentation and Restoration 
Toolkit (ITK). Information about ITK can be found at www.itk.org. You will need
it and cmake installed to build the code. 

 $ mkdir build && cd build
 $ cmake ../
 $ make 
 $ sudo make install

This will produce a shared library called "libsalamander.so" as well as 
four executable programs. The purpose of these programs is described in detail
in the code: 

 binthresh,
 binmorph,
 threshold,
 filter, and
 detect.


Usage
-----
All five programs take a list of JPEG files corresponding to a video stream
on standard input. binmoroph and binthresh apply image processing filters 
and output the result. threshold outputs two data files with statistical 
information about the video stream. filter is the main program that outputs a 
list of files in which target is captured on standard output. For example, 
assuming you have a folder of image files, run: 

 $ ls *.jpg > raw
 $ filter -t 20 60 -m 1 10 -s 2 < raw

The first two arguments refer to the erosion and dilation factors (binary 
morphology) respectively. The second two are optional and specify the range for
the binary threshold. The other programs can be run similarly:
 
 $ binthresh 40 60 < raw
 $ binmorph 2 20 < raw
 $ threshold 2 8 40 60 < raw

