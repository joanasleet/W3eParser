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

    /* parse header */
    char fid[4+1];
    FREAD( 4, char, f, fid );
    fid[4] = '\0';
    printf( "File ID: %s\n", fid );

    int fver[1];
    FREAD( 1, int, f, fver );
    printf( "File Version: %d\n", fver[0] );

    char ts[1];
    FREAD( 1, char, f, ts );
    printf( "Main tileset: %c\n", ts[0] );

    int cts[1];
    FREAD( 1, int, f, cts );
    printf( "Custom tileset: %d\n", cts[0] );

    int a[1];
    FREAD( 1, int, f, a );
    printf( "Ground tileset: %d\n", a[0] );

    char gtsIDs[ 4*a[0]+1 ];
    FREAD( 4*a[0], char, f, gtsIDs );
    gtsIDs[4*a[0]] = '\0';
    printf( "Ground tileset IDs: %s\n", gtsIDs );

    int b[1];
    FREAD( 1, int, f, b );
    printf( "Cliff tileset: %d\n", b[0] );

    char cltsIDs[ 4*b[0]+1 ];
    FREAD( 4*b[0], char, f, cltsIDs );
    cltsIDs[4*b[0]] = '\0';
    printf( "Cliff tileset IDs: %s\n", cltsIDs );

    int dims[2];
    FREAD( 2, int, f, dims );
    printf( "Map size: %dx%d\n", dims[0], dims[1] );

    float center[2];
    FREAD( 2, float, f, center );
    printf( "Center offset: (%f, %f)\n", center[0], center[1] );

    /* parse tilepoints */
    int numtp = dims[0] * dims[1];

    float maxHeight = 0.0f;
    char maxLayer = 0;
    
    /* image */
    int w = dims[0];
    int h = dims[1];
    unsigned char data[w*h*3];

    /* height data */ 
    FCLEAN();
    for( int i=0; i<numtp; i++ ) {

        short ground;
        FREAD( 1, short, f, &ground );

        short water;
        FREAD( 1, short, f, &water );

        char flags;
        FREAD( 1, char, f, &flags );
        char* flagsStr = tobin( flags );

        char texture;
        FREAD( 1, char, f, &texture );

        char layer;
        FREAD( 1, char, f, &layer );
        char layerHeight = (layer & 0x0f);
        if( layerHeight > maxLayer ) maxLayer = layerHeight;

        float absHeight = (ground-0x2000+((layer & 0x0f)-1)*0x0200)/4.0f;
        if( absHeight > maxHeight ) maxHeight = absHeight;
        
        /* print height data */
        printf( "Layer/Vertex Height: %d/%.0f\n", layerHeight, absHeight );

        free( flagsStr );

        /* write image data */
        //unsigned char normH = (unsigned char)((float)layerHeight/9.0f * 255);
        unsigned char normH = (unsigned char)(absHeight/256.0f * 255);
        data[3*i] = (i % 2) ? 100 : 0;
        data[3*i+1] = (flags & 0x0010) ? 255 : 0;
        data[3*i+2] = normH;
    }
    printf( "Max Height: %f\n", maxHeight );
    printf( "Max Layer: %d\n", maxLayer );
    IMG_WRITE( w, h, data );

    fclose( f );
}

