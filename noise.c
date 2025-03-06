#include "game.h"

#define PERSISTENCE 0.5
#define OCTAVES 8


float fract(float n){
    return n - floor(n);
}

float hash1(float px, float py ){
    px = 50.*fract( px*0.3183099 );
    py = 50.*fract( py*0.3183099 );
    
    return fract(px*py*(px+py));
}

float noise(float x, float y ){
    x/=100.0f;
    y/=100.0f;
    float px = floor(x);
    float py = floor(y);
    float wx = fract(x);
    float wy = fract(y);
    float ux = wx*wx*wx*(wx*(wx*6.0-15.0)+10.0);
    float uy = wy*wy*wy*(wy*(wy*6.0-15.0)+10.0);
    float a = hash1(px,py);
    float b = hash1(px+1.0,py);
    float c = hash1(px,py+1.0);
    float d = hash1(px+1.0,py+1.0);
    return (a + (b-a)*ux + (c-a)*uy + (a - b - c + d)*ux*uy)-.5;
}


uint8_t pnoise2d(double x, double y) {
   float tiles[4] = {0.,0.,0.,0.};
   double frequency = 1.0;
   double amplitude = 1.0;
   int i = 0;
   uint8_t pack=0; 
    for(i = 0; i < OCTAVES; i++) {
       double yfreq = y*frequency;
       tiles[0] += noise( x    * frequency, yfreq) * amplitude;
       tiles[1] += noise((x+1) * frequency, yfreq) * amplitude;
       tiles[2] += noise((x+2) * frequency, yfreq) * amplitude;
       tiles[3] += noise((x+3) * frequency, yfreq) * amplitude;
       frequency *= 2;
       amplitude *= PERSISTENCE;
    }
    for(i=0;i<4;i++)
        pack|=(tiles[i]>0.4
            ?3
            :(tiles[i]>-0.2
                ?2
                :(tiles[i]>-0.25)
                    ?1
                    :0))<<(6-2*i);
    return pack;
}