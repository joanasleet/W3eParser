#include "serializer.h"

int main( int argc, char* argv[] ) {

    if( argc < 2 ) {
        printf( "No input file.\n" );
        return 0;
    }

    char* file = argv[1];
    printf( "Input file: %s\n", file );

    FILE* f = fopen( file, "rb" );

    if( !f ) {
        printf( "Cannot open %s\n", file );
        return 1;
    }
    FCLEAN();

    /* parse header */
    char fid[4+1];
    fid[4] = '\0';
    FREAD( 4, char, f, fid );
    FPRINT( "File ID: %s\n", fid );

    int fver[1];
    FREAD( 1, int, f, fver );
    FPRINT( "File Version: %d\n", fver[0] );

    char ts[1];
    FREAD( 1, char, f, ts );
    FPRINT( "Main tileset: %c\n", ts[0] );

    int cts[1];
    FREAD( 1, int, f, cts );
    FPRINT( "Custom tileset: %d\n", cts[0] );

    int a[1];
    FREAD( 1, int, f, a );
    FPRINT( "Ground tileset: %d\n", a[0] );

    char gtsIDs[ 4*a[0]+1 ];
    gtsIDs[4*a[0]] = '\0';
    FREAD( 4*a[0], char, f, gtsIDs );
    FPRINT( "Ground tileset IDs: %s\n", gtsIDs );

    int b[1];
    FREAD( 1, int, f, b );
    FPRINT( "Cliff tileset: %d\n", b[0] );

    char cltsIDs[ 4*b[0]+1 ];
    cltsIDs[4*b[0]] = '\0';
    FREAD( 4*b[0], char, f, cltsIDs );
    FPRINT( "Cliff tileset IDs: %s\n", cltsIDs );

    int dims[2];
    FREAD( 2, int, f, dims );
    FPRINT( "Map size: %dx%d\n", dims[0], dims[1] );

    float center[2];
    FREAD( 2, float, f, center );
    FPRINT( "Center offset: (%.1f, %.1f)\n", center[0], center[1] );

    /* parse tilepoints */
    int numtp = dims[0] * dims[1];

    /* max height data */
    char maxLayer = 0;
    float maxHeight = 0.0f;

    /* image buffer */
    int w = dims[0];
    int h = dims[1];
    unsigned char data[w*h*3];

    /* vertexHeight buffer */
    char vertH[numtp];

    /* height data */ 
    for( int i=0; i<numtp; i++ ) {

        short ground;
        FREAD( 1, short, f, &ground );

        short water;
        FREAD( 1, short, f, &water );

        char flags;
        FREAD( 1, char, f, &flags );
        //char* flagsStr = tobin( flags ); // binary string
        //free( flagsStr );

        char texture;
        FREAD( 1, char, f, &texture );

        char layer;
        FREAD( 1, char, f, &layer );
        char layerHeight = (layer & 0x0f);
        if( layerHeight > maxLayer ) maxLayer = layerHeight;

        float absHeight = (ground-0x2000+((layer & 0x0f)-1)*0x0200)/4.0f;
        if( absHeight > maxHeight ) maxHeight = absHeight;
        
        /* serialize height data */
        //FPRINT( "( %.0f | %d )", absHeight, layerHeight );
        //FPRINT( "%s", ( i+1 % dims[0] == 0 ) ? "\n" : " " );

        /* write pixel data */
        data[3*i] = 0;                              // R
        data[3*i+1] = (flags & 0x0010) ? 255 : 0;   // G
        vertH[i] = layerHeight;                       // B ( normalized further down )
    }
    fclose( f );
    FPRINT( "Max Height: %.1f\n", maxHeight );
    FPRINT( "Max Layer: %d\n", maxLayer );

    /* normalize image data */
    for( int i=0; i<numtp; i++ ) {

        data[3*i+2] = ( (float)vertH[i] / maxLayer* 255 );
    }
    IMG_WRITE( "heightmap.png", w, h, data );

#define ABS(d) (((d)>0)?(d):(-(d)))
#define MAX(a,b) ((a)>(b)?(a):(b))

    /* slope map */
    unsigned char maxslope = 0;
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

        unsigned char px = data[3*i+2]; // pixel

        //printf( "\t%d\n", tn );
        //printf( "%d\t%d\t%d\n", ln, px, rn );
        //printf( "\t%d\n\n", bn );

        unsigned char dtn = ABS(px-tn);
        unsigned char dbn = ABS(px-bn);
        unsigned char dln = ABS(px-ln);
        unsigned char drn = ABS(px-rn);
        //printf( "dt neighbor: ( %d, %d, %d, %d )\n\n", dtn, dbn, dln, drn );

        unsigned char max;
        max = MAX(dtn,dbn);
        max = MAX(max,dln);
        max = MAX(max,drn);

        maxslope = ( maxslope < max ) ? max : maxslope;

        unsigned char nmax = ((float)max/247.0 * 255);
        slope_data[3*i] = nmax;
        slope_data[3*i+1] = nmax;
        slope_data[3*i+2] = nmax;
    }
    printf( "MaxSlope: %d\n", maxslope );
    IMG_WRITE( "slope.png", w, h, slope_data );

    unsigned char access_data[w*h*3];
    unsigned char Ta = 27;
    for( int i=0; i<numtp; i++ ) {
        unsigned char access = ( slope_data[3*i] <= Ta ) ? 255 : 0;
        access_data[3*i] = access;
        access_data[3*i+1] = access;
        access_data[3*i+2] = access;
    }
    IMG_WRITE( "access.png", w, h, access_data );


    printf( "Done.\n" );
}

