#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define FREAD( n, type, file, buf )         \
    if( fread( buf, sizeof( type ), n, file ) != n ) printf( "[%d] Binary read error.\n", __LINE__ ) \

#define OUTPUT_TXT "image.info"
#define FCLEAN() fclose( fopen( OUTPUT_TXT, "w" ) )
#define FPRINT( frmt, ... ) \
    do {                    \
        FILE* f = fopen( OUTPUT_TXT, "a+" ); \
        fprintf( f, frmt, __VA_ARGS__ );         \
        if( f ) fclose( f );                  \
    } while( 0 )                              \

#define IMG_WRITE( file, w, h, data ) \
    do {                              \
        int status = stbi_write_png( file, w, h, 3, data, w*sizeof(unsigned char)*3 ); \
        if( status == 0 ) printf( "Error writing image (%s).\n", file );            \
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

