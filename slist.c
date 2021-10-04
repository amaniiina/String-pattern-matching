#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "slist.h"

#define DEBUG
#if defined(DEBUG)
//function to print all data in list
void print_list(slist_t* list){
    slist_node_t *n= slist_head(list);
    for(int i=0; i< slist_size(list); i++){
        printf("%c->", *(char*)(slist_data(n)) );
        if(i== slist_size(list)-1)
            printf("NULL\n");
        n= slist_next(n);
    }
}
#endif

// initialize list which has been already allocated
void slist_init(slist_t *list){
    if(!list){return;}
    slist_size(list)= 0;
    slist_head(list)= NULL;
    slist_tail(list)= NULL;
}

void slist_destroy(slist_t *list, slist_destroy_t dealloc){
    if(!list){return;}
    slist_node_t *n = NULL;
    while((n = slist_head(list)) != NULL) {
        slist_head(list)= slist_next(n);
        if(dealloc == SLIST_FREE_DATA && slist_data(n)!= NULL){
            free(slist_data(n));
        }
        free(n);
    }
    // free(list);
}

// returns data of first node in list 
void *slist_pop_first(slist_t * list){
    if(list== NULL || slist_head(list)==NULL)
        return NULL; 
    slist_node_t *n= slist_head(list);
    slist_head(list)= slist_next(n);
    void* p= slist_data(n); 
    free(n);
    slist_size(list)--;
    return p;
}

// adds new node to end of list
int slist_append(slist_t * list, void *data){
    slist_node_t *new_node= (slist_node_t*) malloc(sizeof(slist_node_t));
    if(!new_node || !list || !data)
        return -1;
    slist_data(new_node)= data;
    slist_next(new_node)= NULL;
    if(slist_size(list)==0){
        slist_head(list)= new_node;
        slist_tail(list)=slist_head(list);
    }else{
        slist_next(slist_tail(list))= new_node;
        slist_tail(list)= new_node;
    }
    slist_size(list)++;
    return 0;
}

// adds new node to beginning of list
int slist_prepend(slist_t *list, void *data){
    slist_node_t *new_node= (slist_node_t*) malloc(sizeof(slist_node_t));
    if(!new_node || !list || !data){
        return -1;
    }
    slist_data(new_node)= data;
    if(slist_size(list)==0){
        slist_next(new_node)=NULL;    
        slist_head(list)=new_node;
        slist_tail(list)=slist_head(list);
    }
    else{
        slist_next(new_node)= slist_head(list);
        slist_head(list)= new_node;
    }
    slist_size(list)++;
    return 0;
}

// appends list from to list to
int slist_append_list(slist_t* to, slist_t* from){
    if(to==NULL || from==NULL )
        return -1;
    if(slist_head(from)==NULL)
        return 0;
        
    slist_node_t *n= slist_head(from);
    while(n!=NULL){
        slist_append(to, slist_data(n));
        n= slist_next(n);
    }
    return 0;
}
