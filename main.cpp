#include <complex>
#include <cstdio>
#include <cstdint>
#include <CImg.h>
#include <pthread.h>

using namespace cimg_library;

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

CImg<uint8_t> img(1920, 1080, 1, 3);
const int SCR_WDTH = 1920;
const int SCR_HGHT = 1080;
const int SCR_CD   = 32;
const int ITERATS  = 1000;

pixel colorTable[256];
void generateColorTable(){
    srand(time(0));
    for(int i = 0; i < 256; i++){
        colorTable[i].r = rand() % 255;
        colorTable[i].g = rand() % 255;
        colorTable[i].b = rand() % 255;
    }
}

pixel mandelbrot(double x, double y, double zfactor){
    x = (-(SCR_WDTH/6) + x/(SCR_WDTH-1.0)*SCR_WDTH) * zfactor;
    y = (-(SCR_HGHT/8) + y/(SCR_HGHT-1.0)*SCR_HGHT) * zfactor;

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
    static double      scaleFactor = 1.0 + 1.0 / ITERATS;
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
        scaleFactor *= .9;
        // Save the image for compositing later
        char fname[50];
        snprintf(fname, 50, "out/frame%d.jpg", i);
        img.save_other(fname);
    }
}
