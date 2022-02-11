#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	
	node_t* n = solution_node;

	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action){

	node_t* new_node = malloc(sizeof (*new_node));

	//copies the state of the previous board state
	copy_state(&(new_node->state), &(n->state));
	
	new_node->state.cursor = *selected_peg;
	new_node->depth = (n->depth)+1;
	new_node->move = action;
	new_node->parent = n;
    execute_move_t( &(new_node->state), &(new_node->state.cursor), action );
	return new_node;
}

/**
 * Find a solution path as per algorithm description in the handout
 */

void find_solution(state_t* init_state){

	HashTable table;
	position_s* selected_peg = malloc(sizeof(*selected_peg));
	node_t** all_expanded_nodes;
	//Stores all the nodes that are popped out of stack to be freed later
	all_expanded_nodes = malloc(sizeof(*all_expanded_nodes)*(budget+1));
	//check if there is a possible move in the board and if we are not stuck
	int has_next_depth = 0;

	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, 
		           sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	node_t* n = create_init_node(init_state);
	//Add the initial node into stack
	stack_push(n);
	int remaining_peg = num_pegs(&(n->state)); 
	node_t* new_node = NULL;
	
	while(!is_stack_empty()){
		//The next possible game node
		n = stack_top();
		stack_pop();
		all_expanded_nodes[expanded_nodes++] = n;
		/*if number of pegs in the new node to be considered is better than 
		    the current game node*/
		if (num_pegs(&(n->state)) < remaining_peg){
			save_solution(n);
			remaining_peg = num_pegs(&(n->state));
		}
		//for all possible moves in that board state
		for (int x=0; x<SIZE; x++) {
			for (int y=0; y<SIZE; y++) {
				selected_peg->x = x;
				selected_peg->y = y;
				for(int i = left; i <= down; i++){
					/*If the selected peg can move in the specified direction 
					on the board*/
					if(can_apply(&(n->state), selected_peg, i)){
						has_next_depth = 1;
						new_node = applyAction(n, selected_peg, i);
						generated_nodes += 1;
						//If game is won (1 peg left on the board)
						if(won(&(new_node->state))){
							save_solution(new_node);
							remaining_peg = num_pegs(&(new_node->state));
							ht_destroy(&table);
							free_stack();
							free(selected_peg);
							free_expanded(all_expanded_nodes, expanded_nodes);
							free(new_node);
							return;
						}
						//Add new node into stack if it is not a duplicate
						if(!ht_contains(&table, new_node->state.field)){
							ht_insert(&table, new_node->state.field, 
								                 new_node->state.field);
							stack_push(new_node);
						}
						//Free the node if it is a duplicate node
						else{
							free(new_node);
						}
					}
				}
			}
		}
		//If there is no possible moves for this board state
		if(!has_next_depth){
			/*Go back to the parent node and try a different move 
			   (reset the number of remaining pegs)*/
			remaining_peg = num_pegs(&(new_node->parent->state));
			has_next_depth = 0;
		}

		//If number of expanded nodes hits the budget before we find a solution
		if(expanded_nodes >= budget){
			ht_destroy(&table);
			free_stack();
			free(selected_peg);
			free_expanded(all_expanded_nodes, expanded_nodes);
			return;
		}
	}	
}

/**
 * Frees the expanded nodes that were popped out of the stack
*/
void free_expanded(node_t** all_expanded_nodes, int number_of_nodes){
	for(int i=0; i<number_of_nodes; i++){
		free(all_expanded_nodes[i]);
	}
	free(all_expanded_nodes);
}