#include "game.h"

bool circle_contains_point(Vector2 centre, Vector2 point, float radius) {
    float dx = point.x - centre.x;
    float dy = point.y - centre.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool is_square_overlap(Vector2 centre1, Vector2 centre2, float radius1, float radius2) {
    return !(
        centre1.x - radius1 > centre2.x + radius2 || 
        centre1.x + radius1 < centre2.x - radius2 || 
        centre1.y - radius1 > centre2.y + radius2 || 
        centre1.y + radius1 < centre2.y - radius2
    );
}

void free_qtree(QTree* tree) {
    if (!tree){
        return;
    }
    if (tree->tl != NULL) {
        if (tree->tl) free_qtree(tree->tl);
        if (tree->tr) free_qtree(tree->tr);
        if (tree->bl) free_qtree(tree->bl);
        if (tree->br) free_qtree(tree->br);
    }
    
    free(tree->points);
    free(tree->id);
    free(tree);
}

void rebuild_tree(QTree* tree,Enemy_manager* em,Chunk_manager* cm){
	reset_quadtree(tree);
	for (int i = 0;i<em->count;i++){
		Vector2 tree_pos = vec2_sub(em->position[i],cm->offset);
		qtree_insert(tree,tree_pos,i);
	}
}

void reset_quadtree(QTree* tree){
	tree->num_points = 0;
	if (tree->is_divided){
		// keep the subtrees in memory but mark tree as not divided
		reset_quadtree(tree->tl);
		reset_quadtree(tree->tr);
		reset_quadtree(tree->bl);
		reset_quadtree(tree->br);
		tree->is_divided = false;
	}
}

void qtree_insert(QTree* tree, Vector2 pt,int id) {
	if (tree->is_divided){
		qtree_insert(subtree_decide(tree,pt),pt,id);
		return;
	}
    if (tree->num_points < POINTS_PER_QUAD ) {
        tree->points[tree->num_points] = pt;
        tree->id[tree->num_points] = id;
        tree->num_points++;
        return;
    }
   // runs if the current quad is full
    subdivide_qtree(tree);
    for (int i = 0; i < POINTS_PER_QUAD; i++) {
        qtree_insert(subtree_decide(tree,tree->points[i]),tree->points[i],tree->id[i]);
    }
    qtree_insert(subtree_decide(tree,pt),pt,id);
}

void qtree_query(QTree* tree,Vector2 centre,float radius, Enemy_manager* em,int* num_points){
	if (!is_square_overlap(tree->centre,centre,tree->radius,radius)){
		return;
	}
	if (tree->is_divided){
		qtree_query(tree->tl,centre,radius,em,num_points);
		qtree_query(tree->tr,centre,radius,em,num_points);
		qtree_query(tree->bl,centre,radius,em,num_points);
		qtree_query(tree->br,centre,radius,em,num_points);
		return;
	}

    for (int i = 0; i < tree->num_points; i ++) {
    	if (tree->points[i].x == centre.x && tree->points[i].y == centre.y)continue;
        if (circle_contains_point(centre, tree->points[i],radius)) {
            em->query_arr[*num_points] = tree->id[i];
            (*num_points)++;
        }
    }
}

void subdivide_qtree(QTree* tree) {
	if (tree->tl != NULL){
		tree->is_divided = true;
		return;
	}
    float x = tree->centre.x;
    float y = tree->centre.y;
    float r = tree->radius/2.0f;

    Vector2 tl = {x-r,y-r};
    Vector2 tr = {x+r,y-r};
    Vector2 bl = {x-r,y+r};
    Vector2 br = {x+r,y+r};

    tree->tl = create_quad_tree(tl,r);
    tree->tr = create_quad_tree(tr,r);
    tree->bl = create_quad_tree(bl,r);
    tree->br = create_quad_tree(br,r);
    tree->is_divided = true;
}

QTree* subtree_decide(QTree* tree,Vector2 point){
	bool top = point.y < tree->centre.y;
	bool right = point.x > tree->centre.x;
	if (top){ 
		if (right)
			return tree->tr;
		return tree->tl;
	}
	if (right)
		return tree->br;
	return tree->bl;
}

QTree* create_quad_tree(Vector2 centre, float radius){
	QTree* qtree = malloc(sizeof(QTree));
	qtree->is_divided = false;
	qtree->centre = centre;
	qtree->radius = radius;
	qtree->points = malloc(POINTS_PER_QUAD*sizeof(Vector2));
	qtree->id = malloc(POINTS_PER_QUAD*sizeof(int));
	qtree->num_points = 0;
	qtree->tl=NULL;
	qtree->tr=NULL;
	qtree->bl=NULL;
	qtree->br=NULL;
	return qtree;
}
