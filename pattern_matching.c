#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pattern_matching.h"

int pm_init(pm_t * fsm){
    if(!fsm){return -1;}
    fsm->newstate=1; 
    fsm->zerostate= (pm_state_t*) malloc(sizeof(pm_state_t));
    if(!fsm->zerostate){return -1;}
    fsm->zerostate->id= 0;
    fsm->zerostate->depth=0;
    fsm->zerostate->fail=NULL;

    fsm->zerostate->output= (slist_t*)malloc(sizeof(slist_t));
    if(!fsm->zerostate->output){
        free(fsm->zerostate);
        return -1;
    } 
    slist_init(fsm->zerostate->output);

    fsm->zerostate->_transitions=(slist_t*)malloc(sizeof(slist_t));
    if(!fsm->zerostate->_transitions){
        free(fsm->zerostate->output);
        free(fsm->zerostate);
        return -1;
    }
    slist_init(fsm->zerostate->_transitions);
    return 0;
}

// set new edge from from_state to to_state with given char
int pm_goto_set(pm_state_t *from_state, unsigned char symbol, pm_state_t *to_state){
    pm_labeled_edge_t* edge= (pm_labeled_edge_t*) malloc(sizeof(pm_labeled_edge_t));
    if(!edge || !from_state || !to_state)
        return -1;
    edge->label = symbol;
    edge->state = to_state;
    printf("%d ->%c-> %d\n", from_state->id, symbol, to_state->id);
    if(slist_append(from_state->_transitions, edge)== -1){
        free(edge);
        return -1;
    }
    return 0;
}

// get transition from state with given char
pm_state_t* pm_goto_get(pm_state_t *state, unsigned char symbol){
    slist_node_t* node= slist_head(state->_transitions);
    while(node){
        if( ((pm_labeled_edge_t*)slist_data(node))->label == symbol) {
            return ((pm_labeled_edge_t*)slist_data(node))->state;
        }
        node= slist_next(node);
    }
    return NULL;
}

// add strings and accepting states to the finite state machine
int pm_addstring(pm_t *fsm, unsigned char *symbol, size_t n){
    if(strlen(symbol)!= n || fsm==NULL){return -1;}
    int i=0;
    pm_state_t* current_state= fsm->zerostate;
    // while a transition from current with symbol exists 
    while(pm_goto_get(current_state, symbol[i]) && i<n){
        current_state= pm_goto_get(current_state, symbol[i]);
        i++;
    }
    while(i<n){
        //if no edge exists from current create new state and init all it's fields
        pm_state_t* new_state= (pm_state_t*)malloc(sizeof(pm_state_t));
        if(!new_state)
            return -1;
        new_state->id= fsm->newstate;
        printf("Allocating state %d\n", new_state->id);
        new_state->depth= (current_state->depth)+1;
        new_state->fail= NULL;

        new_state->output= (slist_t*)malloc(sizeof(slist_t));
        if(!new_state->output){
            free(new_state);
            return -1;
        }
        slist_init(new_state->output);

        new_state->_transitions=(slist_t*) malloc(sizeof(slist_t));
        if(!new_state->_transitions){
            free(new_state->output);
            free(new_state);
            return -1;
        }
        slist_init(new_state->_transitions);

        // set a new edge from current state to new state
        pm_goto_set(current_state, symbol[i], new_state);
        fsm->newstate= fsm->newstate +1;
        
        current_state= new_state;
        i++;
    }
    // append given string to state's output list
    if(slist_append(current_state->output, symbol)== -1){return -1;
    }

    return 0;
}

int pm_makeFSM(pm_t *fsm){
    slist_t* queue= (slist_t*)malloc(sizeof(slist_t));
    if(!queue || !fsm)
        return -1;
    slist_init(queue);
    slist_node_t* node= slist_head(fsm->zerostate->_transitions);
    pm_state_t* current_state= fsm->zerostate;
    pm_state_t* next_state= NULL;
    pm_state_t* state= NULL;
    // set failure of states with depth 1 to zerostate and append them to queue
    while(node){
        next_state= pm_goto_get(current_state, *(char*)slist_data(node));
        if(next_state != NULL){
            if(slist_append(queue, next_state) == -1){
                free(queue);
                return -1;
            }
            next_state->fail= fsm->zerostate;
            printf("setting f(%d)= %d\n", next_state->id, next_state->fail->id);
        }
        node= slist_next(node);
    }
    // while queue !empty pop current_state(failure already set) and set next_state's failure 
    current_state=slist_pop_first(queue);
    while( current_state != NULL ){
        node= slist_head(current_state->_transitions);
        while(node != NULL){
            next_state= pm_goto_get(current_state, *(char*)slist_data(node));
            if(next_state != NULL){
                if(slist_append(queue, next_state) == -1){
                    free(queue);
                    return -1;
                }
                state= current_state->fail;
                // loop until transition is found or NULL state is reached
                while(state!=NULL && pm_goto_get(state, *(char*)slist_data(node)) == NULL){
                        state= state->fail;
                }
                printf("setting f(%d)= ", next_state->id);
                //if NULL state is reached there is no transition and failure is zerostate 
                if(state== NULL){
                    next_state->fail= fsm->zerostate;
                    printf("%d\n", next_state->fail->id);
                }
                //else failure is the transition found
                else{
                    next_state->fail= pm_goto_get(state, *(char*)slist_data(node));
                    printf("%d\n",next_state->fail->id);
                }
            }
            // append failure state's output to next_state's output 
            if(slist_append_list(next_state->output, next_state->fail->output)== -1){
                return -1;
            }
            node= slist_next(node);
        }
        current_state=slist_pop_first(queue);
    }
    slist_destroy(queue, SLIST_LEAVE_DATA);
    free(queue);
    return 0;
}

// search the fsm for patern matches in given text of size n, starting from state
slist_t* pm_fsm_search(pm_state_t *state, unsigned char *text, size_t n){
    // allocate and initialize list of matches
    slist_t* matches= (slist_t*)malloc(sizeof(slist_t));
    if(!matches || !state){return NULL;}
    slist_init(matches);
    pm_state_t* current_state= state;
    slist_node_t* node= NULL;
    for(int i=0; i<n; i++){
        // loop until transition with char at i is found or NULL state is reached 
        while( current_state!=NULL && pm_goto_get(current_state, text[i])== NULL ){
            current_state= current_state->fail;
        }
        // if current_state is NULL then there is no transition with char in the fsm
        // so move on to next char 
        if(current_state==NULL){
            current_state= state;
            continue;
        }
        current_state= pm_goto_get(current_state, text[i]);

        // if output list isn't empty create matches for all patterns in it
        if(slist_size(current_state->output) > 0){
            node=current_state->output->head;
            while(node != NULL){
                pm_match_t* match= (pm_match_t*) malloc(sizeof(pm_match_t));
                if(!match){
                    slist_destroy(matches, SLIST_FREE_DATA);
                    free(matches);
                    return NULL; 
                }
                // starting position goes back from i the length of current matched pattern
                // since the match is detected on its last character
                match->start_pos= (i-strlen((char*)slist_data(node))) +1; 
                match->end_pos= i;
                match->pattern= slist_data(node);
                match->fstate= current_state;
                if(slist_append(matches, match)==-1){
                    free(match);
                    slist_destroy(matches, SLIST_FREE_DATA);
                    free(matches);
                    return NULL;
                }
                printf("Pattern: %s, starts at: %d, ends at: %d, last state= %d\n", match->pattern,
                                         match->start_pos, match->end_pos, match->fstate->id);
                node= slist_next(node);
            }
        }
    }
    return matches;
}

void pm_destroy(pm_t *fsm){
    slist_t* queue= (slist_t*)malloc(sizeof(slist_t));     
    if(!queue || !fsm){return;}
    slist_init(queue);
    slist_node_t* node= slist_head(fsm->zerostate->_transitions);
    pm_state_t* current_state= fsm->zerostate;
    pm_state_t* next_state= NULL;
    // append each state's transitions to queue then free current state 
    while( current_state != NULL ){
        node= slist_head(current_state->_transitions);
        while(node != NULL){
            next_state= pm_goto_get(current_state, *(char*)slist_data(node));
            if(next_state != NULL){
                if(slist_append(queue, next_state) == -1){
                    free(queue);
                    return;
                }
            }
            node= slist_next(node);
        }
        slist_destroy(current_state->output, SLIST_LEAVE_DATA);
        free(current_state->output);
        slist_destroy(current_state->_transitions, SLIST_FREE_DATA);
        free(current_state->_transitions);
        free(current_state);
        current_state= slist_pop_first(queue);
    }
    slist_destroy(queue, SLIST_FREE_DATA);
    free(queue);
    // free(fsm);
}