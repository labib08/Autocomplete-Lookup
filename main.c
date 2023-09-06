#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ALPHABET_SIZE 26

// Trie node
struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isTerminal;
    int value;  // Data-Type (you can replace this with your specific data type)
};

// Returns a new trie node (initialized to NULLs)
struct TrieNode* createNode(void) {
    struct TrieNode* pNode = (struct TrieNode*)malloc(sizeof(struct TrieNode));

    pNode->isTerminal = false;
    pNode->value = 0;

    for (int i = 0; i < ALPHABET_SIZE; i++)
        pNode->children[i] = NULL;

    return pNode;
}

// Inserts key into the trie with associated value
void trieInsert(struct TrieNode* root, const char* key, int value) {
    struct TrieNode* x = root;

    for (int i = 0; key[i] != '\0'; i++) {
        int index = key[i] - 'a';
        if (!x->children[index]) {
            x->children[index] = createNode();
        }
        x = x->children[index];
    }

    x->value = value;
    x->isTerminal = true;
}

// Searches for a key in the trie and returns its value if found,
// or -1 if not found
int trieSearch(struct TrieNode* root, const char* key) {
    struct TrieNode* x = root;

    for (int i = 0; key[i] != '\0'; i++) {
        int index = key[i] - 'a';
        if (!x->children[index]) {
            return -1;  // Key not found
        }
        x = x->children[index];
    }

    if (x->isTerminal) {
        return x->value;
    } else {
        return -1;  // Key not found
    }
}

int main() {
    struct TrieNode* root = createNode();

    // Insert key-value pairs
    trieInsert(root, "apple", 42);
    trieInsert(root, "banana", 17);

    // Search for keys
    printf("Value of 'apple': %d\n", trieSearch(root, "apple"));     // Should print 42
    printf("Value of 'banana': %d\n", trieSearch(root, "banana"));   // Should print 17
    printf("Value of 'cherry': %d\n", trieSearch(root, "cherry"));   // Should print -1 (not found)

    // Free allocated memory (you should add this to your actual implementation)
    free(root);

    return 0;
}