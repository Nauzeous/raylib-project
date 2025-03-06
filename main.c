#include "game.h"


Player init_player(){
	Player player;
	player.x = 0;
	player.y = 0;
	player.health = 100;
	Image player_sprite = LoadImage("assets/player.png");
	player.sprite = LoadTextureFromImage(player_sprite);
	UnloadImage(player_sprite);
	SetTextureFilter(player.sprite,TEXTURE_FILTER_POINT);
	return player;
}


void draw_map(Texture2D* tex,Chunk_manager* cm,Player* player){

	int screen_offset_x = SCREEN_WIDTH/2 - (player->x-cm->offset.x)*TILE;
	int screen_offset_y = SCREEN_HEIGHT/2 - (player->y-cm->offset.y)*TILE;

	DrawTexturePro(
		*tex,
		// source rect
		(Rectangle){0.0f,0.0f,tex->width,tex->height}, 
		// target rect
		(Rectangle){screen_offset_x,screen_offset_y,tex->width*TILE,tex->height*TILE}, 
		// scaling origin (on the target rect)
		(Vector2){(float)(tex->width * TILE) / 2.0f, 
                  (float)(tex->height * TILE) / 2.0f},
        // something then the fill colour before drawing
		0.0f,WHITE);
}

void draw_enemies(Enemy_manager* em, Player* player){
	for(int i = 0;i<em->count;i++){
		float dx = player->x - em->position[i].x;
		float dy = player->y - em->position[i].y;
		int screen_offset_x = (SCREEN_WIDTH - player->sprite.width*TILE/2)/2 - dx*TILE;
		int screen_offset_y = (SCREEN_HEIGHT - player->sprite.height*TILE/2)/2 - dy*TILE;
		DrawTexturePro(
		player->sprite,
		(Rectangle){0.0f,0.0f,player->sprite.width,player->sprite.height},
		(Rectangle){screen_offset_x,screen_offset_y,
				   (player->sprite.width)*TILE/3,(player->sprite.height)*TILE/3}, 
		(Vector2){0,0}, // origin
		0.0f,WHITE);
	}
}


void draw_player(Player* player){
	int screen_offset_x = (SCREEN_WIDTH - player->sprite.width*TILE/2)/2;
	int screen_offset_y = (SCREEN_HEIGHT - player->sprite.height*TILE/2)/2;

	DrawTexturePro(
		player->sprite,
		(Rectangle){0.0f,0.0f,player->sprite.width,player->sprite.height},
		(Rectangle){screen_offset_x,screen_offset_y,
				   (player->sprite.width)*TILE/2,(player->sprite.height)*TILE/2},
		(Vector2){0,0}, // origin
		0.0f,WHITE);
}

void update_player(Player* player, float dt) {
    Vector2 movement = {0, 0};
    
    if (IsKeyDown(KEY_W)) movement.y -= 1;
    if (IsKeyDown(KEY_S)) movement.y += 1;
    if (IsKeyDown(KEY_A)) movement.x -= 1;
    if (IsKeyDown(KEY_D)) movement.x += 1;
    
    // normalize if moving diagonally
    if (movement.x != 0 && movement.y != 0) {
        float len = 1.0f/sqrtf(2.0f);
        movement.x *= len;
        movement.y *= len;
    }
    
    player->x += movement.x * PLAYER_SPEED * dt;
    player->y += movement.y * PLAYER_SPEED * dt;
}



int main(){
	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"game window");
    int width = 17;
    int height = 17;
	Chunk_manager* chunk_manager = create_chunk_manager(width,height);
	Enemy_manager* enemy_manager = create_enemy_manager(MAX_ENEMIES);
	SetRandomSeed(15);
	
	init_chunks(chunk_manager);
	Player player = init_player();


	QTree* tree = create_quad_tree((Vector2){0,0},width*10);


	Image img = {
		.data = chunk_manager->colour_buf,
		.width = chunk_manager->width*16,
		.height = chunk_manager->height*16,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
		.mipmaps = 1
	};
	SetWindowPosition(900,100);

    Texture2D map_texture = LoadTextureFromImage(img);
    // something to do with removing blurriness when scaling
    SetTextureFilter(map_texture,TEXTURE_FILTER_POINT); 
	SetTargetFPS(60);

	double timer = 0;
	float dt;

	refresh_map(chunk_manager,&map_texture);


	for(int i = 0;i<20;i++){
		for(int j = 0;j<20;j++){
			// spawn 1600 enemies
			add_enemy(enemy_manager,tree,(Vector2){i*6+5,j*6+5},chunk_manager);
			//add_enemy(enemy_manager,tree,(Vector2){-i*5-5,-j*5-5},chunk_manager);
			//add_enemy(enemy_manager,tree,(Vector2){-i*5-5,j*5+5},chunk_manager);
			add_enemy(enemy_manager,tree,(Vector2){i*6+5,-j*6-5},chunk_manager);
		}
	}
	

	while(!WindowShouldClose()){
		dt = GetFrameTime();
		ClearBackground(WHITE);
		update_player(&player,dt);
		update_enemies(tree,enemy_manager,&player,chunk_manager,dt);
		BeginDrawing();
		draw_map(&map_texture,chunk_manager,&player);
		draw_enemies(enemy_manager,&player);
		draw_player(&player);
		EndDrawing();
		
		timer += dt;


		if (timer > 1.0f){
			update_chunks(&player,chunk_manager);
			timer -= 1.0f;
			refresh_map(chunk_manager,&map_texture);
		}
	}

	free_chunk_manager(chunk_manager);
	free_qtree(tree);
	UnloadTexture(player.sprite);
	UnloadTexture(map_texture);
	free_enemy_manager(enemy_manager);

	CloseWindow();

	return 0;
}
