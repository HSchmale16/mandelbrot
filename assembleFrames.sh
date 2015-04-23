#!/bin/bash
# Assembles the generated frames into a movie using ffmpeg
# Henry Schmale
# April 23, 2015

ffmpeg -framerate 5 -i 'out/frame%03d.jpg' -c:v libx264 -vf fps=60 mandel.mp4
