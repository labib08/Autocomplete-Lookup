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
    cafe_t * next;                              // Pointer to the next node
};

typedef struct radix radix_t;
struct radix {
    int integer;
    char *prefix;
    char *bit_prefix;
    radix_t *branchA;                           // Pointer to the left branch of the child node
    radix_t *branchB;                           // Pointer to the right branch of the child node
    radix_t *parentBranch;                      // Pointer to the parent branch
    cafe_t *data;                               // Pointer to the data of the corresponding cafe
};

typedef struct root root_t;
struct root {
    radix_t *head;                              // Pointer to the head of the radix tree node
};

root_t *getRoot(FILE *f);
root_t *rootCreate();
void cafeSkipHeaderLine(FILE *f);
void cafeRead(FILE *f, root_t *rootNode);
cafe_t *createNode();
void putInNode(cafe_t *new_node, char *lines, int wordCount);
void createRadixNode(cafe_t *cafe, radix_t *radixNode);
void getBit(char *trad_name, char *bitArray, int n);
char *getBinary(char c);
void insert(root_t *rootNode, radix_t *radixNode);
int compareBitPrefix(char *incomingBit, char *existingBit, int *indexIncoming);
radix_t *createBranchNode();
void adjustNodeBranch(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexSame);
void changeBranchPrefixBit(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexIncoming);
void changePrefixBit(char *incomingBit, int *indexIncoming);
int decideBranch(char *incomingBit, int indexSame);
void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame);
int bitToInt(char bit);
void insertNode(radix_t *parent, radix_t *child, radix_t *branchNode);
void cleanupRadixTree(radix_t *root);
int findIndex(char* bitPrefix, int indexSame, int n);
int findInteger(char *bitPrefix);
char **returnQueries(char **queries, int *queryCount, int *initialQuerySize);
void freeQueries(char **queries, int queryCount);
int esnureQuerySize(int queryCount, int initialQuerySize);
void findAndTraverse(radix_t *root, char *query, char *queryBit, int prefixLength, int indexSame,
                     int charComp, int byteComp, FILE *f);
int checkMatch(char *prefix, char *query, char *queryBit, int n);
int findPadding(char *bitPrefix);
void printOutFile(FILE *f, cafe_t *cafe);
void storeDuplicates(cafe_t *existingData, cafe_t *incomingData);

int main(int argc, char *argv[]){
    // Check to have at least 3 arguments in command line
    if (argc < 3){
        fprintf(stderr, "Usage: %s input_file_name output_file_name\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Open input and output files
    FILE *inFile = fopen(argv[2], "r");
    FILE *outFile = fopen(argv[3], "w");
    assert(inFile && outFile);
    root_t *rootNode = getRoot(inFile);
    int queryCount = 0;
    char **queries;
    int initialQuerySize = INIT_SIZE;
    queries = (char **)malloc(sizeof(char *) * initialQuerySize);
    assert(queries);

    // Returns the cafes, as an array, that are intended to be searched in the list
    queries = returnQueries(queries, &queryCount, &initialQuerySize);
    if (queryCount){
        for (int i = 0; i < queryCount; i++) {
            int prefixLength = strlen(queries[i]) + 1;
            // Allocates enough memory for bits and null terminator
            char *queryBit = (char *)malloc((BYTE * prefixLength) + 1);
            assert(queryBit);
            // Converts the query input into bits
            getBit(queries[i], queryBit, prefixLength);
            int indexSame = 0;
            int charComp = 0;
            int byteComp = 0;
            findAndTraverse(rootNode -> head, queries[i], queryBit, prefixLength, indexSame, charComp, byteComp, outFile);
            free(queryBit);
        }
    }

    fclose(inFile);
    fclose(outFile);
    // Clean up memory
    cleanupRadixTree(rootNode->head);
    free(rootNode);
    freeQueries(queries, queryCount);
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
    radix_t *radixNode = NULL;
    int indicator = 1, wordCount = 0, i = 0, char_index = 0, line_end = 1;
    char line[MAX_CHAR_LEN + 1] = {0};
    char c;
    // Reads through every line until it reaches EOF
    while ((read = getline(&lines, &len, f)) != -1){
        new_node = createNode();
        char_index = 0;
        indicator = 1;
        wordCount = 0;
        line_end = 1;
        i = 0;
        // Read each character of each line then embeds it in the
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
        radixNode = createBranchNode();
        // The newly created node of the radix tree is filled in
        createRadixNode(new_node, radixNode);
        // Then it is pushed down the radix tree
        insert(rootNode, radixNode);

    }
    // Frees the line pointer
    free(lines);
}

void insert(root_t *rootNode, radix_t *radixNode){
    // The node becomes the root node if the tree is empty
    // Else it is pushed down the tree using a recursive function
    if (rootNode->head == NULL){
        rootNode -> head = radixNode;
    }
    else{
        int indexSame = 0;
        insertRecursively(&(rootNode->head), radixNode, indexSame);
    }
}

void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame) {
    radix_t *temp = *head;
    // The bits of the incoming node and the current existing node is compared
    int isSame = compareBitPrefix(radixNode->bit_prefix, temp->bit_prefix, &indexSame);
    // If they differ then a new branch is created
    if (!isSame) {
        radix_t *branchNode = createBranchNode();
        // The bits of the current, incoming and newly created branchnode is updated accordingly
        changeBranchPrefixBit(radixNode, temp, branchNode, indexSame);
        // If the mismatch is found at the root node, then the newly created branch node becomes
        // the root node. Else the newly created node is connected to the current existing node
        if (temp -> parentBranch == NULL){
            *head = branchNode;
        }
        else{
            insertNode(temp -> parentBranch, temp, branchNode);
        }
        // The branches of the three nodes is adjusted accordingly
        adjustNodeBranch(radixNode, temp, branchNode, indexSame);

    } else {
        // If the bits of the incoming node and the current node that are being compared
        // are the same, then the bits of the incoming node is adjusted for comparison reasons
        changePrefixBit(radixNode -> bit_prefix, &indexSame);
        // The integer of the incoming node is updated
        radixNode -> integer = findInteger(radixNode -> bit_prefix);
        // If the prefix name of the incoming node and current node match,
        // that means they are duplicate, so it is added as a linked list in the
        // data of the current node
        if (temp -> prefix && !strcmp(radixNode -> prefix, temp -> prefix)) {
            storeDuplicates(temp -> data, radixNode -> data);
            // Then frees the radix node, since the node itself is of no use
            free(radixNode -> bit_prefix);
            free(radixNode -> prefix);
            radixNode->data = NULL;
            radixNode->prefix = NULL;
            radixNode->bit_prefix = NULL;
            radixNode -> branchA = NULL;
            radixNode -> branchB = NULL;
            free(radixNode);
        }
         // Decides whether the left part of the tree should be traversed next or the right part
         // and the whole comparison process is starts over again
         else if (decideBranch(radixNode -> bit_prefix, indexSame) == RIGHT) {
            insertRecursively(&((*head)->branchB), radixNode, indexSame);

        }
         else if (decideBranch(radixNode -> bit_prefix, indexSame) == LEFT){
            insertRecursively(&((*head)->branchA), radixNode, indexSame);
        }
    }
}

void storeDuplicates(cafe_t *existingData, cafe_t *incomingData){
    cafe_t *curr = existingData;
    // The duplicate is stored at the end of the linked list
    while (curr -> next){
        curr = curr -> next;
    }
    curr -> next = incomingData;
}

void findAndTraverse(radix_t *root, char *query, char *queryBit, int prefixLength, int indexSame,
                        int charComp, int byteComp, FILE *f) {
    int strComp = 1;
    if (root == NULL){
        return;
    }
    // The bits of the query and the existing root node to be compared are compared
    int isSame = compareBitPrefix(queryBit, root -> bit_prefix, &indexSame);
    // If they are the same check for further matches and increment the bit and char
    // comparison. Else traverse the tree
    if (isSame){
        changePrefixBit(queryBit, &indexSame);
        // If the prefix match, then a match is foynd
        if (root -> prefix && checkMatch(root -> prefix, query, queryBit, prefixLength)){
            // Increment the byte excluding the bits allocated for the null terminator
            byteComp += root -> integer - BYTE;
            // Increment the character count excluding the null terminator
            charComp += prefixLength - 1;
            // Subtract the if the one whole character was encountered while traversing
            charComp -= findPadding(root -> bit_prefix);
            printf("%s --> b%d c%d s%d\n", query, byteComp, charComp, strComp);
            fprintf(f, "%s\n", root -> prefix);
            printOutFile(f, root -> data);
            return;
        }
        else {
            byteComp += root -> integer;
            int num = ((root -> integer)/BYTE);

            if (root->integer % BYTE == 0){
                charComp += num ;
            }
            else{
                num += 1;
                charComp += num ;
            }
        }
        if (decideBranch(queryBit , indexSame) == RIGHT){
            findAndTraverse(root -> branchB, query, queryBit, prefixLength, indexSame, charComp, byteComp, f);
        }
        else if ((decideBranch(queryBit , indexSame) == LEFT)){
            findAndTraverse(root -> branchA, query, queryBit, prefixLength, indexSame, charComp, byteComp, f);
        }
    }
}

void printOutFile(FILE *f, cafe_t *cafe) {
    cafe_t *curr = cafe;
    while (curr){
        fprintf(f, "--> census_year: %d || block_id: %d || property_id: %d"
        "|| base_property_id: %d || building_address: %s || clue_small_area: %s ||"
        " business_address: %s || trading_name: %s || industry_code: %d ||"
        " industry_description: %s || seating_type: %s || number_of_seats: %d ||"
        " longitude: %.5lf || latitude: %.5lf ||\n", curr->cen_year,
        curr -> block_id, curr->prop_id, curr->base_prop_id,
        curr->build_add, curr->clue_small_area, curr->bus_area,
        curr->trad_name, curr->indus_code, curr->indus_desc,
        curr->seat_type, curr->num_of_seats, curr->longitude,
        curr->latitude);
        curr = curr -> next;
    }

}

int findPadding(char *bitPrefix) {
    int index = strlen(bitPrefix);
    int num = 0;
    for (int i = 0; i < index; i++){
        if (bitPrefix[i] == PADDING){
            num ++;
        }
    }
    return num/BYTE;
}

int findInteger(char *bitPrefix) {
    int index = strlen(bitPrefix);
    int integer = 0;
    for (int i = 0; i< index; i++){
        if (bitPrefix[i] != PADDING){
            integer ++;
        }
    }
    return integer;
}
int checkMatch(char *prefix, char *query, char *queryBit, int n) {
    int flag = 1;
    for (int i =0; i < n; i++) {
        if (queryBit[i] != PADDING) {
            flag = 0;
            break;
        }
    }
    return !strcmp(prefix, query) && flag;
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

int decideBranch(char *incomingBit, int indexSame) {
    int bitValue = bitToInt(incomingBit[indexSame]);
    if (bitValue == RIGHT){
        return RIGHT;
    }
    return LEFT;
}

void adjustNodeBranch(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexSame) {
    assert(incoming && existing &&branchNode);
    int bitValue = bitToInt(incoming->bit_prefix[indexSame]);
    // If the next bit after the padding is 0, then the incoming radix node goes to the left
    // branch of the newly created branch node, otherwise goes to the right
    if (bitValue == LEFT){
        branchNode -> branchA = incoming;
        branchNode -> branchB = existing;
    }
    else if (bitValue == RIGHT){
        branchNode -> branchA = existing;
        branchNode -> branchB = incoming;
    }
    // The parent branch of the incoming and existing node is set to the newly created branch node
    incoming -> parentBranch = existing -> parentBranch = branchNode;
}

void changePrefixBit(char *incomingBit, int *indexIncoming){
    int n = *indexIncoming;
    for (int i=0; i< n; i++){
        incomingBit[i] = PADDING;
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

    if (index2< index1){
        totalIndex = index2;
    }
    // The memory for the bit of the newly created branch node is allocated
    char *binaryString = (char*)malloc(totalIndex + 1);
    // The bits that were used in comparison are changed
    while (n != indexIncoming){
        binaryString[n] = incomingBit[n];
        incomingBit[n] = PADDING;
        existingBit[n] = PADDING;
        n++;
    }
    // The rest of the bits are padded
    while (n != totalIndex){
        binaryString[n] = PADDING;
        n++;
    }

    binaryString[n] = '\0'; // Null-terminator is added to the string
    branchNode -> bit_prefix = binaryString;
    // The integer values of the nodes are updated accordingly
    branchNode -> integer = findInteger(branchNode -> bit_prefix);
    incoming -> integer = findInteger(incoming -> bit_prefix);
    existing -> integer = findInteger(existing -> bit_prefix);
}

int compareBitPrefix(char *incomingBit, char *existingBit, int *indexIncoming){
    int flag = 1;
    int i = 0;
    int totalIndex = strlen(existingBit);
    if (strlen(incomingBit) < totalIndex){
        totalIndex = strlen(incomingBit);
    }
    int incomingInd = findIndex(incomingBit, *indexIncoming, totalIndex);
    int existingInd = findIndex(existingBit, *indexIncoming, totalIndex);
    int n = incomingInd;
    if (existingInd < incomingInd){
        n = existingInd;
    }
    // The bits that are needed to be compared are compared
    for (i = *indexIncoming; i < n; i++){
        if (existingBit[i] != incomingBit[i]){
            flag = 0;
            break;
        }
    }
    // The index is updated accordingly
    *indexIncoming = i;
    return flag;
}

int findIndex(char* bitPrefix, int indexSame, int n) {
    int i =0;
    for ( i = indexSame; i<n; i++) {
        if (bitPrefix[i] == PADDING){
            break;
        }
    }
    return i;
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

void createRadixNode(cafe_t *cafe, radix_t *radixNode) {

    int prefix_length = strlen(cafe->trad_name) + 1;
    // Allocate enough memory for bits and null terminator
    char *bitArray = (char *)malloc((BYTE * prefix_length) + 1);
    assert(bitArray);
    // Converts the prefix name into bits
    getBit(cafe->trad_name, bitArray, prefix_length);
    assert(radixNode);
    radixNode->integer = findInteger(bitArray);
    radixNode->prefix = strdup(cafe->trad_name);
    radixNode->bit_prefix = bitArray;
    radixNode->branchA = NULL;
    radixNode->branchB = NULL;
    radixNode->parentBranch = NULL;
    radixNode->data = cafe;
}

void getBit(char *trad_name, char *bitArray, int n) {
    assert(bitArray);
    // Initializes the bit array
    bitArray[0] = '\0';
    for (int i = 0; i < n; i++) {
        // Gets the binary of each character and the concatenates into a string
        char *binary = getBinary(trad_name[i]);
        if (binary) {
            strncat(bitArray, binary, BYTE);
            free(binary);
        }
    }
}

char *getBinary(char c) {
    // Allocates enough memory for 8 bits and the null terminator
    char *binaryString = (char *)malloc(BYTE + 1);
    // Finds the bits of the character
    for (int i = 7; i >= 0; i--) {
        int bit = (c >> i) & 1;
        // Store the bits at the correct position
        binaryString[7 - i] = bit + '0';
    }

    binaryString[BYTE] = '\0'; // Null-terminate the string
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

char **returnQueries(char **queries, int *queryCount, int *initialQuerySize){
    char query[MAX_CHAR_LEN + 1];
    assert(queries);
    int c;
    // Parses through the query file or stdin until a
    // newline or End of File is reached
    while (scanf("%[^\n]", query) == 1){
        while ((c = getchar()) != '\n' && c != EOF);
        // Stores the particular queries in an array
        queries[*queryCount] = strdup(query);
        (*queryCount)++;
        // If the number of elements in the query array reaches
        // the current size, it is going to allocate more memory
        // for the array
        if (esnureQuerySize(*queryCount, *initialQuerySize)){
            *initialQuerySize = *initialQuerySize * 2;
            queries = realloc(queries, *initialQuerySize * sizeof(char *));
            assert(queries);
        }
    }
    // Return the queries array
    return queries;
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
    node->next = NULL;
    return node;
}

int bitToInt(char bit) {
    return bit - '0';
}

int esnureQuerySize(int queryCount, int initialQuerySize) {
    if (queryCount == initialQuerySize){
        return 1;
    }
    return 0;
}

void cleanupRadixTree(radix_t *root) {
    if (root == NULL) {
        return;
    }
    // Free dynamically allocated fields in cafe data
    if (root->data) {
        cafe_t *curr = root -> data;
        // Frees the duplicates as well
        while (curr){
            cafe_t *temp = curr;
            curr = curr->next;
            free(temp->build_add);
            free(temp->clue_small_area);
            free(temp->bus_area);
            free(temp->trad_name);
            free(temp->indus_desc);
            free(temp->seat_type);
            free(temp);

        }
    }

    // Frees dynamically allocated fields in radix node
    free(root->prefix);
    free(root->bit_prefix);
    root->parentBranch = NULL;
    root->data = NULL;
    root->prefix = NULL;
    root->bit_prefix = NULL;
    // Recursively frees left and right subtrees
    cleanupRadixTree(root->branchA);
    cleanupRadixTree(root->branchB);
    root->branchA = NULL;
    root->branchB = NULL;

    // Finally, frees the current node
    free(root);
}

void freeQueries(char **queries, int queryCount){
    for (int i = 0; i < queryCount; i++){
        free(queries[i]);
    }
    free(queries);
}