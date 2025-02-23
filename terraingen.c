#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "raylib.h"
#include "noise.c"
#include "string.h"

typedef struct{
    // coordinates are absolute
    double x;
    double y; 
    int health;
} Player;

typedef struct {
    uint8_t tiles[64]; // 16x16
} Tile_chunk;

typedef struct {
    Tile_chunk** chunks;     // 13x13 -6 to 6
    Color* colour_buf;
    int32_t offset_x;       // absolute coordinates of "centre"
    int32_t offset_y;
    uint8_t width;          // Width of the chunk grid
    uint8_t height;         // Height of the chunk grid
} Chunk_manager;


Tile_chunk* create_chunk(int16_t x,int16_t y){
    Tile_chunk* new_tile= malloc(sizeof(Tile_chunk));
    int a=0,local_x,local_y;
    for(local_y=0;local_y<16;local_y++)
        for(local_x=0;local_x<16;local_x+=4){
            double l = (double)(x + local_x);
            double t = (double)(y + local_y);
            new_tile->tiles[a++] = pnoise2d(l,t);
    }
    return new_tile;
}

void refresh_map(Chunk_manager* cm){
    Color colours[4] = {BLUE,YELLOW,GREEN,GRAY};
    const int tiles_per_chunk = 16*16;
    int tile_index;
    for(int chunk_y = 0;chunk_y < cm->height;chunk_y++){
        for(int chunk_x=0;chunk_x < cm->width;chunk_x++){
            Tile_chunk chunk = cm->chunks[chunk_y][chunk_x];
            int chunk_base = chunk_y*cm->width*tiles_per_chunk + chunk_x*16;
            tile_index=0;
            for(int y=0;y<16;y++){
                int buf_index = chunk_base + y*16*cm->width;
                for(int x=0;x<4;x++){
                    uint8_t mini_tile = chunk.tiles[tile_index++];
                    cm->colour_buf[buf_index++]=colours[(mini_tile>>6)&0x03];
                    cm->colour_buf[buf_index++]=colours[(mini_tile>>4)&0x03];
                    cm->colour_buf[buf_index++]=colours[(mini_tile>>2)&0x03];
                    cm->colour_buf[buf_index++]=colours[mini_tile&0x03];
                }
            }
        }
    }
}


Chunk_manager* create_chunk_manager(int16_t width, int16_t height) {
    Chunk_manager* cm = malloc(sizeof(Chunk_manager));
    cm->width = width;
    cm->height = height;
    cm->offset_x = 0;
    cm->offset_y = 0;
    
    // Allocate array of pointers
    cm->colour_buf = malloc(width*height*32*32 * sizeof(Color));
    cm->chunks = (Tile_chunk**)malloc(height * sizeof(Tile_chunk*));
    for(int i = 0; i < height; i++) {
        // Allocate each row
        cm->chunks[i] = (Tile_chunk*)malloc(width * sizeof(Tile_chunk));
    }
    return cm;
}

void add_chunk(Chunk_manager* cm, int index_x, int index_y) {
    // Check if position is within bounds
    int abs_x = cm->offset_x + 16*(index_x - (cm->width-1)/2);
    int abs_y = cm->offset_y + 16*(index_y - (cm->height-1)/2);
    cm->chunks[index_y][index_x] = *(create_chunk(abs_x,abs_y));

}
 
void update_chunks(Player* player, Chunk_manager* cm){

    int32_t player_chunk_x = ((int32_t)player->x) & 0xfffffff0;
    int32_t player_chunk_y = ((int32_t)player->y) & 0xfffffff0;


    // get new centre chunk coords
    // for now assume there is an odd number of chunks so this is easy
    int32_t local_x = (int32_t)(player->x) - player_chunk_x;
    int32_t local_y = (int32_t)(player->y) - player_chunk_y;
    if (cm->offset_x == player_chunk_x && cm->offset_y == player_chunk_y)return;
    
    int offset_x = (player_chunk_x - cm->offset_x) / 16;
    int offset_y = (player_chunk_y - cm->offset_y) / 16;

    cm->offset_x = player_chunk_x;
    cm->offset_y = player_chunk_y;

    Tile_chunk** temp_chunks = malloc(cm->height * sizeof(Tile_chunk*));
    for(int i = 0; i < cm->height; i++) {
        temp_chunks[i] = malloc(cm->width * sizeof(Tile_chunk));
        memcpy(temp_chunks[i], cm->chunks[i], cm->width * sizeof(Tile_chunk));
    }

    for(int i=0;i<cm->height;i++){
        for(int j = 0;j<cm->width;j++){
            int index_y = i + offset_y;
            int index_x = j + offset_x;
            if (index_y < 0 || index_y >= cm->height){add_chunk(cm,j,i);continue;}
            if (index_x < 0 || index_y >= cm->width){add_chunk(cm,j,i);continue;}
            cm->chunks[i][j] = temp_chunks[index_y][index_x]; 
        }
    }

    for(int i = 0; i < cm->height; i++) {
        free(temp_chunks[i]);
    }
    free(temp_chunks);
    refresh_map(cm);
}

void init_chunks(Chunk_manager* cm){
    for(int i=0;i<cm->height;i++){
        for(int j=0;j<cm->width;j++){
            add_chunk(cm,j,i);
        }
    }
    refresh_map(cm);
}

void free_chunk_manager(Chunk_manager* cm) {
    for(int i = 0; i < cm->height; i++) {
        free(cm->chunks[i]);
    }
    free(cm->chunks);
    cm->width = 0;
    cm->height = 0;
}