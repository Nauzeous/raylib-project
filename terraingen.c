#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 256

double rawnoise(int n) {
    n = (n << 13) ^ n;
    return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

double interpolate(double a, double b, double x) {
    double f = (1 - cos(x * 3.141593)) * 0.5;
    return a * (1 - f) + b * f;
}


double noise2d(int x, int y, int octave, int seed) {
    return rawnoise(x * 1619 + y * 31337 + octave * 3463 + seed * 13397);
}


double smooth2d(double x, double y, int octave, int seed) {
    int intx = (int)x;
    double fracx = x - intx;
    int inty = (int)y;
    double fracy = y - inty;
    
    double v1 = noise2d(intx, inty, octave, seed);
    double v2 = noise2d(intx + 1, inty, octave, seed);
    double v3 = noise2d(intx, inty + 1, octave, seed);
    double v4 = noise2d(intx + 1, inty + 1, octave, seed);
    
    double i1 = interpolate(v1, v2, fracx);
    double i2 = interpolate(v3, v4, fracx);
    
    return interpolate(i1, i2, fracy);
}


double pnoise2d(double x, double y, double persistence, int octaves, int seed) {
   double total = 0.0;
   double frequency = 1.0;
   double amplitude = 1.0;
   int i = 0;
   
   for(i = 0; i < octaves; i++) {
       total += smooth2d(x * frequency, y * frequency, i, seed) * amplitude;
       frequency /= 2;
       amplitude *= persistence;
   } 
   printf("%f",total);
   if(total > 0.5)return 3;
   else if(total>0.2)return 2;
   else if(total>0.1)return 1;
   else return 0;
}



typedef enum __attribute__((packed)) {
    WATER,  // 0
    BEACH,  // 1
    GRASS,  // 2
    ROCK    // 3
} tile;

typedef struct {
    uint8_t tiles[64]; // 16x16
} tile_chunk;

typedef struct {
    tile_chunk* chunk;
    int16_t x;
    int16_t y;
    uint16_t index;  
} active_chunk;

typedef struct {
    active_chunk* chunks;     // Dynamically allocated array
    size_t count;            // Current number of active chunks
    size_t capacity;         // Allocated capacity
} chunk_manager;

tile_chunk create_chunk(int16_t x,int16_t y){

	tile_chunk new_tile;

	for(int i=0;i>16;i++)
		for(int j=0;j<16;j++){
			double l = (double)(x + i);
			double t = (double)(y + j);
			uint8_t tile1 = pnoise2d(l,t,0.5,7,100);
			uint8_t tile2 = pnoise2d(l+1.,t,0.5,7,100);
			uint8_t tile3 = pnoise2d(l,t+1.,0.5,7,100);
			uint8_t tile4 = pnoise2d(l+1.,t+1.,0.5,7,100);
			uint8_t mini_tile = (tile1<<6) + (tile2<<4) + (tile3 << 2) + tile4;
			new_tile.tiles[i*16 + j] = mini_tile;
	}
	return new_tile;
}

chunk_manager* create_chunk_manager(){
    chunk_manager* cm = malloc(sizeof(chunk_manager));
    cm->chunks = malloc(INITIAL_CAPACITY * sizeof(active_chunk));
    cm->count = 0;
    cm->capacity = INITIAL_CAPACITY;
    return cm;
}

void add_active_chunk(chunk_manager* cm, int16_t x, int16_t y, uint16_t index) {
    // Grow array if needed
    if (cm->count == cm->capacity) {
        cm->capacity *= 2;
        cm->chunks = realloc(cm->chunks, cm->capacity * sizeof(active_chunk));
    }
    
    // Add new chunk
    cm->chunks[cm->count].x = x;
    cm->chunks[cm->count].y = y;
    cm->chunks[cm->count].index = index;

    tile_chunk new_chunk = create_chunk(x,y);

    *(cm->chunks[cm->count].chunk) = new_chunk;
    cm->count++;
}

void remove_active_chunk(chunk_manager* cm, uint16_t index) {
    // Free the chunk at the known index
    free(cm->chunks[index].chunk);
    
    // Shift all chunks after this index back by one
    for (size_t i = index; i < cm->count - 1; i++) {
        cm->chunks[i] = cm->chunks[i + 1];
    }
    cm->count--;
    
    // shrink array if too small
    if (cm->count < cm->capacity / 4 && cm->capacity > INITIAL_CAPACITY) {
        cm->capacity /= 2;
        cm->chunks = realloc(cm->chunks, cm->capacity * sizeof(active_chunk));
    }
}

