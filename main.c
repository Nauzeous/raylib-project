#include "game.h"


Player* init_player(){
	Player* player = malloc(sizeof(Player));
	player->position = (Vector2){0,0};
	player->health = 100;
	Image player_sprite = LoadImage("assets/player.png");
	player->sprite = LoadTextureFromImage(player_sprite);
	player->damage = malloc(sizeof(int)*ATTACK_VARIANTS);
	player->projectiles_per_shot = malloc(sizeof(uint8_t)*ATTACK_VARIANTS);
	player->pierce = malloc(sizeof(int8_t)*ATTACK_VARIANTS);
	player->cooldown = malloc(sizeof(float)*ATTACK_VARIANTS);
	player->damage[0] = 100;
	player->damage[1] = 100;
	player->damage[2] = 50;
	player->pierce[0] = 5;
	player->pierce[1] = 1;
	player->cooldown[0] = 0.5;
	player->cooldown[1] = 0.5;
	player->projectiles_per_shot[0] = 1;
	player->projectiles_per_shot[1] = 5;

	UnloadImage(player_sprite);
	SetTextureFilter(player->sprite,TEXTURE_FILTER_POINT);
	return player;
}

Sprite_manager* create_sprite_manager(){
	Sprite_manager* sm = malloc(sizeof(Sprite_manager));
	sm->skeleton = LoadTextureFromImage(LoadImage("assets/skeleton.png"));
	sm->imp = LoadTextureFromImage(LoadImage("assets/imp.png"));
	sm->goblin = LoadTextureFromImage(LoadImage("assets/goblin.png"));
	sm->fireball = LoadTextureFromImage(LoadImage("assets/fireball.png"));
	sm->knife = LoadTextureFromImage(LoadImage("assets/knife.png"));
	sm->shuriken = LoadTextureFromImage(LoadImage("assets/shuriken.png"));
	SetTextureFilter(sm->fireball,TEXTURE_FILTER_POINT); 
	SetTextureFilter(sm->knife,TEXTURE_FILTER_POINT);
	SetTextureFilter(sm->shuriken,TEXTURE_FILTER_POINT); 
	SetTextureFilter(sm->skeleton,TEXTURE_FILTER_POINT); 
	SetTextureFilter(sm->imp,TEXTURE_FILTER_POINT); 
	SetTextureFilter(sm->goblin,TEXTURE_FILTER_POINT); 
	sm->state_colours = malloc(sizeof(Color)*1); // increase when more states are needed
	return sm;
}


void draw_map(Texture2D* tex,Chunk_manager* cm,Player* player){

	int screen_offset_x = SCREEN_WIDTH/2 - (player->position.x-cm->offset.x)*TILE;
	int screen_offset_y = SCREEN_HEIGHT/2 - (player->position.y-cm->offset.y)*TILE;

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

void draw_enemies(Enemy_manager* em, Player* player, Sprite_manager* sm) {
    Texture2D tex;
    Vector2 diff;
    int screen_offset_x, screen_offset_y,spritesheet_offset;
    int scale_width, scale_height, tex_width, tex_height;
    Color tint;
    Rectangle source_rec;
    for(int i = 0; i < em->count; i++) {
    	diff = vec2_sub(player->position,em->position[i]);
        screen_offset_x = (SCREEN_WIDTH - player->sprite.width*TILE/2)/2 - diff.x*TILE;
        screen_offset_y = (SCREEN_HEIGHT - player->sprite.height*TILE/2)/2 - diff.y*TILE;

        switch (em->type[i]){
        	case ENEMY_IMP:
        		tex = sm->imp;
        		break;
        	case ENEMY_GOBLIN:
        		tex = sm->goblin;
        		break;
        	case ENEMY_SKELETON:
        		tex = sm->skeleton;
        		break;
        }

        tint = em->is_hit[i]?ENEMY_HURT_TINT:WHITE;
    	tex_width = tex.width/4;
    	tex_height = tex.height;
    	spritesheet_offset = em->frame[i]*tex_width;
        scale_width = tex_width*5;
        scale_height = tex_height*5;
        if(diff.x < 0.0f){
            // flip sprite - all sprites face right by default
            spritesheet_offset += tex_width;
        	tex_width = -tex_width;
        }
        source_rec = (Rectangle){spritesheet_offset,0.0f,tex_width,tex_height};
        DrawTexturePro(
            tex,
            source_rec,
            (Rectangle){screen_offset_x, screen_offset_y, scale_width, scale_height}, 
            (Vector2){scale_width/2, scale_height/2}, // scaling origin
            0.0f, tint);
    }
}


void draw_bullets(Bullet_manager* bm, Player* player, Sprite_manager* sm){
    Texture2D tex;
    Vector2 diff;
    float rotation;
    int screen_offset_x, screen_offset_y, spritesheet_offset;
    int scale_width, scale_height, tex_width, tex_height;
    int num_frames=1;
    Rectangle source_rec;

    for(int i = 0; i < bm->count; i++) {
    	diff = vec2_sub(player->position,bm->position[i]);
        screen_offset_x = (SCREEN_WIDTH - 8*TILE)/2 - diff.x*TILE;
   		screen_offset_y = (SCREEN_HEIGHT - 8*TILE)/2 - diff.y*TILE;

        rotation = atan(bm->velocity[i].y/bm->velocity[i].x)*180.0f/PI;
        switch (bm->attack[i]){
        	case FIREBALL:
        		tex = sm->fireball;
        		num_frames = 4;
        		break;
        	case KNIFE:
        		tex = sm->knife;
        		num_frames = 1;
        		break;
        	case SHURIKEN:
        		tex = sm->shuriken;
        		num_frames = 1;
        		rotation = bm->live_time[i]*1e4;
        		break;

        }
		tex_width = tex.width/num_frames;
		tex_height = tex.height;
		spritesheet_offset = bm->frame[i]*tex_width;
        
        scale_width = 40;
        scale_height = 40;
        if(bm->velocity[i].x < 0.0f){
        	spritesheet_offset+=tex_width;
        	tex_width=-tex_width;
        }
        source_rec = (Rectangle){spritesheet_offset,0.0f,tex_width,tex_height};
        DrawTexturePro(
            tex,
            source_rec,
            (Rectangle){screen_offset_x, screen_offset_y, scale_width, scale_height}, 
            (Vector2){scale_width/2, scale_height/2}, // origin
            rotation, WHITE);
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
        float len = 1.0f/sqrt(2.0f);
        movement.x *= len;
        movement.y *= len;
    }
    
    player->position.x += movement.x * PLAYER_SPEED * dt;
    player->position.y += movement.y * PLAYER_SPEED * dt;
}



int main(){
	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"game window");
    int width = 17;
    int height = 17;
	Chunk_manager* chunk_manager = create_chunk_manager(width,height);
	Enemy_manager* enemy_manager = create_enemy_manager(MAX_ENEMIES);
	Sprite_manager* sprite_manager = create_sprite_manager();
	Bullet_manager* bullet_manager = create_bullet_manager(MAX_BULLETS);
	SetRandomSeed(15);
	bool alt_fire = false;
	
	init_chunks(chunk_manager);
	Player* player = init_player();
	
	QTree* tree = create_quad_tree((Vector2){0,0},width*16);
	player_shoot_shuriken(player,bullet_manager);


	Image img = {
		.data = chunk_manager->colour_buf,
		.width = chunk_manager->width*16,
		.height = chunk_manager->height*16,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
		.mipmaps = 1
	};
	SetWindowPosition(900,100);

    Texture2D* map_texture = malloc(sizeof(Texture2D));
	*map_texture = LoadTextureFromImage(img);
    // something to do with removing blurriness when scaling
    SetTextureFilter(*map_texture,TEXTURE_FILTER_POINT); 
	SetTargetFPS(60);
	double dt;
	uint64_t frame=0;
	refresh_map(chunk_manager,map_texture);
	Enemy_type types[3] = {ENEMY_IMP,ENEMY_GOBLIN,ENEMY_SKELETON};
	for(int i = 0;i<1000;i++){
		add_enemy(enemy_manager,(Vector2){GetRandomValue(-200,200),GetRandomValue(-200,200)},types[GetRandomValue(0,3)]);
		}

	while(!WindowShouldClose()){
		dt = GetFrameTime();
		ClearBackground(WHITE);
		update_player(player,dt);
		update_enemies(tree,enemy_manager,player,chunk_manager,bullet_manager,dt);
		BeginDrawing();
		draw_map(map_texture,chunk_manager,player);
		draw_enemies(enemy_manager,player,sprite_manager);
		draw_bullets(bullet_manager,player,sprite_manager);
		draw_player(player);
		EndDrawing();
		frame++;

		advance_frames(enemy_manager,bullet_manager,frame);

		if (frame % (FRAMERATE/2) == 0){
			for (int i = 0;i<20;i++){
				add_enemy(enemy_manager,(Vector2){GetRandomValue(-200,200),GetRandomValue(-200,200)},types[GetRandomValue(0,3)]);
			}
			printf("%d enemies \n",enemy_manager->count);
			player_shoot_fireball(player,tree,chunk_manager,enemy_manager,bullet_manager);
			player_shoot_knives(player,bullet_manager,alt_fire);
			alt_fire = !alt_fire;
			update_chunks(player,chunk_manager,map_texture);
			
		}
	}

	printf("you killed %lu enemies! \n",enemy_manager->kills);

	free_chunk_manager(chunk_manager);
	free_qtree(tree);
	UnloadTexture(player->sprite);
	UnloadTexture(*map_texture);
	free_enemy_manager(enemy_manager);

	CloseWindow();

	return 0;
}
