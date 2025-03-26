#include "game.h"

void update_bullets(Bullet_manager* bm,Enemy_manager* em, Chunk_manager* cm,QTree* tree,Player* player,float dt,double time){
	update_bullet_positions(bm,player,dt);
	for (int i = 0;i<bm->count;i++){
		int hit_enemies = 0;
		Vector2 bullet_tree_pos = vec2_sub(bm->position[i],cm->offset);
		qtree_query(tree,bullet_tree_pos,3.0f,em,&hit_enemies);
		if (bm->belongs_to_player[i]){
			for (int j = 0;j<hit_enemies;j++){
				if (bm->pierce[i] <= 0 && bm->attack[i] != SHURIKEN)break;
				int ei = em->query_arr[j];
				if (em->is_hit[ei]) // enemy is being hit
					continue;
				em->is_hit[ei] = true;
				em->health[ei] -= 50; // change this to account for damage values later 
				em->time_hit[ei] = time;
				bm->pierce[i]--;
			}
		} else if (is_square_overlap(bm->position[i],player->position,5.0f,5.0f)){ // bullet hit player
			player->health -= 5;
		}
	}

	for (int j = 0;j<bm->count;j++){
		if ((bm->live_time[j] > BULLET_TTL || bm->pierce[j] <= 0) && bm->attack[j]!=SHURIKEN){
			remove_bullet(bm,j);
			j--;
		}
	}
}

void update_bullet_positions(Bullet_manager* bm,Player* player,float dt){
	for(int i = 0;i<bm->count;i++){
		bm->live_time[i] += dt;
		switch (bm->attack[i]){
			case FIREBALL:
			case KNIFE:
				bm->position[i] = vec2_add(bm->position[i],vec2_mult(bm->velocity[i],dt));
				break;
			case SHURIKEN: // velocity stores angular frequency and offset
				float angular_freq = bm->velocity[i].x;
				float angle_offset = bm->velocity[i].y;
				float angle = angle_offset + angular_freq*bm->live_time[i];
				Vector2 circle_pos = vec2_mult((Vector2){cosf(angle),sinf(angle)},SHURIKEN_RADIUS);
				bm->position[i] = vec2_add(vec2_add(player->position,circle_pos),(Vector2){4,4});
				break;
		}
	}
}

void remove_bullet(Bullet_manager* bm, int id){
	int end = --bm->count;
	if (id == end)return;
	bm->position[id] = bm->position[end];
	bm->velocity[id] = bm->velocity[end];
	bm->attack[id] = bm->attack[end];
	bm->live_time[id] = bm->live_time[end];
	bm->pierce[id] = bm->pierce[end];
	bm->belongs_to_player[id] = bm->belongs_to_player[end];
	bm->frame[id] = bm->frame[end];
}

Bullet_manager* create_bullet_manager(int size){
	Bullet_manager* bm = malloc(sizeof(Bullet_manager));
	bm->position = malloc(sizeof(Vector2)*size);
	bm->velocity = malloc(sizeof(Vector2)*size);
	bm->attack = malloc(sizeof(Attack_type)*size);
	bm->live_time = malloc(sizeof(float)*size);
	bm->pierce = malloc(sizeof(int8_t)*size);
	bm->count = 0;
	bm->belongs_to_player = malloc(sizeof(bool)*size);
	bm->frame = malloc(sizeof(uint8_t)*size);
	return bm;
}

void remove_enemy(Enemy_manager* em, int id){
	int end = --em->count;
	if (id == end)return;
	em->health[id] = em->health[end];
	em->position[id] = em->position[end];
	em->velocity[id] = em->velocity[end];
	em->time_hit[id] = em->time_hit[end];
	em->is_hit[id] = em->is_hit[end];
	em->frame[id] = em->frame[end];
	em->type[id] = em->type[end];
}

void add_bullet(Bullet_manager* bm,Attack_type type, int8_t pierce,
				Vector2 position,Vector2 velocity,bool belongs_to_player){
	int i = bm->count;
	bm->position[i] = position;
	bm->attack[i] = type;
	bm->velocity[i] = velocity;
	bm->belongs_to_player[i] = belongs_to_player;
	bm->live_time[i] = 0;
	bm->pierce[i] = pierce;
	bm->frame[i]=0;
	bm->count++;
}

void shoot_at_player(Bullet_manager* bm,Player* player, Enemy_manager* em, int id){
	//Enemy_type type = em->type[id];
	Vector2 position = em->position[id];
	Vector2 velocity = vec2_sub(player->position,position);
	scale_vec(&velocity,10.0f);
	add_bullet(bm,FIREBALL,1,position,velocity,false);
	return;
}

void enemy_separation(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm){
	int num_points;
	int *x = &num_points;
	for(int i = 0;i<em->count;i++){
		Vector2 curr_point=em->position[i];
		Vector2 vel = vec2_sub(player->position,curr_point);

		int separation_threshold;
		if (em->count < 400){
			separation_threshold = 10;
		}
        else if (em->count < 1000) {
            separation_threshold = 50;
        } 
        else if (em->count < 2000) {
            separation_threshold = 70;
        } 
        else if (em->count < 4000) {
            separation_threshold = 90;
        } 
        else {
            separation_threshold = 99;
        }
        if (GetRandomValue(0,100)<separation_threshold){
        	em->velocity[i] = vel;
        	continue;
        }
        scale_vec(&vel,ENEMY_SPEED);
		num_points = 0;

		Vector2 tree_pos = vec2_sub(curr_point,cm->offset);

		qtree_query(tree,tree_pos,PERCEPTION_RADIUS,em,x);
		Vector2 separation = {0,0};

		// get separation
		for(int j=0;j<num_points;j++){
			Vector2 other_pos = em->position[em->query_arr[j]];
			Vector2 diff = vec2_sub(curr_point,other_pos);
			float dist = sqrt(diff.x*diff.x+diff.y*diff.y+1e-3);
			float separation_strength = (1.0f-dist/PERCEPTION_RADIUS)*ENEMY_SPEED;
			//dont use scale vec here since you already have the length of the vector
			diff = vec2_mult(diff,separation_strength/dist);
			separation = vec2_add(separation,diff);
			
		}

		vel = vec2_add(vel,separation);
		em->velocity[i] = vel;
	}
	return;
}



void player_shoot_fireball(Player* player,QTree* tree,Chunk_manager* cm,Enemy_manager* em,Bullet_manager* bm){
	int enemies_in_range = 0;
	int num_fireballs = 1;

	qtree_query(tree,vec2_sub(player->position,cm->offset),PLAYER_PERCEPTION,em,&enemies_in_range);
	int8_t pierce = player->pierce[FIREBALL];
	if (enemies_in_range == 0)return;
	if (enemies_in_range>10)enemies_in_range=10;
	Vector2* positions = malloc(sizeof(Vector2)*enemies_in_range);

	// fill array with positions relative to player
	for(int i = 0;i<enemies_in_range;i++){
		positions[i] = vec2_sub(em->position[em->query_arr[i]],player->position);
	}
	if (enemies_in_range<num_fireballs)num_fireballs = enemies_in_range;
	// insertion sort based function
	positions = closest_enemies(positions,enemies_in_range);

	for(int i = 0;i<num_fireballs;i++){
		Vector2 direction = positions[i];
		scale_vec(&direction,FIREBALL_SPEED);	
		add_bullet(bm,FIREBALL,pierce,player->position,direction,true);
	}
}

void player_shoot_shuriken(Player* player, Bullet_manager* bm){
	int num_shurikens = 6;
	int damage = player->damage[SHURIKEN];
	float angle = 2.0f*PI/(float)num_shurikens;
	//// velocity contains angular frequency and a second angular frequency for the change in radius
	
	Vector2 origin = vec2_add(player->position,(Vector2){4,4});
	for (int i = 0;i<num_shurikens;i++){
		Vector2 velocity = (Vector2){2,angle*i};
		Vector2 position = player->position; // dont actually need to set position since the position is directly assigned to
		add_bullet(bm,SHURIKEN,1,position,velocity,true);
	}
}

void player_shoot_knives(Player* player, Bullet_manager* bm, bool alt_fire){
	int num_knives=player->projectiles_per_shot[KNIFE];
	int damage = player->damage[KNIFE];
	int pierce = player->pierce[KNIFE];
	float angle_offset = 2.0f*PI/(float)num_knives;
	Vector2 direction;
	Vector2 origin = vec2_add(player->position,(Vector2){4,4});
	direction = (Vector2){1,0};
	if (alt_fire){
		vec2_rotate(&direction,angle_offset/2.0f);
	}
	scale_vec(&direction,KNIFE_SPEED);

	for(int i = 0;i<num_knives;i++){
		add_bullet(bm,KNIFE,pierce,origin,direction,true);
		vec2_rotate(&direction,angle_offset);
	}
}


void add_enemy(Enemy_manager* em,Vector2 pos,Enemy_type type){
	int i = em->count;
	em->health[i]=100;
	em->position[i]=pos;
	em->velocity[i] = (Vector2){0,0};
	em->type[i] = type;
	em->is_hit[i] = false;
	em->frame[i] = GetRandomValue(0,3);
	em->time_hit[i] = 0.0f;
	em->count++;
}





void update_enemy_positions(Enemy_manager* em,float dt){
	for(int i = 0;i<em->count;i++){
		scale_vec(em->velocity+i,ENEMY_SPEED);
		em->position[i] = vec2_add(em->position[i],vec2_mult(em->velocity[i],dt));
	}
}

Enemy_manager* create_enemy_manager(int size){
	Enemy_manager* em = malloc(sizeof(Enemy_manager));
	em->health = malloc(size*sizeof(int));
	em->position = malloc(size*sizeof(Vector2));
	em->velocity = malloc(size*sizeof(Vector2));
	em->type = malloc(size*sizeof(Enemy_type));
	em->query_arr = malloc(size*sizeof(int));
	em->time_hit = malloc(size*sizeof(double));
	em->is_hit = malloc(size*sizeof(bool));
	em->frame = malloc(size*sizeof(int));
	em->kills = 0;
	em->count = 0;
	return em;
}

void update_health(Enemy_manager* em,double time){
	for(int i = 0;i<em->count;i++){
		if (em->is_hit[i] && time-em->time_hit[i] > INVINCIBILITY_TIME){
			em->is_hit[i] = false;
		}
		if (em->health[i] <= 0){
			remove_enemy(em,i);
			em->kills++;
			i--;
		}
	}
}

void free_enemy_manager(Enemy_manager* em){
	free(em->health);
	free(em->position);
	free(em->velocity);
	free(em->query_arr);
	free(em->time_hit);
	free(em->is_hit);
	free(em->frame);
	free(em->type);
}


void update_enemies(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm, Bullet_manager* bm, float dt){
	double time = GetTime();
	enemy_separation(tree,em,player,cm);
	update_enemy_positions(em,dt);
	rebuild_tree(tree,em,cm);
	update_bullets(bm,em,cm,tree,player,dt,time);
	update_health(em,time);
}

void advance_frames(Enemy_manager* em,Bullet_manager* bm,uint64_t game_frame){
	int max_frames;
	if (game_frame % 15 == 0){
		for (int i = 0;i<em->count;i++){
			max_frames = 4;
			em->frame[i] = em->frame[i]++ % max_frames;
		}
	}
	if (game_frame % 5 == 0){
		for(int j = 0;j<bm->count;j++){
			switch (bm->attack[j]){
				case FIREBALL:
					max_frames = 4;
					break;
				case KNIFE:
					max_frames = 1;
					break;
			}
			bm->frame[j] = bm->frame[j]++ % max_frames;

		}
	}
}

void free_bullet_manager(Bullet_manager* bm){
	free(bm->position);
	free(bm->velocity);
	free(bm->attack);
	free(bm->live_time);
	free(bm->pierce);
	free(bm->belongs_to_player);
	free(bm->frame);
}