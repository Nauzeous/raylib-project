#include "terraingen.c"
#include "raylib.h"
#include "raymath.h"

#define G 400

typedef struct{
	Vector2 position;
	int health;

}Player;

void draw_map(chunk_manager* cm,Player* player){
	Color colours[4]=[{0,0,255},{255,210,0},{0,200,0},{128,128,128}];
	for(int i=0;i<cm->count;i++){
		tile_chunk chunk = *(cm->chunks[i]);
		for(int i=0;i<16;i++)
		for(int j=0;j<16;j++){
			uint8_t mini_tile = chunk[i*16+j];
			uint8_t tile1 = mini_tile & 192;
			uint8_t tile2 = mini_tile & 48;
			uint8_t tile3 = mini_tile & 12;
			uint8_t tile4 = mini_tile & 3;
			
		}
		
	}
}

int main(){
	chunk_manager* chunk_manager = create_chunk_manager();
	add_active_chunk(chunk_manager,1,2,0);
	Player player;
	player.position =(Vector2){200,200};
	player.health = 100;

	BeginDrawing();
		ClearBackground(LIGHTGRAY);
		BeginMode2D(camera);
		draw_map(chunk_manager,&player)

	return 0;
}
