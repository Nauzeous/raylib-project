#include "game.h"


void refresh_map(Chunk_manager* cm,Texture2D* tex){
    Color colours[4] = {BLUE,YELLOW,GREEN,GRAY};
    const int tiles_per_chunk = 16*16;
    int tile_index;
    for(int chunk_y = 0;chunk_y < cm->height;chunk_y++){
        for(int chunk_x=0;chunk_x < cm->width;chunk_x++){
            int chunk_base = chunk_y*cm->width*tiles_per_chunk + chunk_x*16;
            tile_index=0;
            for(int y=0;y<16;y++){
                int buf_index = chunk_base + y*16*cm->width;
                for(int x=0;x<4;x++){
                    uint8_t mini_tile = cm->chunks[chunk_y][chunk_x]->tiles[tile_index++];
                    cm->colour_buf[buf_index++]=colours[(mini_tile>>6)&0x03];
                    cm->colour_buf[buf_index++]=colours[(mini_tile>>4)&0x03];
                    cm->colour_buf[buf_index++]=colours[(mini_tile>>2)&0x03];
                    cm->colour_buf[buf_index++]=colours[mini_tile&0x03];
                }
            }
        }
    }
    UpdateTexture(*tex,cm->colour_buf);
}


Chunk_manager* create_chunk_manager(uint8_t width, uint8_t height) {
    Chunk_manager* cm = malloc(sizeof(Chunk_manager));
    cm->width = width;
    cm->height = height;
    cm->offset = (Vector2){0,0};
    cm->colour_buf = malloc(width*height*16*16 * sizeof(Color));
    cm->chunks = (Tile_chunk***)malloc(height * sizeof(Tile_chunk**));
    for(int i = 0; i < height; i++) {
        // Allocate each row
        cm->chunks[i] = (Tile_chunk**)malloc(width * sizeof(Tile_chunk*));
        for (int j = 0;j<width;j++){
            cm->chunks[i][j] = malloc(sizeof(Tile_chunk));
        }
    }
    return cm;
}

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

void add_chunk(Chunk_manager* cm, int index_x, int index_y) {
    // assume the indexes are in bounds of array
    int abs_x = cm->offset.x + 16*(index_x - (cm->width-1)/2);
    int abs_y = cm->offset.y + 16*(index_y - (cm->height-1)/2);
    cm->chunks[index_y][index_x] = create_chunk(abs_x,abs_y);
}
 
void update_chunks(Player* player, Chunk_manager* cm, Texture2D* map_tex){

    int32_t player_chunk_x = ((int32_t)player->position.x) & 0xfffffff0;
    int32_t player_chunk_y = ((int32_t)player->position.y) & 0xfffffff0;

    // dont need to update if the players chunk hasnt changed
    if (cm->offset.x == player_chunk_x && cm->offset.y == player_chunk_y)return;
    
    int offset_x = (player_chunk_x - cm->offset.x) / 16;
    int offset_y = (player_chunk_y - cm->offset.y) / 16;

    cm->offset.x = player_chunk_x;
    cm->offset.y = player_chunk_y;

    Tile_chunk*** temp_chunks = malloc(cm->height * sizeof(Tile_chunk*));
    for(int i = 0; i < cm->height; i++) {
        temp_chunks[i] = malloc(cm->width * sizeof(Tile_chunk*));
        memcpy(temp_chunks[i], cm->chunks[i], cm->height * sizeof(Tile_chunk*));
    }

    for(int i=0;i<cm->height;i++){
        for(int j = 0;j<cm->width;j++){
            int index_y = i + offset_y;
            int index_x = j + offset_x;
            if (index_y < 0 || index_y >= cm->height){add_chunk(cm,j,i);continue;}
            if (index_x < 0 || index_x >= cm->width){add_chunk(cm,j,i);continue;}
            cm->chunks[i][j] = temp_chunks[index_y][index_x]; 
        }
    }
    for(int i = 0; i < cm->height; i++){
        free(temp_chunks[i]);
    }
    free(temp_chunks);
    refresh_map(cm,map_tex);
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
        for (int j = 0; j < cm->width;j++){
            free(cm->chunks[i][j]);
        }
        free(cm->chunks[i]);
    }
    free(cm->chunks);
    free(cm->colour_buf);
    free(cm);
}