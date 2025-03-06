#include "game.h"



void enemy_separation(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm){
	int num_points;
	int *x = &num_points;
	for(int i = 0;i<em->count;i++){
		Vector2 curr_point=em->position[i];
		Vector2 vel = (Vector2){player->x - curr_point.x,player->y-curr_point.y};

		int separation_threshold = 10;
		if (em->count < 400){
			separation_threshold = 20;
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

        float len = ENEMY_SPEED/sqrt(vel.x*vel.x+vel.y*vel.y);
		vel.x*=len;
		vel.y*=len;
		num_points = 0;

		Vector2 tree_pos = {curr_point.x - cm->offset.x,
							curr_point.y - cm->offset.y};
		float separation_mult = ENEMY_SPEED*10.0f;

		qtree_query(tree,tree_pos,PERCEPTION_RADIUS,em,x);
		Vector2 separation = {0,0};

		// get separation
		for(int j=0;j<num_points;j++){
			Vector2 diff = {curr_point.x-em->query_arr[j].x,
							curr_point.y-em->query_arr[j].y};
			float dist = 1.0f/(diff.x*diff.x+diff.y*diff.y+1e-3);
			separation.x += diff.x*dist;
			separation.y += diff.y*dist; 
		}

		// avoid division by 0 if there are no other points in range
		if (num_points > 0){
			separation_mult /= (float)(num_points*num_points);
			vel.x +=  2.0f*ENEMY_SPEED*separation.x;
			vel.y += 2.0f*ENEMY_SPEED*separation.y;
		}
		em->velocity[i] = vel;
	}
	return;
}


void add_enemy(Enemy_manager* em, QTree* tree,Vector2 pos,Chunk_manager* cm){
	int i = em->count;
	em->health[i]=100;
	Vector2 tree_pos = {pos.x-cm->offset.x,pos.y-cm->offset.y};
	em->position[i]=pos;
	em->velocity[i] = (Vector2){0,0};
	em->type[i] = ENEMY_ZOMBIE;
	em->count++;
	qtree_insert(tree,tree_pos);
}




void update_enemy_positions(Enemy_manager* em,float dt){
	for(int i = 0;i<em->count;i++){
		float vel_x = em->velocity[i].x;
		float vel_y = em->velocity[i].y;
		float normalising_factor = dt*ENEMY_SPEED / sqrt(vel_x*vel_x+vel_y*vel_y);
		vel_x *= normalising_factor;
		vel_y *= normalising_factor;
		em->position[i].x += vel_x;
		em->position[i].y += vel_y;
	}	
}

Enemy_manager* create_enemy_manager(int size){
	Enemy_manager* em = malloc(sizeof(Enemy_manager));
	em->health = malloc(size*sizeof(int));
	em->position = malloc(size*sizeof(Vector2));
	em->velocity = malloc(size*sizeof(Vector2));
	em->type = malloc(size*sizeof(Enemy_type));
	em->query_arr = malloc(size*sizeof(Vector2));
	em->capacity = size;
	em->count = 0;
	return em;
}

void free_enemy_manager(Enemy_manager* em){
	free(em->health);
	free(em->position);
	free(em->velocity);
	free(em->query_arr);
	free(em->type);
}


void update_enemies(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm, float dt){
	enemy_separation(tree,em,player,cm);
	update_enemy_positions(em,dt);
	rebuild_tree(tree,em,cm);
}
