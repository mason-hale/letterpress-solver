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

// checks to make sure users input correctly
bool verify_input (int num, char* first, char* second, char* third)
{
    if (num != 4)
    {
    
        /* The user will be entering three strings, described as follows:
         * 1) White space letters (unclaimed by either player)
         * 2) Light red space letters (claimed by opponent but up-for-grabs)
         * 3) Dark red space and blue space letters (not up-for-grabs--won't improve score if used)
         */
        
        printf("Usage: ./final OpponentPotentialString "\
        "UnclaimedString OpponentBlockedAndPlayer'sString \n");
        return false;
    }
    
    // make sure the user enters the right number of letters
    if (strlen(first) + strlen(second) + strlen(third) 
        != LIST_MAX_LENGTH)
    {
        printf("Please enter a total of 25 letters.\n");
        return false;
    }
    
    // and check that they're all letters
    if (!check_alpha(first) || !check_alpha(second) 
            || !check_alpha(third))
    {
        printf("Please only enter letters!\n");
        return false;
    }
        
    return true;
}

// inserts a single word into the trie
void trie_insert(char* word, trie_node* root)
{
    int length = strlen(word);
    // initialize a crawler that starts at the root node
    trie_node* crawl = root;
    for(int i = 0; i < length; i++)
    {
        // finds the 0-25 index of the letter
        int index = word[i] - 'a';
        // if there is already a pointer for that letter, crawl to the 
        // node it points to, then continue down
        if (crawl->children[index] != NULL)
        {
            crawl = crawl->children[index];    
        }
        // if there is no pointer for that letter at this level of the trie,
        // malloc one then crawl to it
        else 
        {
            crawl->children[index] = calloc(1, sizeof(trie_node));
            crawl->children[index]->stored_word = 
              calloc(DICT_MAX_LENGTH, sizeof(char));
            crawl = crawl->children[index];
        }
        // if we are at the last character of the word, copy it into the stored
        // word space of the node
        if (word[i + 1] == '\0')
        {
            strcpy((crawl->stored_word), word);
        }
    };
    return;
}

// loads an entire dictionary into the trie
bool load(const char* dictionary, trie_node* root)
{
    // attempt to open the dictionary filepath in read mode
    FILE* dict = fopen(dictionary, "r");
    if (dict == NULL)
    {
        return false;
    }
    
    // start reading words and inserting them until out of words
    char word [DICT_MAX_LENGTH];
    while((fscanf(dict, "%s", word)) == 1)
    {
        trie_insert(word, root);
    }
    
    fclose(dict);
    
    return true;
}

// checks if a given query is stored in the dictionary
bool search(char* query, trie_node* root)
{
    // initialize a crawler
    trie_node* crawl = root;
    // the offset represents how many characters from stored substring we 
    // have crawled. in a trie it will always be 0, but in the radix tree
    // it will increase every time we hit a substring
    int offset = 0;
    for (int i = 0; query[i + offset] != '\0'; i++)
    {
        if(crawl->substring != NULL)
        {
            int sub_length = strlen(crawl->substring);
            // crawl through the substring, checking each letter of the query
            // at the point we're at against the substring
            for(int j = 0; j < sub_length; j++)
            {
                if (query[i + j + offset] != crawl->substring[j])
                {
                    return false;
                }
            }
            // if the subsection of the query lines up with the stored
            // substring, increase the offset and continue down the tree
            offset += sub_length;
        }
        // gets the 0-25 index of the letter
        int index = query[i + offset] - 'a';
        // if there is no pointer at this node for the letter, then the word 
        // isn't there
        if (crawl->children[index] == NULL)
        {
            return (strcmp(query, crawl->stored_word) == 0);
        }
        // otherwise crawl to the next node
        else crawl = crawl->children[index];
    }
    
    // we are now at the last node, where this word should be stored,
    // so check that the stored word is indeed the query
    return (strcmp(query, crawl->stored_word) == 0);
}

// recursive helper function that unloads all children of a pointer to a node
void unload(trie_node* n)
{
    // base case
    if (n == NULL)
    {
        return;
    }
    
    for(int i = 0; i < ALPH_SIZE; i++)
    {
        // if the node has children, unload them first
        if (n->children[i] != NULL)
        {
            unload(n->children[i]);
            n->children[i] = NULL;
        }
    }
    
    // free all parts of the node
    if (n->substring != NULL)
    {
        free(n->substring);
    }
    free(n->stored_word);
    free(n);
    
}

// inserts a node into a linked list
list_node* list_insert(char* word, list_node* head)
{
    // allocate space for the new node (
    list_node* new_node = malloc(sizeof(list_node));
    
    if (new_node == NULL)
    {
        printf("Failed to allocate memory.");
        return NULL;
    }
    
    // allocate space to store the new node's word
    new_node->stored_word = calloc(LIST_MAX_LENGTH, sizeof(char));
    
    // store the word in the node
    strcpy(new_node->stored_word, word);
    
    // base case for empty list
    if (head == NULL)
    {
        new_node->next = NULL;
        head = new_node;
    }
    // insert into existing list
    else
    {
        new_node->next = head;
        head = new_node;
    }
    
    return head;
}

// frees all memory used by the linked list
bool free_list(list_node* head)
{
    // initialize a list_node pointer to act as a crawler
    list_node* crawler = head;
    
    // crawl through the list, freeing all memory
    while (crawler != NULL)
    {
        list_node* temp = crawler;
        crawler = crawler->next;
        free(temp->stored_word);
        free(temp);
    }
    
    return true;
}

// finds all possible words the user can construct with his/her game board
list_node* find_words(int* letters, trie_node* trie, list_node* head)
{
    bool enough_letters = true;
    
    // checks if there's a substring (only true of radix tree nodes)
    if(trie->substring != NULL)
    {
        // takes out the letters of the substring at that node from possible
        // letters that the user can use
        for (int j = 0; trie->substring[j] != '\0'; j++)
        {
            int index = trie->substring[j] - 'a';
            letters[index]--;
        }
        
        // make sure we had enough letters to spell out the substring
        
        for (int i = 0; i < ALPH_SIZE; i++)
        {
            if (letters[i] < 0)
                enough_letters = false;
        }
    }
    
    // there is a word to store at this level of the trie, so store it
    if (enough_letters && trie->stored_word != NULL 
        && strlen(trie->stored_word) != 0)
    {
        head = list_insert(trie->stored_word, head);
    }
    
    // makes sure there are enough letters to further traverse the trie/tree
    if (enough_letters == true)
    {
        // iterates through all of the user's possible letters and all of the 
        // current trie node's pointers simultaneously 
        for (int i = 0; i < ALPH_SIZE; i++)
        {
            // if the user has the requisite letters and there are trie pointers to 
            // traverse associated with those letters, the trie is traversed with
            // the letter used at that pointer removed (and then replaced)
            if (letters[i] > 0 && trie->children[i] != NULL)
            {
                letters[i]--;
                head = find_words(letters, trie->children[i], head);
                letters[i]++; 
            
            }
        }
    
    }
    
    // adds back in the letters for parental node usage
    if(trie->substring != NULL)
    {
        for (int j = 0; trie->substring[j] != '\0'; j++)
        {
            int index = trie->substring[j] - 'a';
            letters[index]++;
        }
    }
    
    return head;

}

// Finds the score for a word:

// By Letterpress rules, taking a letter from the opponent (any letter from
// list1) is worth 2 points, because it adds one to your score and subtracts 1 
// from the opponent. Using unclaimed letters (letters in list2) is worth 1 point 
int score_word(char* word, char* list1, char* list2)
{
    int score = 0;
    // sets of 26 "buckets" for letters -- tracks the number available of each letter
    int list1_counts[ALPH_SIZE] = {0};
    int list2_counts[ALPH_SIZE] = {0};
    // Fill the "buckets" -- for each letter, increment bucket value by 1.
    for(int i = 0, length = strlen(list1); i < length; i++)
    {
        list1_counts[list1[i] - 'a']++;
    }
    for(int i = 0, length = strlen(list2); i < length; i++)
    {
        list2_counts[list2[i] - 'a']++;
    }
    // For each letter, check if it's in list1 if yes, add 2 to score and decrement that
    // letter's bucket. If not, check list2 and add 1 to score. Otherwise, the letter 
    // has no value.
    for(int i = 0, length = strlen(word); i < length; i++)
    {
        if(list1_counts[word[i] - 'a'] != 0)
        {
            score += 2;
            list1_counts[word[i] - 'a']--;
        }
        else if (list2_counts[word[i] - 'a'] != 0)
        {
            score += 1;
            list2_counts[word[i] - 'a']--;
        }
    }
    
    return score;
}

// Makes sure a string is just letters.
bool check_alpha(char* word)
{
    for(int i = 0, length = strlen(word); i < length; i++)
    {
        if(!isalpha(word[i]))
            return false;
    }
    
    return true;
}

// distills possibilities into a list of the RESULTS best words
queue find_finalists (list_node* head, queue q, char* string1, char* string2)
{
    list_node* crawler = head;
    
    // continues to increment until hitting RESULTS, at which point nodes' scores
    // are taken into account before inserting into list
    int count = 0;
    
    // number defining the lowest score value for something to get on the list
    int min_required = 0;
    
    while (crawler != NULL)
    {
        // scores a node
        crawler->score = score_word(crawler->stored_word, string1, string2);
        
        // fills up the initial queue with RESULTS nodes
        if (count < RESULTS)
        {
            q = enqueue(q, crawler);
            
            min_required = q.rear->score;
            
            count++;
        } 
        // adds node to queue if its score exceeds the minimum score of queue
        // removes the new lowest-score node
        else if (crawler->score > min_required)
        {
            //print_finalists(q.front);
            q = enqueue(q, crawler);
            q = dequeue(q);
            min_required = q.rear->score;
        }
        
        crawler = crawler->next;
    }

    print_finalists(q.front);
    return q;
}

// adds node to the finalist queue
queue enqueue (queue q, list_node* node)
{
    // allocates space for new node of queue
    list_node* new_node = calloc(1, sizeof(list_node));
    
    // sets score of that node
    new_node->score = node->score;
    
    // allocates space for its word
    new_node->stored_word = calloc(DICT_MAX_LENGTH, sizeof(char));
    
    // copies word of possibilities list node into queue list node
    strcpy(new_node->stored_word,node->stored_word);
    
    list_node* crawler1 = q.front;
    list_node* crawler2 = q.front;
    
    // inserts node into empty queue
    if (q.front == NULL)
    {
        q.front = new_node;
        q.rear = new_node;

        return q;
    }
    
    // special case when new node is the new maximum (or equal to it)
    if (node->score >= q.front->score)
    {
        new_node->next = q.front;
        q.front = new_node;
        return q;
    }
    
    // crawls through queue
    while (crawler1 != NULL)
    {
        // puts node into its correct place (ordered by score)
        if (node->score >= crawler1->score)
        {
            new_node->next = crawler1;
            crawler2->next = new_node;
            return q;
        }
        
        crawler2 = crawler1;
        crawler1 = crawler1->next;
    }
    
    // adds node if it's last (can happen during initial filling of queue)
    crawler2->next = new_node;
    q.rear = new_node;
    
    return q;
}

// removes node from queue (QDQQQ!)
queue dequeue (queue q)
{
    list_node* crawler = q.front;
    
    while (crawler != NULL)
    {
        // pops off rear of queue
        if (crawler->next == q.rear)
        {
            crawler->next = NULL;
            if (q.rear != NULL)
            {
                free(q.rear->stored_word);
                free(q.rear);
            }
            
            q.rear = crawler;
            
            return q;
        }
        
        crawler = crawler->next;
    }
    
    return q;
}

// prints finalist queue
void print_finalists (list_node* front)
{
    list_node* crawler = front;
    int number = 1;
    
    while (crawler != NULL)
    {
        printf("%d. %s - %d\n", number, crawler->stored_word, crawler->score);
        crawler = crawler->next;
        number++;
    }
}

/**
 * Returns number of seconds between b and a.
 */
double calculate(const struct rusage* b, const struct rusage* a)
{
    if (b == NULL || a == NULL)
    {
        return 0.0;
    }
    else
    {
        return ((((a->ru_utime.tv_sec * 1000000 + a->ru_utime.tv_usec) -
                 (b->ru_utime.tv_sec * 1000000 + b->ru_utime.tv_usec)) +
                ((a->ru_stime.tv_sec * 1000000 + a->ru_stime.tv_usec) -
                 (b->ru_stime.tv_sec * 1000000 + b->ru_stime.tv_usec)))
                / 1000000.0);
    }
}
