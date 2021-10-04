String Pattern-Matching Algorithm

Included Files:
slist.c- implementation of a single linked list with the fields head node, tail node and size. In addition to init, append, prepend, pop_first, append_list and destroy functions.
pattern_matching.c- implementation of a dictionary-matching algorithm that locates elements of a finite set of strings and uses slist.c.


Notes:
- Memory allocation and freeing for pm_t* must be done in main (pm_destroy(pm_t *) does not free the pointer to the fsm). 
 Meaning user must call pm_destroy(pm_t*) and then free(pm_t*) in main.
- Memory allocation and freeing for slist_t* must be done in main (slist_destroy(slist_t *, slist_destroy_t) does not free the pointer to the list).
Meaning user must call slist_destroy(slist_t*) and then free(slist_t*) in main.
- Data acquired from slist_pop_first(slist_t*) must be freed in main.
- List returned from pm_search must be freed in main.


