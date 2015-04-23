#include <complex>
#include <cstdio>
#include <cstdint>
#include <CImg.h>
#include <pthread.h>
#include <gmp.h>
#include <gmpxx.h>

using namespace cimg_library;

#define DX (XMAX-XMIN)
#define DY (YMAX-YMIN)

const int    SCR_WDTH = 1920;    //!< Width of the image generated
const int    SCR_HGHT = 1080;    //!< Height of the image generated
const int    SCR_CD   = 32;      //!< Bits of Color Depth of the screen
const int    FRAMES   = 40000;    //!< Total Number of frames to calculate
const int    MAX_ITER = 256;     //!< Maximum number of iter for mandelbrot
double       XMIN     = -1.438;
double       XMAX     = -1.400;
double       YMIN     = -(DX * ((double)SCR_HGHT/SCR_WDTH));
double       YMAX     = -YMIN;
double       ITER_SCL = .0005;

CImg<uint8_t> img(SCR_WDTH, SCR_HGHT, 1, 3);

struct pixel{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alpha;

    pixel(){
        r     = 0;
        g     = 0;
        b     = 0;
        alpha = 255;
    }

    template<typename TYP>
        pixel(TYP red, TYP gre, TYP blu, TYP alp){
            r     = red % 255;
            g     = gre % 255;
            b     = blu % 255;
            alpha = alp % 255;
        }
};

struct thread_data{
    double x;
};

pixel colorTable[256];
void generateColorTable(){
    srand(time(0));
    for(int i = 0; i < 256; i++){
        colorTable[i].r = rand() % 255;
        colorTable[i].g = rand() % 255;
        colorTable[i].b = rand() % 255;
    }
}

inline double map(double x, double in_min, double in_max, 
        double out_min, double out_max){
    return (x - in_min) * (out_max - out_min) / 
        (in_max - in_min) + out_min;
}


pixel mandelbrot(double x, double y){
    x = map(x, 0, SCR_WDTH, XMIN, XMAX);
    y = map(y, 0, SCR_HGHT, YMIN, YMAX);

    std::complex<double> c(x, y);
    std::complex<double> z = 0;
    unsigned int iters;
    int n = 0;

    for(iters = 0; iters < MAX_ITER && std::abs(z) < 2.0; ++iters) 
        z = z*z + c;

    n = (iters == MAX_ITER) ? 1 : iters;
    return colorTable[n % 256];
}

void* rendThrPt(void *d){
    thread_data *md;
    md = (thread_data*) d;
    for(int y = 0; y < SCR_HGHT; y++){
        pixel p = mandelbrot((double)md->x, (double)y);
        img(md->x, y, 0, 0) = p.r;
        img(md->x, y, 0, 1) = p.g;
        img(md->x, y, 0, 2) = p.b;
    }
    pthread_exit(NULL);
}

int main(){
    static pthread_t   threads[SCR_WDTH];
    static thread_data data[SCR_WDTH];
    int rc, x;
    double scale;

    generateColorTable();
    printf("Images are %dpx by %dpx\n", SCR_WDTH, SCR_HGHT);
    printf("X = (%.3f, %.3f)\n", XMIN, XMAX);
    printf("Y = (%.3f, %.3f)\n", YMIN, YMAX);
    for(int i = 0; i < FRAMES; i++){
        for(x = 0; x < SCR_WDTH; x++){
            data[x].x     = x;
            rc = pthread_create(&threads[x], NULL, rendThrPt, 
                    (void*)&data[x]);
            if(rc){
                printf("Couldn't Create Thread: %d\n", rc);
            }
        }
        for(x = 0; x < SCR_WDTH; x++){
            pthread_join(threads[x], NULL);
        }
        printf("(%010d) s=%.5f  (%.5f, %.5f)-(%.5f, %.5f)  (%.5f)(%.5f)\n",
               i, scale, XMIN, YMIN, XMAX, YMAX,
               XMAX - XMIN, YMAX - YMIN);
        //ITER_SCL *= ITER_SCL;
        scale = ((DX*ITER_SCL));
        XMIN = XMIN + scale;
        XMAX = XMAX - scale;
        YMIN = -(DX * ((double)SCR_HGHT/SCR_WDTH));
        YMAX = -YMIN;
        // Save the image for compositing later
        char fname[50];
        snprintf(fname, 50, "out/frame%010d.jpg", i);
        img.save_other(fname);
    }
}
