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
	int screen_offset_x = (int)((player->x - (double)cm->offset_x) * TILE);
	int screen_offset_y = (int)((player->y - (double)cm->offset_y) * TILE);
	DrawTexturePro(
		*tex,
		(Rectangle){0.0f,0.0f,tex->width,tex->height}, //source rect 
		(Rectangle){0.0f,0.0f,SCREEN_WIDTH*2,SCREEN_HEIGHT*2}, // target rect
		(Vector2){screen_offset_x,screen_offset_y}, // origin
		0.0f,WHITE);
	EndDrawing();
}

void update_player(Player* player, float dt) {
    Vector2 movement = {0, 0};
    
    if (IsKeyDown(KEY_W)) movement.y -= 1;
    if (IsKeyDown(KEY_S)) movement.y += 1;
    if (IsKeyDown(KEY_A)) movement.x -= 1;
    if (IsKeyDown(KEY_D)) movement.x += 1;
    
    // Normalize diagonal movement
    if (movement.x != 0 && movement.y != 0) {
        float len = sqrtf(2.0f);
        movement.x /= len;
        movement.y /= len;
    }
    
    float speed = 20.0f; // Adjust as needed
    player->x += movement.x * speed * dt;
    player->y += movement.y * speed * dt;
}

int main(){
	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"game window");
    int width = (SCREEN_WIDTH / 16)/TILE + 9;
    int height = (SCREEN_HEIGHT / 16)/TILE + 9;
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
	double timer = 0;
	while(!WindowShouldClose()){
		ClearBackground(WHITE);

		draw_map(&map_texture,chunk_manager,&player);
		float dt = GetFrameTime();
		update_player(&player,dt);

		timer += dt;

		if (timer > 3.0f){
			update_chunks(&player,chunk_manager);
			timer -= 3.0f;
		}
	}
	free_chunk_manager(chunk_manager);
	return 0;
}
