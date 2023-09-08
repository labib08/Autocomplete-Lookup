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
#define COUNT 10

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
void changeBranchPrefixBit(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexIncoming);
void changePrefixBit(radix_t *incoming, int *indexIncoming);
int decideBranch(radix_t *incoming, int indexSame);
void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame);
int bitToInt(char bit);
void insertNode(radix_t *parent, radix_t *child, radix_t *branchNode);
void print2DUtil(radix_t* root, int space);
void print2D(radix_t* root);
void cleanupRadixTree(radix_t *root);

int main(int argc, char *argv[]){
    FILE *inFile = fopen(argv[2], "r");
    assert(inFile);
    root_t *rootNode = getRoot(inFile);
    //printPreorder(rootNode -> head);
    //print2D(rootNode -> head);
    //printf("%p", rootNode->head);
    //printf("Integer == %d\nPrefix == %s\nBit_Prefix == %s\nbranchA == %p\nbranchB == %p\ncafe == %p\n", rootNode->head->integer, rootNode->head->prefix, rootNode->head->bit_prefix, rootNode->head->branchA, rootNode->head->branchB, rootNode->head->data);
    fclose(inFile);  // Close the input file after reading data

    // Clean up memory
    cleanupRadixTree(rootNode->head);
    free(rootNode);
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
    //radix_t *temp = rootNode -> head;
    //radix_t *childTemp = NULL;

    if (rootNode->head == NULL){
        rootNode -> head = radixNode;
        rootNode -> parent = rootNode -> head;
    }
    else{
        int indexSame = 0;
        insertRecursively(&(rootNode->head), radixNode, indexSame);
    }
}

void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame) {
    radix_t *temp = *head;

    //printf("Different Branch: %s = %s and %s= %s\n", radixNode->prefix, radixNode->bit_prefix, temp->prefix, temp->bit_prefix);
    int isSame = compareBitPrefix(radixNode->bit_prefix, temp->bit_prefix, radixNode->integer, temp->integer,
                                  &indexSame);
    //printf("%d\n", isSame);
    //printf("%d\n", indexSame);
    //printf("Different Branch: %s = %s and %s= %s\n", radixNode->prefix, radixNode->bit_prefix, temp->prefix, temp->bit_prefix);
    if (!isSame) {
        radix_t *branchNode = createBranchNode();
        //printf("Different Branch: %s = %s and %s= %s\n", radixNode->prefix, radixNode->bit_prefix, temp->prefix, temp->bit_prefix);
        changeBranchPrefixBit(radixNode, temp, branchNode, indexSame);
        //printf("Different Branch: %s = %s and %s= %s\n", radixNode->prefix, radixNode->bit_prefix, temp->prefix, temp->bit_prefix);
        //printf("Different Branch: %s = %s and %s= %s\n", radixNode->prefix, radixNode->bit_prefix, temp->prefix, temp->bit_prefix);
        //printf("%d\n", indexSame);
        //printf("BranchNode = %s\n", branchNode->bit_prefix);
        if (temp -> parentBranch == NULL){
            *head = branchNode;
        }
        else{
            //printf("%d\n", temp -> parentBranch->branchB == temp);
            insertNode(temp -> parentBranch, temp, branchNode);

        }
        adjustNodeBranch(radixNode, temp, branchNode, indexSame);
        //printf("BranchNode = %s\n", branchNode->bit_prefix);
    } else {
        //printf("index1 == %d\n", indexSame);
        //printf("Same Branch: %s and \n", radixNode->bit_prefix);
        //printf("Same Branch: %s and %s\n", radixNode->prefix, radixNode->bit_prefix);
        changePrefixBit(radixNode, &indexSame);
        //printf("index2 == %d\n", indexSame);
        //printf("--------\n");
       //printf("Same Branch: %s and %s\n", radixNode->prefix, radixNode->bit_prefix);
        if (decideBranch(radixNode, indexSame) == RIGHT) {
            //printf("%s\n", radixNode->bit_prefix);
            insertRecursively(&((*head)->branchB), radixNode, indexSame);

        }
         else if (decideBranch(radixNode, indexSame) == LEFT){
            //printf("%s\n", radixNode->prefix);
            insertRecursively(&((*head)->branchA), radixNode, indexSame);
        }
    }
}

void insertNode(radix_t *parent, radix_t *child, radix_t *branchNode){
    if (parent -> branchA == child) {
        parent -> branchA = branchNode;
    }
    else if (parent -> branchB == child) {
        parent -> branchB = branchNode;
    }
    branchNode -> parentBranch = parent;
}

int decideBranch(radix_t *incoming, int indexSame) {
    int bitValue = bitToInt(incoming->bit_prefix[indexSame]);
    //printf("%d\n", indexSame);
    if (bitValue == RIGHT){
        return RIGHT;
    }
    return LEFT;
}

void adjustNodeBranch(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexSame) {
    assert(incoming && existing &&branchNode);
    int bitValue = bitToInt(incoming->bit_prefix[indexSame]);
    if (bitValue == LEFT){

        branchNode -> branchA = incoming;
        branchNode -> branchB = existing;

    }
    else if (bitValue == RIGHT){
        branchNode -> branchA = existing;
        branchNode -> branchB = incoming;
    }
    incoming -> parentBranch = existing -> parentBranch = branchNode;
}

void changePrefixBit(radix_t *incoming, int *indexIncoming){
    assert(incoming);
    int n = *indexIncoming;

   // printf("%d == %s\n", *indexIncoming, incoming->prefix);
    for (int i=0; i< n; i++){
        incoming -> bit_prefix[i] = PADDING;
        *indexIncoming = i + 1;
    }
}

void changeBranchPrefixBit(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexIncoming) {
    assert(incoming && existing &&branchNode);
    int n = 0;
    char *incomingBit = incoming -> bit_prefix;
    char *existingBit = existing -> bit_prefix;

    int index1 = strlen(incoming -> bit_prefix);
    int index2 = strlen(existing -> bit_prefix);

    int totalIndex = index1;
    //printf("strlen == %lu", strlen(incoming->bit_prefix));
    //printf("TotalIndex == %d\n", totalIndex);
    //printf("Index Incoming == %d\n", indexIncoming);
    if (index2< index1){
        totalIndex = index2;
    }

    char *binaryString = (char*)malloc(totalIndex);
    //printf("TotalIndex == %d\n", totalIndex);
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
    //printf("%d\n", n);
    branchNode -> bit_prefix = binaryString;
}

int compareBitPrefix(char *incomingBit, char *existingBit, int incomingInt, int existingInt, int *indexIncoming){
    int flag = 1;
    int i = 0;
    int n = incomingInt;
    if (existingInt < incomingInt){
        n = existingInt;
    }
    if (*indexIncoming < 0 || *indexIncoming >= n) {
        // Handle the out-of-bounds case, e.g., return an error code
        return -1;
    }
    //printf("indexIncoming == %d AND n == %d\n", *indexIncoming, *indexIncoming + n);
    for (i = *indexIncoming; i < n; i++){
        if (existingBit[i] != incomingBit[i]){
            flag = 0;

            break;
        }
    }
    *indexIncoming = i;
   // printf("%d == %s\n", *indexIncoming, incomingBit);
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

int bitToInt(char bit) {
    return atoi(&bit); // Convert the character bit to an integer
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

void print2DUtil(radix_t* root, int space)
{
    // Base case
    if (root == NULL)
        return;

    // Increase distance between levels
    space += COUNT;

    // Process right child first
    print2DUtil(root->branchB, space);

    // Print current node after space
    // count
    printf("\n");
    for (int i = COUNT; i < space; i++){
        printf(" ");
    }

    printf("%s\n", root->prefix);

    // Process left child
    print2DUtil(root->branchA, space);
}

void print2D(radix_t* root)
{
    // Pass initial space count as 0
    print2DUtil(root, 0);
}
void cleanupRadixTree(radix_t *root) {
    if (root == NULL) {
        return;
    }

    // Recursively free left and right subtrees
    cleanupRadixTree(root->branchA);
    cleanupRadixTree(root->branchB);

    // Free dynamically allocated fields
    free(root->prefix);
    free(root->bit_prefix);
    free(root);

    // Don't forget to free cafe data if it's dynamically allocated
    if (root->data) {
        free(root->data->build_add);
        free(root->data->clue_small_area);
        free(root->data->bus_area);
        free(root->data->trad_name);
        free(root->data->indus_desc);
        free(root->data->seat_type);
        free(root->data);
    }
}