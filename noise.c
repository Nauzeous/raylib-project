#include "game.h"

#define PERSISTENCE 0.5
#define OCTAVES 8


float fract(float n){
    return n - floor(n);
}

float hash1(float px, float py ){
    px = 50.0f*fract( px*0.3183099f );
    py = 50.0f*fract( py*0.3183099f );
    return fract(px*py*(px+py));
}


Vector2* closest_enemies(Vector2* relative_positions, int size){
    // use an insertion sort to sort two lists by one of them as a key
    // must pass in the array of relative positions to the player
    double dist[size];
    for(int i = 0;i<size;i++){
        Vector2 rp = relative_positions[i];
        dist[i] = rp.x*rp.x+rp.y*rp.y;
    }
    for(int i=0;i<size;i++){
        int j = i;
        double current = dist[i];
        while(j>0 && dist[j-1]>current){
            dist[j]=dist[j-1];
            relative_positions[j]=relative_positions[j-1];
            j--;
        }
        dist[j]=current;
        relative_positions[j]=relative_positions[i];
    }
    return relative_positions;
}


void scale_vec(Vector2* in,float scale){
    if (!(in->x || in->y))return;
    float len = scale/sqrt(in->x*in->x+in->y*in->y);
    in->x *= len;
    in->y *= len;
}

void vec2_rotate(Vector2* a,float angle){ // angle in radians
    float c = cos(angle),s=sin(angle);
    Vector2 temp = *a;
    a->x = temp.x * c - s * temp.y;
    a->y = temp.x * s + c * temp.y;
}

Vector2 vec2_sub(Vector2 a, Vector2 b){ // a - b
    return (Vector2){a.x-b.x,a.y-b.y};
}

Vector2 vec2_add(Vector2 a, Vector2 b){ 
    return (Vector2){a.x+b.x,a.y+b.y};
}

Vector2 vec2_mult(Vector2 a, float b){ 
    return (Vector2){a.x*b,a.y*b};
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


uint8_t pnoise2d(double x, double y){
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