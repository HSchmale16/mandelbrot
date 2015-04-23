#include <complex>
#include <cstdio>
#include <cstdint>
#include <CImg.h>
#include <pthread.h>

using namespace cimg_library;

const int    SCR_WDTH = 1920;    //!< Width of the image generated
const int    SCR_HGHT = 1080;    //!< Height of the image generated
const int    SCR_CD   = 32;      //!< Bits of Color Depth of the screen
const int    ITERATS  = 1000;    //!< Total Number of frames to calculate
const double XMIN     = -2.000;
const double XMAX     =  2.000;
const double DX       = XMAX - XMIN;
const double YMIN     = -1.300;
const double YMAX     =  1.300;
const double DY       = XMAX - XMIN;
const double ITER_SCL = .95;     //!< Scale Value Multiplier Each Iteration

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
    double scale;
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
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


pixel mandelbrot(double x, double y, double zfactor){
    x = XMIN + map(x, 0, SCR_WDTH, 0, DX) * zfactor;
    y = YMIN + map(y, 0, SCR_HGHT, 0, DY) * zfactor;

    std::complex<double> c(x, y);
    std::complex<double> z = 0;
    unsigned int iters;
    int n = 0;

    for(iters = 0; iters < ITERATS && std::abs(z) < 2.0; ++iters) 
        z = z*z + c;

    n = (iters == ITERATS) ? 1 : iters;
    return colorTable[n % 256];
}

void* rendThrPt(void *d){
    thread_data *md;
    md = (thread_data*) d;
    for(int y = 0; y < SCR_HGHT; y++){
        pixel p = mandelbrot((double)md->x, 
                (double)y,
                md->scale);
        img(md->x, y, 0, 0) = p.r;
        img(md->x, y, 0, 1) = p.g;
        img(md->x, y, 0, 2) = p.b;
    }
    pthread_exit(NULL);
}

int main(){
    static double      scaleFactor = 1.0;
    static pthread_t   threads[SCR_WDTH];
    static thread_data data[SCR_WDTH];
    int rc, x;

    generateColorTable();
    for(int i = 0; i < ITERATS; i++){
        for(x = 0; x < SCR_WDTH; x++){
            data[x].scale = scaleFactor;
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
        printf("(%d) scale = %f\n", i, scaleFactor);
        scaleFactor *= ITER_SCL;
        // Save the image for compositing later
        char fname[50];
        snprintf(fname, 50, "out/frame%d.jpg", i);
        img.save_other(fname);
    }
}
