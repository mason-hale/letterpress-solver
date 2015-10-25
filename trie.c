/* 
 * 
 * Letterpress "Best Words" Solver
 * By Jason Stein and Mason Hale
 *
 */


/*
 * Function Libraries
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "common.h"


// user input will be stored here
char available[LIST_MAX_LENGTH];

int main(int argc, char* argv[])
{
    // start the clock!
    getrusage(RUSAGE_SELF, &before_all);
    
    // trie root 
    trie_node* root = calloc(1,sizeof(trie_node));
    root->stored_word = NULL;
    
    // all possible words linked list head
    list_node* head = NULL;

    // queue for finalists (top 25) linked list nodes
    queue q = {NULL, NULL};
    
    // benchmarks
    double time_load = 0.0, time_find = 0.0, time_unload = 0.0, 
        time_total = 0.0;
   
    // if input is valid, parse arguments
    if (verify_input(argc, argv[1],argv[2],argv[3]))
    {
        // concatenate the command line arguments into one string
        strcat(available, argv[1]);
        strcat(available, argv[2]);
        strcat(available, argv[3]);
        // make all letters lowercase for convenience
        for(int i = 0, length = strlen(available); i < length; i++)
        {
            available[i] = tolower(available[i]);
        }
        printf("Available letters: %s\n", available); 
    }
    else
        return 1;
    
    // attempt to load the dictionary
    getrusage(RUSAGE_SELF, &before);
    if (!load(DICTIONARY, root))
    {
        printf("error: failed to load dictionary\n");
        return 1;
    }
    getrusage(RUSAGE_SELF, &after);
    time_load = calculate(&before, &after);
    
    int letters[ALPH_SIZE] = {0};
    
    for (int i = 0; i < strlen(available); i++)
    {
        letters[available[i] - 'a']++;
    }
    
    // some tests we used in developing the project
    /*
    assert(search("mason", root));
    assert(search("butts", root));
    assert(search("agammaglobulinemias", root));
    assert(!search("asdfgh", root));
    assert(!search("backlightedd", root));
    assert(search("backlighted",root));
    assert(search("uncopyrightable",root));
    */
    
    getrusage(RUSAGE_SELF, &before);
   
    head = find_words(letters, root, head);
    
    q = find_finalists(head, q, argv[1], argv[2]);
    
    getrusage(RUSAGE_SELF, &after);
    time_find = calculate(&before, &after);
 
    getrusage(RUSAGE_SELF, &before);    
    if (!free_list(head) || !free_list(q.front))
    {
        return 1;
    }
    head = NULL;
    unload(root);
    getrusage(RUSAGE_SELF, &after);
    time_unload = calculate(&before, &after);
    
    // stop the clock!
    getrusage(RUSAGE_SELF, &after_all);
    time_total = calculate(&before_all, &after_all);
    
    printf("Time to load: %f\n",time_load);
    printf("Time to find words: %f\n",time_find);
    printf("Time to unload: %f\n",time_unload);
    printf("Total program time: %f\n",time_total);
    return 0;
}



