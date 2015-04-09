#include "ParserUtil.h"

int main( int argc, char* argv[] ) {

    if( argc < 2 ) {
        printf( "No input file.\n" );
        return 0;
    }

    const char* file = argv[1];
    printf( "Input file: %s\n", file );

    /* parse file */
    W3eData w3eData = parse( file );
    printf( "tilepoint pointer: %p\n", w3eData.tps );

    int *size = w3eData.size;
    int numtp = size[0] * size[1];

    /* image buffer */
    int w = size[0];
    int h = size[1];
    unsigned char data[w*h*3];

    /* write height map image*/ 
    for( int i=0; i<numtp; i++ ) {

        tilepoint tp = w3eData.tps[i];

        /* red is empty */
        data[3*i] = 0;   

        /* green shows ramps */
        data[3*i+1] = ( tp.flags & 0x0010 ) ? 255 : 0;   

        /* blue shows height */
        data[3*i+2] = ( (float)tp.layerH/w3eData.maxLayer * 255 );
    }
    IMG_WRITE( "heightmap.png", w, h, data );

#define ABS(d) (((d)>0)?(d):(-(d)))
#define MAX(a,b) ((a)>(b)?(a):(b))

    /* write slope map image */
    unsigned char slope_data[w*h*3];
    for( int i=0; i<numtp; i++ ) {

        /* von Neumann neighbors */
        int tn_i = 3*(i-w)+2;
        int bn_i = 3*(i+w)+2;
        int ln_i = 3*(i-1)+2;
        int rn_i = 3*(i+1)+2;
        unsigned char tn = (i-w >= 0)    ? data[tn_i] : 0; // top
        unsigned char bn = (i+w < numtp) ? data[bn_i] : 0; // bot
        unsigned char ln = (i % w > 0)   ? data[ln_i] : 0; // left
        unsigned char rn = (i % w < w-1) ? data[rn_i] : 0; // right

        /* pixel */
        unsigned char px = data[3*i+2];

        unsigned char dtn = ABS(px-tn);
        unsigned char dbn = ABS(px-bn);
        unsigned char dln = ABS(px-ln);
        unsigned char drn = ABS(px-rn);

        unsigned char max;
        max = MAX(dtn,dbn);
        max = MAX(max,dln);
        max = MAX(max,drn);

        slope_data[3*i] = max;
        slope_data[3*i+1] = max;
        slope_data[3*i+2] = max;
    }
    IMG_WRITE( "slope.png", w, h, slope_data );
    
    /* write access map image */
    unsigned char ta = 27;
    unsigned char access_data[w*h*3];
    
    for( int i=0; i<numtp; i++ ) {

        unsigned char access = ( slope_data[3*i] <= ta ) ? 255 : 0;
        access_data[3*i] = access;
        access_data[3*i+1] = access;
        access_data[3*i+2] = access;
    }
    IMG_WRITE( "access.png", w, h, access_data );

    free( w3eData.tps );
    printf( "Done.\n" );
}

