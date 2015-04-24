#!/bin/bash
# Assembles the generated frames into a movie using ffmpeg
# Henry Schmale
# April 23, 2015

final_name=video.avi
frat_glob=out/frame%010d.jpg

# $1 - file glob pattern
# $2 - output file name
assembImg2Mp4(){
    if [ -e $2 ] ; then rm -f $2 ; fi
    ffmpeg -framerate 60 -i $1 -c:v libx264 -vf fps=60 $2
}

assembImg2Mp4 $frat_glob $final_name


