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

    int *size = w3eData.size;
    int numtp = size[0] * size[1];

    /* image buffer */
    int w = size[0];
    int h = size[1];
    unsigned char heightMap[w*h];

    /* write height map image*/ 
    for( int i=0; i<numtp; i++ ) {

        heightMap[i] = ( (float)( w3eData.tps[i].layerH ) / ( w3eData.maxLayer ) * 255 );
    }
    IMG_WRITE( "heightmap.png", w, h, 1, heightMap );
    
    /* write ramp map */
    unsigned char rampMap[w*h];
    for( int i=0; i<numtp; i++ ) {

        rampMap[i] = ( w3eData.tps[i].flags & 0x0010 ) ? 255 : 0;
    }
    IMG_WRITE( "rampMap.png", w, h, 1, rampMap );

    /* write slope map image */
    unsigned char slopeMap[w*h];
    for( int i=0; i<numtp; i++ ) {

        /* von Neumann neighbors */
        int tn_i = (i-w);
        int bn_i = (i+w);
        int ln_i = (i-1);
        int rn_i = (i+1);
        unsigned char tn = (i-w >= 0)    ? heightMap[tn_i] : 0; // top
        unsigned char bn = (i+w < numtp) ? heightMap[bn_i] : 0; // bot
        unsigned char ln = (i % w > 0)   ? heightMap[ln_i] : 0; // left
        unsigned char rn = (i % w < w-1) ? heightMap[rn_i] : 0; // right

        /* pixel */
        unsigned char px = heightMap[i];

        unsigned char dtn = ABS(px-tn);
        unsigned char dbn = ABS(px-bn);
        unsigned char dln = ABS(px-ln);
        unsigned char drn = ABS(px-rn);

        unsigned char max;
        max = MAX(dtn,dbn);
        max = MAX(max,dln);
        max = MAX(max,drn);

        slopeMap[i] = max;
    }
    IMG_WRITE( "slope.png", w, h, 1, slopeMap );
    
    /* write access map image */
    unsigned char ta = 27;
    unsigned char accessMap[w*h];
    
    for( int i=0; i<numtp; i++ ) {

        accessMap[i] = ( slopeMap[i] <= ta ) ? 255 : 0;
    }
    IMG_WRITE( "access.png", w, h, 1, accessMap);

    /* final valid paths for pathfinding */
    unsigned char pathfMap[w*h];
    for( int i=0; i<numtp; i++ ) {

        pathfMap[i] = ( accessMap[i] || rampMap[i] ) ? 255 : 0;
    }
    IMG_WRITE( "pathf.png", w, h, 1, pathfMap );

    /* blend ramps with height map */
    unsigned char heightMap2[w*h];
    for( int i=0; i<numtp; i++ ) {

        /* copy non ramped height data */
        if( !rampMap[i] ) {
            heightMap2[i] = heightMap[i];
            continue;
        }

        /* 
         * blur ramped height data */

        /* 3x3 kernel */
        int tl = (i-w-1); int t = (i-w); int tr = (i-w+1);
        int l = (i-1);    int px = (i);  int r = (i+1);
        int bl = (i+w-1); int b = (i+w); int br = (i+w+1);
        
        heightMap2[i] = (float)(
                heightMap[tl] + heightMap[t] + heightMap[tr] +
                heightMap[l]  + heightMap[px]+ heightMap[r]  +
                heightMap[bl] + heightMap[b] + heightMap[br] ) * (1.0f/9.0f);
    }
    IMG_WRITE( "heightmap2.png", w, h, 1, heightMap2 );

    /* sharpen height map */
    unsigned char heightMapS[w*h];
    for( int i=0; i<numtp; i++ ) {

        /* copy non ramped height data */
        //if( rampMap[i] ) {
        //    heightMapS[i] = heightMap[i];
        //    continue;
        //}

        /* 
         * sharpen height data */

        /* 3x3 kernel */
        int tl = (i-w-1); int t = (i-w); int tr = (i-w+1);
        int l = (i-1);    int px = (i);  int r = (i+1);
        int bl = (i+w-1); int b = (i+w); int br = (i+w+1);

        unsigned char tln = ((i-w) >= 0 && (i%w) > 0)      ? heightMap[tl] : 0;
        unsigned char tn  = ((i-w) >= 0)                   ? heightMap[t]  : 0; 
        unsigned char trn = ((i-w) >= 0 && (i%w) < w-1)    ? heightMap[tr] : 0; 
        unsigned char ln  = ((i%w) > 0)                    ? heightMap[l]  : 0;
        unsigned char pxn = 1                              ? heightMap[px] : 0;
        unsigned char rn  = ((i%w) < w-1)                  ? heightMap[r]  : 0;
        unsigned char bln = ((i+w) < numtp && (i%w) > 0)   ? heightMap[bl] : 0;
        unsigned char bn  = ((i+w) < numtp)                ? heightMap[b]  : 0;
        unsigned char brn = ((i+w) < numtp && (i%w) < w-1) ? heightMap[br] : 0;
        
        heightMapS[i] = (
                -0.0f*tln + -1.0f*tn  + -0.0f*trn +
                -1.0f*ln  +  5.0f*pxn + -1.0f*rn  +
                -0.0f*bln + -1.0f*bn  + -0.0f*brn ) * (1.0f/16.0f);
        heightMapS[i] += heightMap[i];
    }
    IMG_WRITE( "sharpHeightmap.png", w, h, 1, heightMapS );

    /* texture map */
    unsigned char textureMap[w*h];
    for( int i=0; i<numtp; i++ ) {
        char tex = ( (w3eData.tps[i].flags) & 0x0f );
        textureMap[i] = (float)tex / 8.0f * 255.0f;
    }
    IMG_WRITE( "textures.png", w, h, 1, textureMap );

    free( w3eData.tps );
    printf( "Done.\n" );
}

