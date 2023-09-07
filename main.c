#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define INIT_SIZE 2     // Initial size of arrays
#define MAX_CHAR_LEN 128
#define BYTE 8
#define PADDING 'x'
#define LEFT 0
#define RIGHT 1

typedef struct cafe cafe_t;
struct cafe {
    int cen_year;
    int block_id;
    int prop_id;
    int base_prop_id;
    char *build_add;
    char *clue_small_area;
    char *bus_area;
    char *trad_name;
    int indus_code;
    char *indus_desc;
    char *seat_type;
    int num_of_seats;
    double longitude;
    double latitude;
};

typedef struct radix radix_t;
struct radix {
    int integer;
    char *prefix;
    char *bit_prefix;
    radix_t *branchA;
    radix_t *branchB;
    radix_t *parentBranch;
    cafe_t *data;
};

typedef struct root root_t;
struct root {
    radix_t *head;
    radix_t *parent;
};

root_t *getRoot(FILE *f);
root_t *rootCreate();
void cafeSkipHeaderLine(FILE *f);
void cafeRead(FILE *f, root_t *rootNode);
cafe_t *createNode();
void putInNode(cafe_t *new_node, char *lines, int wordCount);
radix_t *createRadixNode(cafe_t *cafe, radix_t *radixNode);
int getBit(char *trad_name, char *bitArray, int n);
char *getBinary(char c);
void insert(root_t *rootNode, radix_t *radixNode);
int compareBitPrefix(char *incomingBit, char *existingBit, int incomingInt, int existingInt, int *indexIncoming);
radix_t *createBranchNode();
void adjustNodeBranch(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexSame);
void printPreorder(radix_t *root);
void changePrefixBit(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexIncoming);
int decideBranch(radix_t *incoming, int indexSame);
void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame);

int main(int argc, char *argv[]){
    FILE *inFile = fopen(argv[2], "r");
    assert(inFile);
    root_t *rootNode = getRoot(inFile);
    printPreorder(rootNode -> head);
    //printf("%p", rootNode->head);
    //printf("Integer == %d\nPrefix == %s\nBit_Prefix == %s\nbranchA == %p\nbranchB == %p\ncafe == %p\n", rootNode->head->integer, rootNode->head->prefix, rootNode->head->bit_prefix, rootNode->head->branchA, rootNode->head->branchB, rootNode->head->data);
    return 0;
}

root_t *getRoot(FILE *f){
    root_t *rootNode = rootCreate();
    cafeSkipHeaderLine(f);
    cafeRead(f, rootNode);
    return rootNode;
}

root_t *rootCreate(){
    root_t *rootNode = malloc(sizeof(*rootNode));
    assert(rootNode);
    rootNode -> head = NULL;
    rootNode -> parent = NULL;
    return rootNode;
}

void cafeSkipHeaderLine(FILE *f) {
    while (fgetc(f) != '\n');
}

void cafeRead(FILE *f, root_t *rootNode) {
    // Initialize the counters and lists needed to store each field
    char *lines = NULL;
    size_t len = 0;
    ssize_t read;
    cafe_t *new_node = NULL;
    int indicator = 1, wordCount = 0, i = 0, char_index = 0, line_end = 1;
    char line[MAX_CHAR_LEN + 1] = {0};
    char c;
    radix_t *radixNode;
    // Reads through every line until it reaches EOF
    while ((read = getline(&lines, &len, f)) != -1){
        new_node = createNode();
        char_index = 0;
        indicator = 1;
        wordCount = 0;
        line_end = 1;
        i = 0;
        // Read each character of each line then embeds it in the
        // node that is linked to the list
        while (line_end && (c = lines[i]) != '\0'){
            assert(wordCount < 15);
            // to accommodate for the qoutation marks used in csv file
            if (c == '"'){
                indicator = !indicator;
            }
            else if ((c == ',' || c == '\n' || c == EOF) && indicator){
                if (c == '\n') {
                    line_end = 0;
                }
                wordCount++;
                line[char_index] = '\0';
                // Adds the word to a node variable
                // as a string depending upon the word_count
                putInNode(new_node, line, wordCount);
                char_index = 0;
            }
            else {
                line[char_index] = c;
                char_index++;
            }
            i++;
        }
        // Appends the filled in node to the linked list
        radixNode = createRadixNode(new_node, radixNode);
        //printf("%s\n", radixNode -> data -> trad_name);
         insert(rootNode, radixNode);
    }
    // Frees the line pointer
    free(lines);
}

void insert(root_t *rootNode, radix_t *radixNode){
    radix_t *temp = rootNode -> head;
    //radix_t *childTemp = NULL;

    if (rootNode->head == NULL){
        rootNode -> head = radixNode;
        rootNode -> parent = rootNode -> head;
    }
    else{
        int indexSame, indexStart = 0;
        insertRecursively(&(rootNode->head), radixNode, indexSame);
        //printf("%s\n", temp -> bit_prefix);
        /**
        int indexSame = 0;

        int isSame = compareBitPrefix(radixNode -> bit_prefix, temp -> bit_prefix, radixNode -> integer, temp -> integer,
                                        &indexSame);


        if (!isSame) {
            radix_t *branchNode = createBranchNode();
            changePrefixBit(radixNode, temp, branchNode, indexSame);
            //printf("%s\n", radixNode -> bit_prefix);

            adjustNodeBranch(radixNode, temp, branchNode, indexSame);

            if (rootNode -> parent == rootNode -> head) {
                rootNode -> head = branchNode;
            }
            //printf("Integer == %d\nPrefix == %p\nBit_Preix == %s\nbranchA == %p\nbranchB == %p\ncafe == %p\n", branchNode->integer, branchNode->prefix, branchNode->bit_prefix, branchNode->branchA, branchNode->branchB, branchNode->data);
        }
        else{
            if (decideBranch(radixNode, isSame)){

            }
        }
        */
    }
    //printf("Integer == %d\nPrefix == %s\nBit_Preix == %s\nbranchA == %p\nbranchB == %p\ncafe == %s\n", radixNode->integer, radixNode->prefix, radixNode->bit_prefix, radixNode->branchA, radixNode->branchB, radixNode->data->trad_name);
    //printf("************\n");

}

void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame) {
    radix_t *temp = *head;


    int isSame = compareBitPrefix(radixNode->bit_prefix, temp->bit_prefix, radixNode->integer, temp->integer,
                                  &indexSame);

    if (!isSame) {
        radix_t *branchNode = createBranchNode();
        changePrefixBit(radixNode, temp, branchNode, indexSame);
        if (temp -> parentBranch == NULL){
            *head = branchNode;
        }
        else{
            branchNode = temp -> parentBranch;
        }
        adjustNodeBranch(radixNode, temp, branchNode, indexSame);

        /**
        if (temp == *head) {
            *head = branchNode; // Update the head pointer
        }
        else{

        }
        */
    } else {
        if (decideBranch(radixNode, isSame)) {

        }
    }
}
int decideBranch(radix_t *incoming, int indexSame) {
    if (incoming -> bit_prefix[indexSame] == RIGHT){
        return RIGHT;
    }
    return LEFT;
}

void adjustNodeBranch(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexSame) {

    if (incoming -> bit_prefix [indexSame] == LEFT){
        branchNode -> branchA = incoming;
        branchNode -> branchB = existing;
    }
    else {
        branchNode -> branchA = existing;
        branchNode -> branchB = incoming;
    }
    incoming -> parentBranch = existing -> parentBranch = branchNode;
}

void changePrefixBit(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexIncoming) {
    int n = 0;

    char *incomingBit = incoming -> bit_prefix;
    char *existingBit = existing -> bit_prefix;

    int totalIndex = incoming -> integer;

    if (existing -> integer < incoming -> integer){
        totalIndex = existing -> integer;
    }

    char *binaryString = (char*)malloc(totalIndex);

    while (n != indexIncoming){
        binaryString[n] = incomingBit[n];
        incomingBit[n] = PADDING;
        existingBit[n] = PADDING;
        n++;
    }

    branchNode -> integer = n;
    incoming -> integer -= n;
    existing -> integer -= n;

    while (n != totalIndex){
        binaryString[n] = PADDING;
        n++;
    }

    branchNode -> bit_prefix = binaryString;

}

int compareBitPrefix(char *incomingBit, char *existingBit, int incomingInt, int existingInt, int *indexIncoming){
    int flag = 1;

    int n = incomingInt;
    if (existingInt < incomingInt){
        n = existingInt;
    }

    for (int i = *indexIncoming; i < n; i++){
        if (existingBit[i] != incomingBit[i]){
            flag = 0;
            *indexIncoming = i;
            break;
        }
    }

    return flag;
}

radix_t *createRadixNode(cafe_t *cafe, radix_t *radixNode) {
    int prefix_length = strlen(cafe->trad_name) + 1;
    char *bitArray = (char *)malloc((BYTE * prefix_length + 2) * sizeof(char));
    int n = getBit(cafe->trad_name, bitArray, prefix_length);
    radixNode = malloc(sizeof(*radixNode));
    radixNode -> integer = n;
    radixNode -> prefix = strdup(cafe -> trad_name);
    radixNode -> bit_prefix = strdup(bitArray);
    radixNode -> branchA = NULL;
    radixNode -> branchB = NULL;
    radixNode -> parentBranch = NULL;
    radixNode -> data = cafe;
    return radixNode;
}

radix_t *createBranchNode() {
    radix_t *radixNode = (radix_t *) malloc(sizeof(radix_t));
    assert(radixNode);
    radixNode -> prefix = NULL;
    radixNode -> bit_prefix = NULL;
    radixNode -> branchA = NULL;
    radixNode -> branchB = NULL;
    radixNode -> parentBranch = NULL;
    radixNode -> data = NULL;
    return radixNode;
}

int getBit(char *trad_name, char *bitArray, int n) {
    //strcpy(bitArray, "0b");
    int num = 0;
    for (int i=0; i< n; i++){
        strcat(bitArray, getBinary(trad_name[i]));
        num ++;
    }
    return num * BYTE;
}

char *getBinary(char c){
    char *binaryString = (char*)malloc(BYTE);
    for (int i = 7; i >= 0; i--) {
        int bit = (c >> i) & 1;
        char bitChar = bit + '0'; // Convert bit to character '0' or '1'
        strncat(binaryString, &bitChar, 1); // Append the bit character to the string
    }

    return binaryString;
}

void putInNode(cafe_t *new_node, char *lines, int wordCount){
    // Which field the word added to depends on
    // where it is found in a line
    //(recoreded by word_count variable)
    if (wordCount == 1){
        // atoi converts the string to integer
        new_node->cen_year = atoi(lines);
    }
    else if (wordCount == 2){
        new_node->block_id = atoi(lines);
    }
    else if (wordCount == 3){
        new_node->prop_id = atoi(lines);
    }
    else if (wordCount == 4){
        new_node->base_prop_id = atoi(lines);
    }
    else if (wordCount == 5){
        new_node->build_add = strdup(lines);
    }
    else if (wordCount == 6){
        new_node->clue_small_area = strdup(lines);
    }
    else if (wordCount == 7){
        new_node->bus_area = strdup(lines);
    }
    else if (wordCount == 8){
        new_node->trad_name = strdup(lines);
    }
    else if (wordCount == 9){
        new_node->indus_code = atoi(lines);
    }
    else if (wordCount == 10){
        new_node->indus_desc = strdup(lines);
    }
    else if (wordCount == 11){
        new_node->seat_type = strdup(lines);
    }
    else if (wordCount == 12){
        new_node->num_of_seats = atoi(lines);
    }
    else if (wordCount == 13){
        // atof converts the string to double
        new_node->longitude = atof(lines);
    }
    else{
        new_node->latitude = atof(lines);
    }
}

cafe_t *createNode(){
    // Initialize the node and the sets the parameters to NULL
    cafe_t *node = (cafe_t *)malloc(sizeof(cafe_t));
    assert(node);
    node->build_add = NULL;
    node->clue_small_area = NULL;
    node->bus_area = NULL;
    node->trad_name = NULL;
    node->indus_desc = NULL;
    node->seat_type = NULL;
    return node;
}

void printPreorder(radix_t *root) {
    if (root == NULL)
        return;

    // First print data of node
    printf("Integer == %d\nPrefix == %s\nBit_Prefix == %s\nbranchA == %p\nbranchB == %p\nparentBranch == %p\ncafe == %p\n", root->integer, root->prefix, root->bit_prefix, root->branchA, root->branchB, root ->parentBranch, root->data);
    printf("********\n");
    // Then recur on left subtree
    printPreorder(root->branchA);

    // Now recur on right subtree
    printPreorder(root->branchB);
}