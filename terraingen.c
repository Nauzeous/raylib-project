#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void rotate2D(double* x, double* y, float angle){
    float c = cos(angle);
    float s = sin(angle);
    double tx = c * (*x) - s * (*y);
    double ty = s * (*x) + c * (*y);
    *x = tx;
    *y = ty;
}

float fract(float n){
    return n - (float)(int)n;
}

float hash1(float px, float py ){
    px = 50.*fract( px*0.3183099 );
    py = 50.*fract( py*0.3183099 );
    
    return fract(px*py*(px+py));
}


float noise(float x, float y ){
    x/=10.0f;
    y/=10.0f;
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


uint8_t pnoise2d(double x, double y, double persistence, int octaves, int seed) {
   float tiles[4] = {0.,0.,0.,0.};
   double frequency = 1.0;
   double amplitude = 1.0;
   int i = 0;
   uint8_t pack=0;

   
    for(i = 0; i < octaves; i++) {
       double yfreq = y*frequency;

       tiles[0] += noise( x    * frequency, yfreq) * amplitude;
       tiles[1] += noise((x+1) * frequency, yfreq) * amplitude;
       tiles[2] += noise((x+2) * frequency, yfreq) * amplitude;
       tiles[3] += noise((x+3) * frequency, yfreq) * amplitude;
       frequency *= 2;
       amplitude *= persistence;
    }
    for(i=0;i<4;i++)pack|=(tiles[i]>0.4?3:(tiles[i]>-0.2?2:(tiles[i]>-0.3)?1:0))<<(6-2*i);
    return pack;

}

typedef struct{
    int16_t x;
    int16_t y;
    int health;
} Player;

typedef struct {
    uint8_t tiles[64]; // 16x16
} Tile_chunk;


typedef struct {
    Tile_chunk** chunks;     // 13x13 -6 to 6
    int16_t width;          // Width of the chunk grid
    int16_t height;         // Height of the chunk grid
} Chunk_manager;


Tile_chunk create_chunk(int16_t x,int16_t y){

    Tile_chunk new_tile;
    int a=0;
    for(int i=0;i<16;i++)
        for(int j=0;j<4;j++){
            double l = (double)(x + j*4);
            double t = (double)(y + i);
            new_tile.tiles[a++] = pnoise2d(l,t,0.5,8,8);
    }
    return new_tile;
}



Chunk_manager* create_chunk_manager(int16_t width, int16_t height) {
    Chunk_manager* cm = malloc(sizeof(Chunk_manager));
    cm->width = width;
    cm->height = height;
    
    // Allocate array of pointers
    cm->chunks = (Tile_chunk**)malloc(height * sizeof(Tile_chunk*));
    for(int i = 0; i < height; i++) {
        // Allocate each row
        cm->chunks[i] = (Tile_chunk*)malloc(width * sizeof(Tile_chunk));
    }
    return cm;
}


void add_chunk(Chunk_manager* cm, int16_t x, int16_t y) {
    // Check if position is within bounds
    if(x < 0 || x >= cm->width || y < 0 || y >= cm->height) {
        return;
    }
    cm->chunks[y][x] = create_chunk(x*16,y*16);
}

void scroll_right(Chunk_manager* cm, Player* player) { // map moves right, leaving space on the left
    for(int x = cm->width - 1; x > 0; x--) {
        for(int y = 0; y < cm->height; y++) {
            cm->chunks[x][y] = cm->chunks[x - 1][y];
        }
    }
    int16_t top = player->y & 0xfff0 - 6*16;
    int16_t left = player->x & 0xfff0 - 6*16;
    for(int y = 0;y<cm->height;y++){
        cm->chunks[cm->width -1][y] = create_chunk(left,top + 16*y);
    }
}

void scroll_left(Chunk_manager* cm, Player* player) { // map moves right, leaving space on the left
    for(int x = 1; x<cm->width;x++) {
        for(int y = 0; y<cm->height;y++) {
            cm->chunks[x-1][y] = cm->chunks[x][y];
        }
    }
    int16_t top = player->y & 0xfff0 - 6*16;
    int16_t left = player->x & 0xfff0 + 6*16;
    for(int y = 0;y<cm->height;y++){
        cm->chunks[0][y] = create_chunk(left,top + 16*y);
    }
}

void scroll_up(Chunk_manager* cm, Player* player) { // map moves right, leaving space on the left
    for(int x = 0; x<cm->width;x++) {
        for(int y = 0; y<cm->height-1;y++) {
            cm->chunks[x][y] = cm->chunks[x][y+1];
        }
    }
    int16_t top = player->y & 0xfff0 + 6*16;
    int16_t left = player->x & 0xfff0 - 6*16;
    for(int x = 0;x<cm->width;x++){
        cm->chunks[x][cm->height -1] = create_chunk(left+16*x,top);
    }
}

void scroll_down(Chunk_manager* cm, Player* player) { // map moves right, leaving space on the left
    for(int x = 0; x<cm->width;x++) {
        for(int y = 1; y<cm->height;y++) {
            cm->chunks[x][y-1] = cm->chunks[x][y];
        }
    }

    int16_t top = player->y & 0xfff0 - 6*16;
    int16_t left = player->x & 0xfff0 - 6*16;
    for(int x = 0;x<cm->width;x++){
        cm->chunks[x][0] = create_chunk(left+16*x,top);
    }
}


void init_chunks(Chunk_manager* cm){
    for(int i=0;i<cm->height;i++){
        for(int j=0;j<cm->width;j++){
            add_chunk(cm,j,i);
        }
    }
}

void free_chunk_manager(Chunk_manager* cm) {
    for(int i = 0; i < cm->height; i++) {
        free(cm->chunks[i]);
    }
    free(cm->chunks);
    cm->width = 0;
    cm->height = 0;
}