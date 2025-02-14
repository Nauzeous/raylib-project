#include "terraingen.c"
#include "raylib.h"
#include "raymath.h"

#define G 400

#define TILE 5
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800


void draw_map(Chunk_manager* cm, Player* player) {
    BeginDrawing();
    int16_t x_offset = 0;
    int16_t y_offset = 0;
    Color colours[4] = {BLUE, YELLOW, GREEN, GRAY};
    
    for(int a = 0; a < cm->height; a++) {
        for(int b = 0; b < cm->width; b++) {
            Tile_chunk chunk = cm->chunks[a][b];  // Use pointer to avoid copy
            int tile_index=0,i=0,j=0;
            for(; i < 16; i++) {
                for(j=0; j < 4; j++) {
                    // Correct index calculation:
                 
                    
                    int16_t tile_x = (b*16 + j*4 - x_offset)*TILE;
                    int16_t tile_y = (a*16 + i - y_offset)*TILE;
                    
                    if(tile_x > SCREEN_WIDTH || tile_x < -TILE*4) continue;
                    if(tile_y > SCREEN_HEIGHT || tile_y < -TILE) continue;
                    
                    uint8_t mini_tile = chunk.tiles[tile_index++];
                    
                    // Draw the 4 tiles encoded in this byte
                    DrawRectangle(tile_x,        tile_y, TILE, TILE, colours[(mini_tile >> 6) & 0x03]);
                    DrawRectangle(tile_x + TILE, tile_y, TILE, TILE, colours[(mini_tile >> 4) & 0x03]);
                    DrawRectangle(tile_x + TILE*2, tile_y, TILE, TILE, colours[(mini_tile >> 2) & 0x03]);
                    DrawRectangle(tile_x + TILE*3, tile_y, TILE, TILE, colours[mini_tile & 0x03]);
                }
            }
        }
    }
    EndDrawing();
}

int main(){
	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"game window");	
	Chunk_manager* chunk_manager = create_chunk_manager(13,13);
	init_chunks(chunk_manager);
	Player player;
	player.x = 0;
	player.y = 0;
	player.health = 100;

	SetTargetFPS(60);
	Camera2D camera = {0};
	camera.target = (Vector2){player.x,player.y};
	camera.offset = (Vector2){SCREEN_WIDTH / 2.0f,SCREEN_HEIGHT/2.0f};
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	//BeginDrawing();
	while(!WindowShouldClose()){
		ClearBackground(WHITE);
		draw_map(chunk_manager,&player);
	}
	free_chunk_manager(chunk_manager);
	return 0;
}
