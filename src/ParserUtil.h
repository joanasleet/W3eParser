#ifndef PARSERUTIL_H
#define PARSERUTIL_H

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define FREAD( n, type, file, buf )     \
	do {				\
		if( fread( buf, sizeof( type ), n, file ) != n ) \
		printf( "[%d] Binary read error.\n", __LINE__ ); \
	} while( 0 )			\

#define IMG_WRITE( file, w, h, c, data ) \
    do {                              \
        int status = stbi_write_png( file, w, h, c, data, w*sizeof(unsigned char)*c ); \
        if( status == 0 ) printf( "Error writing image (%s).\n", file );            \
    } while( 0 )                                                         \

#define ABS(d) (((d)>0)?(d):(-(d)))
#define MAX(a,b) ((a)>(b)?(a):(b))

/* int to binary string */
char* tobin(char n) {
    
    char* str = malloc( sizeof( char ) * 8 );
    for( int i=8-1; i>=0; i-- ) {
        *( str+i ) = ( ( ( 1 & n ) == 1 ) ? '1' : '0' );
        n >>= 1;
    }
    return str;
}

/* tilepoint structure */
typedef struct {

    /* height info */
    float vertexH;
    char layerH;

    short water;
    short ground;

    /* bitfields */
    char texture;
    char flags;
    char layer;

} tilepoint;

/* file structure */
typedef struct {

	int size[2];
	tilepoint *tps;

    char maxLayer;
    float maxVertex;

} W3eData;

/* parse w3e file */
W3eData parse( const char* w3eFile ) {

    FILE* f = fopen( w3eFile, "rb" );

    if( !f ) {
        printf( "Cannot open %s\n", w3eFile );
        W3eData nub;
        return nub;
    }

    /* parse header */
    char fid[4];
    FREAD( 4, char, f, fid );

    int fver[1];
    FREAD( 1, int, f, fver );

    char ts[1];
    FREAD( 1, char, f, ts );

    int cts[1];
    FREAD( 1, int, f, cts );

    int a[1];
    FREAD( 1, int, f, a );

    char gtsIDs[ 4*a[0] ];
    FREAD( 4*a[0], char, f, gtsIDs );

    int b[1];
    FREAD( 1, int, f, b );

    char cltsIDs[ 4*b[0] ];
    FREAD( 4*b[0], char, f, cltsIDs );

    int size[2];
    FREAD( 2, int, f, size);

    float center[2];
    FREAD( 2, float, f, center );

    int numtp = size[0] * size[1];

    /* prep data struct */
    W3eData data;
    data.size[0] = size[0];
    data.size[1] = size[1];
    data.tps = malloc( sizeof( tilepoint ) * numtp );

    /*
     * parse tilepoints */
    char maxLayer = 0;
    float maxVertex = 0.0f;

    for( int i=0; i<numtp; i++ ) {

        tilepoint *tp = &( data.tps[i] );
        
        FREAD( 1, short, f, &(tp->ground) );
        FREAD( 1, short, f, &(tp->water) );
        FREAD( 1, char, f, &(tp->flags) );
        FREAD( 1, char, f, &(tp->texture) );
        FREAD( 1, char, f, &(tp->layer) );

        /* find max layer */
        char layerHeight = ( tp->layer & 0x0f );
        if( layerHeight > maxLayer ) maxLayer = layerHeight;
        tp->layerH = layerHeight;

        /* find max vertex */
        float vertHeight = ( tp->ground - 0x2000 + ( ( tp->layer & 0x0f )-1 )*0x0200)/4.0f;
        if( vertHeight > maxVertex ) maxVertex = vertHeight;
        tp->vertexH = vertHeight;
    }
    data.maxLayer = maxLayer;
    data.maxVertex = maxVertex;

    fclose( f );

    return data;
}

#endif

