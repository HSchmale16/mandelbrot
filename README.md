# mandelbrot
Mandelbrot Fractel Video generation system.

Spits out a bunch of pictures of the mandelbrot fractal being zoomed in on,
that can be combined with ffmpeg or similar tool into a smooth video.
It uses a fully threaded algorithim to calculate each row of the image,
with about 1000 threads depending on the desired resolution of the output 
image.


## Depends
* CImg Library
* pthread

