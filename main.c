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
    cafe_t * next;
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
void createRadixNode(cafe_t *cafe, radix_t *radixNode);
void getBit(char *trad_name, char *bitArray, int n);
char *getBinary(char c);
void insert(root_t *rootNode, radix_t *radixNode);
int compareBitPrefix(char *incomingBit, char *existingBit, int *indexIncoming);
radix_t *createBranchNode();
void adjustNodeBranch(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexSame);
void printPreorder(radix_t *root);
void changeBranchPrefixBit(radix_t *incoming, radix_t *existing, radix_t *branchNode, int indexIncoming);
void changePrefixBit(char *incomingBit, int *indexIncoming);
int decideBranch(char *incomingBit, int indexSame);
void insertRecursively(radix_t **head, radix_t *radixNode, int indexSame);
int bitToInt(char bit);
void insertNode(radix_t *parent, radix_t *child, radix_t *branchNode);
void print2DUtil(radix_t* root, int space);
void print2D(radix_t* root);
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
    if (argc < 3){
        fprintf(stderr, "Usage: %s input_file_name output_file_name\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *inFile = fopen(argv[2], "r");
    FILE *outFile = fopen(argv[3], "w");
    assert(inFile && outFile);
    root_t *rootNode = getRoot(inFile);
    printPreorder(rootNode -> head);
    //print2D(rootNode -> head);
    //printf("%p", rootNode->head);
    //printf("Integer == %d\nPrefix == %s\nBit_Prefix == %s\nbranchA == %p\nbranchB == %p\ncafe == %p\n", rootNode->head->integer, rootNode->head->prefix, rootNode->head->bit_prefix, rootNode->head->branchA, rootNode->head->branchB, rootNode->head->data);
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
            char *queryBit = (char *)malloc((BYTE * prefixLength) + 1); // Allocate enough memory for bits and null terminator
            assert(queryBit);
            getBit(queries[i], queryBit, prefixLength);
            int indexSame = 0;
            int charComp = 0;
            int byteComp = 0;
            findAndTraverse(rootNode -> head, queries[i], queryBit, prefixLength, indexSame, charComp, byteComp, outFile);
            //printf("----------\n");
        }
    }

    fclose(inFile);  // Close the input file after reading data
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
    //radix_t *radixNode;
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
        radix_t *radixNode = createBranchNode();
        // Appends the filled in node to the linked list
        createRadixNode(new_node, radixNode);
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
    int isSame = compareBitPrefix(radixNode->bit_prefix, temp->bit_prefix, &indexSame);
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

        changePrefixBit(radixNode -> bit_prefix, &indexSame);
        radixNode -> integer = findInteger(radixNode -> bit_prefix);
        //printf("index2 == %d\n", indexSame);
        //printf("--------\n");
        //printf("Same Branch: %s and %s\n", radixNode->prefix, radixNode->bit_prefix);

        if (temp -> prefix && !strcmp(radixNode -> prefix, temp -> prefix)) {
            storeDuplicates(temp -> data, radixNode -> data);
        }

         //printf("%d\n", strcmp(radixNode -> prefix, temp -> prefix));
         else if (decideBranch(radixNode -> bit_prefix, indexSame) == RIGHT) {
            //printf("%s\n", radixNode->bit_prefix);
            insertRecursively(&((*head)->branchB), radixNode, indexSame);

        }
         else if (decideBranch(radixNode -> bit_prefix, indexSame) == LEFT){
            //printf("%s\n", radixNode->prefix);
            //printf("Same Branch: %s and %s\n", radixNode->prefix, radixNode->bit_prefix);
            insertRecursively(&((*head)->branchA), radixNode, indexSame);
        }
    }
}

void storeDuplicates(cafe_t *existingData, cafe_t *incomingData){
    cafe_t *curr = existingData;
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
    int isSame = compareBitPrefix(queryBit, root -> bit_prefix, &indexSame);
    if (isSame){
        changePrefixBit(queryBit, &indexSame);

        //printf("%d\n", (((root -> integer)/BYTE) + 1));

        //printf("%s == %s\n", query, queryBit);
        if (root -> prefix && checkMatch(root -> prefix, query, queryBit, prefixLength)){
            //printf("%s == %s == %s\n",root -> prefix, query, queryBit);
            //printf("Add = %d\n", prefixLength - 1);
            byteComp += root -> integer - BYTE;
            charComp += prefixLength - 1;
            //printf("Subtract = %d\n",findPadding(root -> bit_prefix) );
            charComp -= findPadding(root -> bit_prefix);
            //printf("%s --> b%d c%d s%d\n", query, byteComp, charComp, strComp);
            fprintf(f, "%s\n", root -> prefix);
            printOutFile(f, root -> data);
            return;
        }
        else {
            byteComp += root -> integer;
            int num = ((root -> integer)/BYTE);
            //printf("%d\n", root ->integer);
            //printf("Add = %d\n", (((root -> integer)/BYTE) + 1));
            if (root->integer % BYTE == 0){
                //printf("Add = %d\n", num);
                charComp += num ;
            }
            else{
                num += 1;
                //printf("Add = %d\n", num);
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

    //printf("%d\n", n);
    //printf("%s == %s\n", query, queryBit);
}

void printOutFile(FILE *f, cafe_t *cafe) {
    fprintf(f, "--> census_year: %d || block_id: %d || property_id: %d"
        "|| base_property_id: %d || building_address: %s || clue_small_area: %s ||"
        " business_address: %s || trading_name: %s || industry_code: %d ||"
        " industry_description: %s || seating_type: %s || number_of_seats: %d ||"
        " longitude: %.5lf || latitude: %.5lf ||\n", cafe->cen_year,
        cafe -> block_id, cafe->prop_id, cafe->base_prop_id,
        cafe->build_add, cafe->clue_small_area, cafe->bus_area,
        cafe->trad_name, cafe->indus_code, cafe->indus_desc,
        cafe->seat_type, cafe->num_of_seats, cafe->longitude,
        cafe->latitude);
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

void changePrefixBit(char *incomingBit, int *indexIncoming){

    int n = *indexIncoming;

   // printf("%d == %s\n", *indexIncoming, incoming->prefix);
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
    //printf("strlen == %lu", strlen(incoming->bit_prefix));
    //printf("TotalIndex == %d\n", totalIndex);
    //printf("Index Incoming == %d\n", indexIncoming);
    if (index2< index1){
        totalIndex = index2;
    }

    char *binaryString = (char*)malloc(totalIndex + 1);
    //printf("TotalIndex == %d\n", totalIndex);
    while (n != indexIncoming){
        binaryString[n] = incomingBit[n];
        incomingBit[n] = PADDING;
        existingBit[n] = PADDING;
        n++;
    }

    while (n != totalIndex){
        binaryString[n] = PADDING;
        n++;
    }
    //printf("%d\n", n);
    binaryString[n] = '\0'; // Null-terminate the string
    branchNode -> bit_prefix = binaryString;
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
    if (*indexIncoming < 0 || *indexIncoming >= n) {
        // Handle the out-of-bounds case, e.g., return an error code
        //printf("n === %d\n", n);
        //printf("isSame === %d\n", *indexIncoming);
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
    char *bitArray = (char *)malloc((BYTE * prefix_length) + 1); // Allocate enough memory for bits and null terminator
    assert(bitArray);
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
    bitArray[0] = '\0';
    for (int i = 0; i < n; i++) {
        char *binary = getBinary(trad_name[i]);
        if (binary) {
            strncat(bitArray, binary, BYTE);
            free(binary);
        }
    }
}

char *getBinary(char c) {
    char *binaryString = (char *)malloc(BYTE + 1); // Allocate enough memory for 8 bits + null terminator
    //char binaryString[BYTE + 1];
    //assert(binaryString);

    for (int i = 7; i >= 0; i--) {
        int bit = (c >> i) & 1;
        binaryString[7 - i] = bit + '0'; // Store the bit at the correct position
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
    if (bit == '0' || bit == '1') {
        return bit - '0'; // Convert '0' or '1' to 0 or 1
    } else {
        // Handle the case where the character is not '0' or '1'
        // You can add error handling or return a default value
        return -1; // Return an error code or default value as needed
    }
}

int esnureQuerySize(int queryCount, int initialQuerySize) {
    if (queryCount == initialQuerySize){
        return 1;
    }
    return 0;
}

void printPreorder(radix_t *root) {
    if (root == NULL)
        return;

    // First print data of node

    //printf("Integer == %d\nPrefix == %s\nBit_Prefix == %s\nbranchA == %p\nbranchB == %p\nparentBranch == %p\ncafe == %p\n", root->integer, root->prefix, root->bit_prefix, root->branchA, root->branchB, root ->parentBranch, root->data);
    if (root -> prefix && root -> data -> next){
        cafe_t *temp = root -> data;
        while (temp) {
            printf("%s == %s\n\n", temp->trad_name, temp->seat_type);
            temp = temp -> next;
        }
    }
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


    // Free dynamically allocated fields in cafe data
    if (root->data) {
        free(root->data->build_add);
        free(root->data->clue_small_area);
        free(root->data->bus_area);
        free(root->data->trad_name);
        free(root->data->indus_desc);
        free(root->data->seat_type);
        free(root->data);
    }

    // Free dynamically allocated fields in radix node
    free(root->prefix);
    free(root->bit_prefix);
    root->parentBranch = NULL;
    root->data = NULL;
    root->prefix = NULL;
    root->bit_prefix = NULL;
    cleanupRadixTree(root->branchA);
    cleanupRadixTree(root->branchB);
    root->branchA = NULL;
    root->branchB = NULL;



    // Finally, free the current node
    free(root);
}

void freeQueries(char **queries, int queryCount){
    for (int i = 0; i < queryCount; i++){
        free(queries[i]);
    }
    free(queries);
}
