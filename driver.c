/** If the query file is not included in the terminal,
    the program will as for query names, type each individual
    query cafe name and keep pressing enter. Once all the query
    names are added, press ^D to end. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define INIT_SIZE 2     // Initial size of arrays
#define MAX_CHAR_LEN 128
#define BYTE 8

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

typedef struct array array_t;
struct array {
	cafe_t **A;
	int size;
	int n;
};

// census_year,block_id,property_id,base_property_id,building_address,clue_small_area,business_address,trading_name,industry_code,industry_description,seating_type,number_of_seats,longitude,latitude
//int parseField(FILE *f, char *buffer, int bufferSize);
array_t *getCafeArray(FILE *f);
char **returnQueries(char **queries, int *queryCount, int *initialQuerySize);
void cafeSkipHeaderLine(FILE *f);
int esnureQuerySize(int queryCount, int initialQuerySize);
array_t *arrayCreate();
void cafeRead(FILE *f, array_t *arr);
cafe_t *createNode();
void putInNode(cafe_t *new_node, char *lines, int wordCount);
void arrayEnsureSize(array_t *arr);
int tradNameCmp(const void *a, const void *b);
int charCompFunc(char *trad_name, char *prefix, int prefix_length);
void arrayAppend(array_t *arr, cafe_t *c);
void findAndTraverse(array_t *cafes, char *prefix, FILE *f);
void printOutFile(FILE *f, array_t *cafes, int ind);
void freeCafes(array_t *arr);
void freeQueries(char **queries, int queryCount);

int main(int argc, char *argv[]){
    // Check to have at least 3 arguments in command line
	// open input and output files

    if (argc < 3){
        fprintf(stderr, "Usage: %s input_file_name output_file_name\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *inFile = fopen(argv[2], "r");
    FILE *outFile = fopen(argv[3], "w");
    assert(inFile && outFile);

    array_t *cafes = getCafeArray(inFile);
    //int j=0;
    for (int i=0; i< cafes->n; i++){
        //printf("%d, %d, %d, %d, %s, %s, %s, %s, %d, %s, %s, %d, %lf , %lf\n", cafes -> A[i] ->cen_year, cafes -> A[i] ->block_id, cafes -> A[i] ->prop_id, cafes -> A[i] ->base_prop_id, cafes -> A[i] ->build_add, cafes -> A[i] ->clue_small_area, cafes -> A[i] ->bus_area, cafes -> A[i] ->trad_name, cafes -> A[i] ->indus_code, cafes -> A[i] ->indus_desc, cafes -> A[i] ->seat_type, cafes -> A[i] ->num_of_seats, cafes -> A[i] ->longitude, cafes -> A[i] ->latitude);
        //printf("************\n");
        //printf("(%d) %s\n",j++, cafes->A[i]->trad_name);
    }
    //printf("************\n");
    int queryCount = 0;
    char **queries;
    int initialQuerySize = INIT_SIZE;
    queries = (char **)malloc(sizeof(char *) * initialQuerySize);
    assert(queries);

    // Returns the cafes, as an array, that are intended to be searched in the list
    queries = returnQueries(queries, &queryCount, &initialQuerySize);
    if (queryCount){
        for (int i = 0; i < queryCount; i++) {
            findAndTraverse(cafes, queries[i], outFile);
        }
    }

    freeCafes(cafes);
    // Frees the queries array
    freeQueries(queries, queryCount);
    fclose(inFile);
    return 0;

}

array_t *getCafeArray(FILE *f){
    array_t *cafes = arrayCreate();
	cafeSkipHeaderLine(f);
    cafeRead(f, cafes);
	return cafes;
}

void cafeSkipHeaderLine(FILE *f) {
	while (fgetc(f) != '\n');
}

array_t *arrayCreate() {

	array_t *arr = malloc(sizeof(*arr));
	int size = INIT_SIZE;
	arr->size = size;
	arr->A = malloc(size * sizeof(*(arr->A)));
	arr->n = 0;
	return arr;
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

void cafeRead(FILE *f, array_t *arr) {
    // Initialize the counters and lists needed to store each field
    char *lines = NULL;
    size_t len = 0;
    ssize_t read;
    cafe_t *new_node = NULL;
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
        arrayAppend(arr, new_node);
    }
    // Frees the line pointer
    free(lines);
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

int tradNameCmp(const void *a, const void *b) {
    const cafe_t *cafeA = *(const cafe_t **)a;
    const cafe_t *cafeB = *(const cafe_t **)b;
     return strcmp(cafeA->trad_name, cafeB->trad_name);
}

void arrayEnsureSize(array_t *arr) {
	if (arr->n == arr->size) {
		arr->size <<= 1;       // Same as arr->size *= 2;
		arr->A = realloc(arr->A, arr->size * sizeof(*(arr->A)));
		assert(arr->A);
	}
}


void arrayAppend(array_t *arr, cafe_t *c) {
    int i = arr->n - 1;
    arrayEnsureSize(arr);

    // Find the correct position to insert the new element based on trad_name
    while (i >= 0 && strcmp(arr->A[i]->trad_name, c->trad_name) > 0) {
        arr->A[i + 1] = arr->A[i];
        i--;
    }

    // Insert the new element in the correct position
    arr->A[i + 1] = c;
    (arr->n)++;
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

int esnureQuerySize(int queryCount, int initialQuerySize) {
    if (queryCount == initialQuerySize){
        return 1;
    }
    return 0;
}

void findAndTraverse(array_t *cafes, char *prefix, FILE *f) {
    int lo = 0;
    int hi = cafes->n - 1;
    int mid;
    int prefix_length = strlen(prefix);

    int strComp = 0;
    int charComp = 1;
    int byteComp = 1;

    fprintf(f, "%s\n", prefix);
    // Perform binary search to find the first matching prefix
    while (lo <= hi) {
        mid = (lo + hi) / 2;
        int cmp = strncmp(cafes->A[mid]->trad_name, prefix, prefix_length);

        if (cmp == 0) {
            // Found a match, print the record and move towards the start
            int start = mid;

            while (start >=0){
                //printf("%s\n", cafes->A[start]->trad_name);
                strComp++;
                charComp += charCompFunc(cafes->A[start]->trad_name, prefix, prefix_length);
                if (strncmp(cafes->A[start]->trad_name, prefix, prefix_length) != 0) {
                    break;
                }

                printOutFile(f, cafes, start);
                start--;
            }
            // Move towards the end
            int end = mid + 1;

            while (end < cafes->n){

                //printf("%s\n", cafes->A[end]->trad_name);
                strComp++;
                charComp += charCompFunc(cafes->A[end]->trad_name, prefix, prefix_length);
                if ( strncmp(cafes->A[end]->trad_name, prefix, prefix_length) != 0){
                    break;
                }

                printOutFile(f, cafes, end);
                end++;
            }
            break;
        }

        else if (cmp < 0) {
            lo = mid + 1;
        }
        else {
            hi = mid - 1;
        }
        strComp++;
        charComp += charCompFunc(cafes->A[mid]->trad_name, prefix, prefix_length);
    }
    byteComp = charComp * 8;
    printf("%s --> b%d c%d s%d\n", prefix, byteComp, charComp, strComp);
    return;
}

void printOutFile(FILE *f, array_t *cafes, int ind){
    fprintf(f, "--> census_year: %d || block_id: %d || property_id: %d"
        "|| base_property_id: %d || building_address: %s || clue_small_area: %s ||"
        " business_address: %s || trading_name: %s || industry_code: %d ||"
        " industry_description: %s || seating_type: %s || number_of_seats: %d ||"
        " longitude: %.5lf || latitude: %.5lf ||\n", cafes->A[ind]->cen_year,
        cafes->A[ind]->block_id, cafes->A[ind]->prop_id, cafes->A[ind]->base_prop_id,
        cafes->A[ind]->build_add, cafes->A[ind]->clue_small_area, cafes->A[ind]->bus_area,
        cafes->A[ind]->trad_name, cafes->A[ind]->indus_code, cafes->A[ind]->indus_desc,
        cafes->A[ind]->seat_type, cafes->A[ind]->num_of_seats, cafes->A[ind]->longitude,
        cafes->A[ind]->latitude);
}
int charCompFunc(char *trad_name, char *prefix, int prefix_length){
    int charComp =0;
    for (int i=0; i< prefix_length; i++){
        charComp++;
        if (trad_name[i] != prefix[i]){
            break;
        }
    }
    return charComp;
}

void freeCafes(array_t *arr){
     for (int i = 0; i < arr->n; i++) {
        free(arr->A[i]->build_add);
        free(arr->A[i]->clue_small_area);
        free(arr->A[i]->bus_area);
        free(arr->A[i]->trad_name);
        free(arr->A[i]->indus_desc);
        free(arr->A[i]->seat_type);
        free(arr->A[i]);
    }
    free(arr->A);
    free(arr);
}

void freeQueries(char **queries, int queryCount){
    for (int i = 0; i < queryCount; i++){
        free(queries[i]);
    }
    free(queries);
}
