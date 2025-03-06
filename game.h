#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TILE 5
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800
#define PLAYER_SPEED 20.0f
#define ENEMY_SPEED 5.0f
#define POINTS_PER_QUAD 10
#define MAX_ENEMIES 10000
#define PERCEPTION_RADIUS 15.0f // used for boid separation

typedef struct {
	// each byte has 4 tiles, so the chunk is 16x16
    uint8_t tiles[64]; 
} Tile_chunk;

typedef struct {
    Tile_chunk*** chunks;  // main chunks array
    Color* colour_buf;	  // colour array for rendering
    Vector2 offset;       // absolute coordinates of "centre"
    uint8_t width;        // Width (in chunks)
    uint8_t height;       // Height (in chunks)
} Chunk_manager;

typedef struct{
    double x;
    double y; 
    int health;
    Texture2D sprite;
} Player;

typedef struct QTree_{
    bool is_divided;
    Vector2 centre;
    float radius;
    Vector2* points;
    int num_points;

    struct QTree_* tl;
    struct QTree_* tr;
    struct QTree_* bl;
    struct QTree_* br;
}QTree;

typedef enum {
    ENEMY_ZOMBIE,
    ENEMY_BAT,
    ENEMY_GHOST,
    ENEMY_WEREWOLF,
    ENEMY_BOSS,
    ENEMY_TYPE_COUNT
} Enemy_type;

typedef struct {
    int* health;
    Vector2* position;
    Vector2* velocity;
    Vector2* query_arr;
    Enemy_type* type;
    int capacity;
    int count;
} Enemy_manager;



// from main
Player init_player();
void draw_map(Texture2D* tex,Chunk_manager* cm,Player* player);
void draw_player(Player* player);
void update_player(Player* player, float dt);
int main();

// from terraingen
void refresh_map(Chunk_manager* cm,Texture2D* tex);
Chunk_manager* create_chunk_manager(int16_t width, int16_t height);
Tile_chunk* create_chunk(int16_t x,int16_t y);
void add_chunk(Chunk_manager* cm, int index_x, int index_y);
void update_chunks(Player* player, Chunk_manager* cm);
void init_chunks(Chunk_manager* cm);
void free_chunk_manager(Chunk_manager* cm);

// from enemies
void enemy_separation(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm);
void add_enemy(Enemy_manager* em, QTree* tree,Vector2 pos,Chunk_manager* cm);
void update_enemy_positions(Enemy_manager* em,float dt);
Enemy_manager* create_enemy_manager(int size);
void update_enemies(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm, float dt);
void free_enemy_manager(Enemy_manager* em);


// from noise
float fract(float n);
float hash1(float px, float py);
float noise(float x, float y);
uint8_t pnoise2d(double x, double y);

// from quadtree
void qtree_query(QTree* tree,Vector2 centre,float radius, Enemy_manager* em,int* num_points);
void subdivide_qtree(QTree *qtree);
void qtree_insert(QTree* tree, Vector2 pt);
QTree* subtree_decide(QTree* tree,Vector2 pt);
QTree* create_quad_tree(Vector2 centre,float radius);
void remove_enemy_tree(Vector2 pos,QTree* tree,int id);
void reset_quadtree(QTree* tree);
void rebuild_tree(QTree* tree,Enemy_manager* em,Chunk_manager* cm);
void free_qtree(QTree* tree);
bool square_contains_point(Vector2 square_centre,Vector2 point,float radius);
bool is_square_overlap(Vector2 centre1, Vector2 centre2, float radius1, float radius2);

#endif