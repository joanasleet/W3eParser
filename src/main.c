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
    float vertH[numtp];

    /* height data */ 
    FPRINT( "%s\n", "Tilepoint Data: ( VertexHeight | Layer )" );
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
        vertH[i] = absHeight;                       // B ( normalized further down )
    }
    fclose( f );
    FPRINT( "Max Height: %.1f\n", maxHeight );
    FPRINT( "Max Layer: %d\n", maxLayer );

    /* normalize image data */
    for( int i=0; i<numtp; i++ ) {

        data[3*i+2] = ( vertH[i] / maxHeight * 255 );
    }
    IMG_WRITE( w, h, data );

    printf( "Done.\n" );
}

