#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

/*
 * Global Constants
 */
#define ALPH_SIZE 26
#define DICT_MAX_LENGTH 45
#define LIST_MAX_LENGTH 25
#define DICTIONARY "words.txt"
#define RESULTS 25

/*
 * ADTs for Trie, Linked List, and Queue
 */
typedef struct trie_node 
{
    char* substring;
    char* stored_word;
    struct trie_node* children [ALPH_SIZE];
}
trie_node;

typedef struct list_node
{
    char* stored_word;
    int score;
    struct list_node* next;
}
list_node;

typedef struct queue
{
    list_node* front;
    list_node* rear;
}queue;

// structs for timing data
struct rusage before, after, before_all, after_all;


// trie function prototypes 
void trie_insert(char* word, trie_node* root);
bool load(const char* dictionary, trie_node* root);
void unload(trie_node* n);
bool search(char* query, trie_node* root);

// list function prototypes 
list_node* list_insert(char* word, list_node* head);
bool free_list(list_node* head);

// queue function prototypes
queue enqueue (queue q, list_node* node);
queue dequeue (queue q);

// user input function prototypes
bool verify_input (int num, char* first, char* second, char* third);
bool check_alpha(char* word);

// general functionality prototypes
list_node* find_words(int* letters, trie_node* trie, list_node* head);
int score_word(char* word, char* list1, char* list2);
queue find_finalists (list_node* head, queue q, char* string1, char* string2);
void print_finalists (list_node* front);

// time calculation
double calculate(const struct rusage* b, const struct rusage* a);


#endif
