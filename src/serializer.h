#ifndef SERIALIZER_H
#define SERIALIZER_H

#define OUTPUT_FILE "image.txt"

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define FREAD( n, type, file, buf )         \
    if( fread( buf, sizeof( type ), n, file ) != n ) printf( "[%d] Binary read error.\n", __LINE__ ) \

#define FCLEAN() fclose( fopen( OUTPUT_FILE, "w" ) )
#define FPRINT( frmt, ... ) \
    do {                    \
        FILE* f = fopen( OUTPUT_FILE, "a+" ); \
        fprintf( f, frmt, __VA_ARGS__ );         \
        if( f ) fclose( f );                  \
    } while( 0 )                              \

#define OUTPUT_IMAGE "image.png"
#define IMG_WRITE( w, h, data ) \
    do {                              \
        int status = stbi_write_png( OUTPUT_IMAGE, w, h, 3, data, w*sizeof(unsigned char)*3 ); \
        if( status == 0 ) printf( "Error writing image.\n" );            \
    } while( 0 )                                                         \

/* int to binary string */
char* tobin(char n) {
    
    char* str = malloc( sizeof( char ) * 8 );
    for( int i=8-1; i>=0; i-- ) {
        *( str+i ) = ( ( ( 1 & n ) == 1 ) ? '1' : '0' );
        n >>= 1;
    }
    return str;
}

#endif

