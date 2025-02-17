#include "terraingen.c"
#include "raylib.h"
#include "raymath.h"

#define G 400

#define TILE 6
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800


void draw_map(Texture2D* tex,Chunk_manager* cm,Player* player){
	BeginDrawing();
	UpdateTexture(*tex,cm->colour_buf);
	DrawTexturePro(
		*tex,
		(Rectangle){0.0f,0.0f,tex->width,tex->height},
		(Rectangle){0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT},
		(Vector2){0.0f,0.0f},
		0.0f,WHITE);
	EndDrawing();
}

int main(){
	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"game window");
    int width = (SCREEN_WIDTH / 16)/TILE + 1;
    int height = (SCREEN_HEIGHT / 16)/TILE + 1;
	Chunk_manager* chunk_manager = create_chunk_manager(width,height);
	init_chunks(chunk_manager);
	Player player;
	player.x = 0;
	player.y = 0;
	player.health = 100;

	Image img = {
		.data = chunk_manager->colour_buf,
		.width = chunk_manager->width*16,
		.height = chunk_manager->height*16,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
		.mipmaps = 1
	};

    Texture2D map_texture = LoadTextureFromImage(img);   
    SetTextureFilter(map_texture,TEXTURE_FILTER_POINT); 

	SetTargetFPS(60);
	Camera2D camera = {0};
	camera.target = (Vector2){player.x,player.y};
	camera.offset = (Vector2){SCREEN_WIDTH / 2.0f,SCREEN_HEIGHT/2.0f};
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	while(!WindowShouldClose()){
		draw_map(&map_texture,chunk_manager,&player);
	}
	free_chunk_manager(chunk_manager);
	return 0;
}
