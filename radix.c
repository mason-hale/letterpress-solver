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

// header file containing functions common to both implementations
#include "common.h"

// radix tree function prototypes
int only_child(trie_node* node);
trie_node* radix_collapse(trie_node* node);

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
    
    // some tests we used in developing the project
    /* 
    assert(search("mason", root));
    assert(search("butts", root));
    assert(search("agammaglobulinemias", root));
    assert(!search("asdfgh", root));
    */
    
    // collapse to a radix tree
    root = radix_collapse(root);
    
    getrusage(RUSAGE_SELF, &after);
    time_load = calculate(&before, &after);
    
    int letters[ALPH_SIZE] = {0};
    
    for (int i = 0; i < strlen(available); i++)
    {
        letters[available[i] - 'a']++;
    }
    
    // more of those
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
    
    // find words
    head = find_words(letters, root, head);
    q = find_finalists(head, q, argv[1], argv[2]);
    
    getrusage(RUSAGE_SELF, &after);
    time_find = calculate(&before, &after);
    
    getrusage(RUSAGE_SELF, &before);
    
    // free all memory
    if (!free_list(head) || !free_list(q.front))
    {
        return 1;
    }  
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

int only_child(trie_node* node)
{
    int count = 0;
    int index = 0;
    
    for(int i = 0; i < ALPH_SIZE; i++)
    {
        if (node->children[i] != NULL)
        {
            count++;
            index = i;
        }
    } 
    
    if (count == 0)
    {
        return -1;
    }
    if (count > 1)
    { 
        return -2;
    }
    return index;
}

trie_node* radix_collapse(trie_node* node)
{
    // try to find the index of the node's only child
    int child_index = only_child(node);
    // if the node has no children, simply return the node
    if (child_index == -1)
    {
        return node;
    }
    // if the node has more than one child, try to collapse all of them
    if (child_index == -2)
    {
        for(int i = 0; i < ALPH_SIZE; i++)
        {
            trie_node* temp = node->children[i];
            if (temp != NULL)
            {
                node->children[i] = radix_collapse(temp);
            }
        }
        return node;
    }
    // get the pointer to the only child node
    trie_node* child_node = node->children[child_index];
    // don't collapse onto a node with a stored word (or things will get messy)
    if (strlen(node->stored_word) != 0)
    {
        node->children[child_index] = radix_collapse(child_node);
        return node;
    }
    // this is where we actually start collapsing
    // update the node's substring to include the letter that points to the 
    // only child
    char child_letter = child_index + 'a';
    if (node->substring == NULL)
    {
        node->substring = calloc(DICT_MAX_LENGTH,sizeof(char));
        node->substring[0] = child_letter;
    }
    else
    {
        int length = strlen(node->substring);
        node->substring[length] = child_letter;
        node->substring[length + 1] = '\0';
    }
    // replace the node's children with its child's children
    for (int i = 0; i < ALPH_SIZE; i++)
    {
        node->children[i] = child_node->children[i];
    }
    // if the child node stored a word, now store it at the parent
    if (child_node->stored_word != NULL)
    {
        strcpy(node->stored_word, child_node->stored_word);
    }
    // free the child node we just eliminated
    free(child_node->stored_word);
    free(child_node);
    // try to collapse this node further
    radix_collapse(node);
    return node;
}
