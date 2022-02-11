#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "utils.h"

void initialize_ai();

void find_solution( state_t* init_state );
void free_memory(unsigned expanded_nodes);
void free_expanded(node_t** all_expanded_nodes, int number_of_nodes);

#endif
