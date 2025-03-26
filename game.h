#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define FRAMERATE 60
#define NORMALIZE_DIAGONAL 0.707
#define TILE 5
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800
#define PLAYER_SPEED 20.0f
#define ENEMY_SPEED 6.0f
#define POINTS_PER_QUAD 10
#define MAX_ENEMIES 10000
#define PERCEPTION_RADIUS 10.0f // used for boid separation
#define ATTACK_VARIANTS 3
#define MAX_BULLETS 400
#define BULLET_TTL 3.0f
#define INVINCIBILITY_TIME 0.15f
#define FIREBALL_SPEED 60.0f
#define KNIFE_SPEED 60.0f
#define SHURIKEN_RADIUS 30.0f
#define PLAYER_PERCEPTION 60.0f
#define ENEMY_HURT_TINT (Color){ 212, 129, 129, 255 }



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

typedef enum{
    FIREBALL,
    KNIFE,
    SHURIKEN
} Attack_type;

typedef struct{
    Vector2* position;
    Vector2* velocity;  
    Attack_type* attack;
    uint16_t count;
    float* live_time;
    int8_t* pierce;
    bool* belongs_to_player;
    uint8_t* frame;
} Bullet_manager;

typedef struct{
    // number of frames to the side
    Texture2D skeleton; // 4
    Texture2D goblin; // 4
    Texture2D imp; // 4
    Texture2D fireball; // 4
    Texture2D knife; // 1
    Texture2D shuriken;
    Color* state_colours;
} Sprite_manager;

typedef struct{
    Vector2 position;
    int health;
    Texture2D sprite;
    int* damage;
    int8_t* pierce;
    float* cooldown;
    uint8_t* projectiles_per_shot;
} Player;

typedef struct QTree_{
    bool is_divided;
    Vector2 centre;
    float radius;
    Vector2* points;
    uint16_t* id;
    int8_t num_points;

    struct QTree_* tl;
    struct QTree_* tr;
    struct QTree_* bl;
    struct QTree_* br;
}QTree;

typedef enum {
    ENEMY_IMP,
    ENEMY_GOBLIN,
    ENEMY_SKELETON
} Enemy_type;

typedef struct {
    int* health;
    Vector2* position;
    Vector2* velocity;
    double* time_hit;
    bool* is_hit;
    Enemy_type* type;
    uint8_t* frame;
    uint16_t count;
    uint64_t kills;

    // this is used for quadtree queries
    // and contains indexes for the arrays in this struct
    uint16_t* query_arr;
} Enemy_manager;


// from main
Player* init_player();
void draw_map(Texture2D* tex,Chunk_manager* cm,Player* player);
void draw_player(Player* player);
void draw_enemies(Enemy_manager* em, Player* player,Sprite_manager* sm);
void update_player(Player* player, float dt);
int main();

// from terraingen
void refresh_map(Chunk_manager* cm,Texture2D* tex);
Chunk_manager* create_chunk_manager(uint8_t width, uint8_t height);
Tile_chunk* create_chunk(int16_t x,int16_t y);
void add_chunk(Chunk_manager* cm, int index_x, int index_y);
void update_chunks(Player* player, Chunk_manager* cm, Texture2D* map_tex);
void init_chunks(Chunk_manager* cm);
void free_chunk_manager(Chunk_manager* cm);

// from enemies
void enemy_separation(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm);
void add_enemy(Enemy_manager* em, Vector2 pos,Enemy_type type);
void remove_enemy(Enemy_manager* em, int id);
void update_enemy_positions(Enemy_manager* em,float dt);
Enemy_manager* create_enemy_manager(int size);
void update_enemies(QTree* tree,Enemy_manager* em,Player* player,Chunk_manager* cm, Bullet_manager* bm, float dt);
void free_enemy_manager(Enemy_manager* em);
void take_damage(int id,int damage);
void add_bullet(Bullet_manager* bm,Attack_type type,int8_t pierce,Vector2 position,Vector2 velocity,bool belongs_to_player);
void remove_bullet(Bullet_manager* bm, int id);
void update_bullet_positions(Bullet_manager* bm,Player* player,float dt);
void update_bullets(Bullet_manager* bm,Enemy_manager* em, Chunk_manager* cm,QTree* tree,Player* player,float dt,double time);
void update_health(Enemy_manager* em,double time);
void shoot_at_player(Bullet_manager* bm,Player* player, Enemy_manager* em, int id);
Bullet_manager* create_bullet_manager(int size);
void player_shoot_fireball(Player* player,QTree* tree,Chunk_manager* cm,Enemy_manager* em,Bullet_manager* bm);
void player_shoot_knives(Player* player,Bullet_manager* bm, bool alt_fire);
void player_shoot_shuriken(Player* player, Bullet_manager* bm);
void advance_frames(Enemy_manager* em,Bullet_manager* bm,uint64_t frame);

// from noise
float fract(float n);
float hash1(float px, float py);
float noise(float x, float y);
uint8_t pnoise2d(double x, double y);
void scale_vec(Vector2* in,float scale);
void vec2_rotate(Vector2* a, float angle);
Vector2 vec2_sub(Vector2 a, Vector2 b);
Vector2 vec2_add(Vector2 a, Vector2 b);
Vector2 vec2_mult(Vector2 a, float b);
Vector2* closest_enemies(Vector2* relative_positions, int size);

// from quadtree
void qtree_query(QTree* tree,Vector2 centre,float radius, Enemy_manager* em,int* num_points);
void subdivide_qtree(QTree *qtree);
void qtree_insert(QTree* tree, Vector2 pt,int id);
QTree* subtree_decide(QTree* tree,Vector2 pt);
QTree* create_quad_tree(Vector2 centre,float radius);
void remove_enemy_tree(Vector2 pos,QTree* tree,int id);
void reset_quadtree(QTree* tree);
void rebuild_tree(QTree* tree,Enemy_manager* em,Chunk_manager* cm);
void free_qtree(QTree* tree);
bool circle_contains_point(Vector2 square_centre,Vector2 point,float radius);
bool is_square_overlap(Vector2 centre1, Vector2 centre2, float radius1, float radius2);

#endif