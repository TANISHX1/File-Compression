#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_TREE_HT 100
#define MAX_CHARS 256
#define MAX_PATH_LEN 256

// Huffman tree node
struct MinHeapNode {
    unsigned char data;  // Character
    unsigned freq;       // Frequency
    struct MinHeapNode* left, * right;
    };

// Min Heap structure (Priority Queue)
struct MinHeap {
    unsigned size;
    unsigned capacity;
    struct MinHeapNode** array;
    };

// Function prototypes
struct MinHeapNode* newNode(unsigned char data, unsigned freq);
struct MinHeap* createMinHeap(unsigned capacity);
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b);
void minHeapify(struct MinHeap* minHeap, int idx);
int isSizeOne(struct MinHeap* minHeap);
struct MinHeapNode* extractMin(struct MinHeap* minHeap);
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode);
void buildMinHeap(struct MinHeap* minHeap);
int isLeaf(struct MinHeapNode* root);
struct MinHeap* createAndBuildMinHeap(unsigned char data[], int freq[], int size);
struct MinHeapNode* buildHuffmanTree(unsigned char data[], int freq[], int size);
void printCodes(struct MinHeapNode* root, int arr[], int top);
void storeCodes(struct MinHeapNode* root, int arr[], int top, char* huffmanCodes[]);
void HuffmanCodes(unsigned char data[], int freq[], int size, char* huffmanCodes[]);
void countFrequency(FILE* file, int freq[], long* fileSize);
void compressFile(const char* input_file, const char* output_file);
void decompressFile(const char* input_file, const char* output_file);
int validatePath(const char* path, int isInputFile);
long getFileSize(const char* filename);

// Get file size using stat
long getFileSize(const char* filename)
    {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
        }
    return -1;
    }

// Allocate a new min heap node
struct MinHeapNode* newNode(unsigned char data, unsigned freq)
    {
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    if (temp == NULL) {
        printf("ERROR: Memory allocation failed\n");
        exit(1);
        }
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
    }

// Create a min heap of given capacity
struct MinHeap* createMinHeap(unsigned capacity)
    {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    if (minHeap == NULL) {
        printf("ERROR: Memory allocation failed\n");
        exit(1);
        }
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity * sizeof(struct MinHeapNode*));
    if (minHeap->array == NULL) {
        printf("ERROR: Memory allocation failed\n");
        exit(1);
        }
    return minHeap;
    }

// Swap two min heap nodes
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b)
    {
    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
    }

// Heapify at given index
void minHeapify(struct MinHeap* minHeap, int idx)
    {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size &&
        minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size &&
        minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
        }
    }

// Check if size of heap is 1
int isSizeOne(struct MinHeap* minHeap)
    {
    return (minHeap->size == 1);
    }

// Extract minimum value node from heap
struct MinHeapNode* extractMin(struct MinHeap* minHeap)
    {
    struct MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
    }

// Insert a new node to Min Heap
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode)
    {
    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
        }

    minHeap->array[i] = minHeapNode;
    }

// Build min heap
void buildMinHeap(struct MinHeap* minHeap)
    {
    int n = minHeap->size - 1;
    int i;
    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
    }

// Check if this node is leaf
int isLeaf(struct MinHeapNode* root)
    {
    return !(root->left) && !(root->right);
    }

// Creates a min heap and builds it
struct MinHeap* createAndBuildMinHeap(unsigned char data[], int freq[], int size)
    {
    printf("Creating min heap for %d characters...\n", size);

    struct MinHeap* minHeap = createMinHeap(size);
    if (!minHeap) {
        printf("ERROR: Failed to create min heap\n");
        exit(1);
        }

    for (int i = 0; i < size; ++i) {
        minHeap->array[i] = newNode(data[i], freq[i]);
        if (!minHeap->array[i]) {
            printf("ERROR: Failed to create node for character %d\n", i);
            exit(1);
            }
        }

    minHeap->size = size;
    printf("Building min heap...\n");
    buildMinHeap(minHeap);
    printf("Min heap built successfully\n");

    return minHeap;
    }

// Build Huffman Tree and return root
struct MinHeapNode* buildHuffmanTree(unsigned char data[], int freq[], int size)
    {
    printf("Building Huffman tree for %d characters...\n", size);

    struct MinHeapNode* left, * right, * top;
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

    printf("Constructing Huffman tree by merging nodes...\n");
    int nodes_merged = 0;

    // Step by step building of Huffman Tree
    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        // Create a new internal node with '$' as data and frequency equal to sum of two nodes
        top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);

        nodes_merged++;
        }

    printf("Huffman tree completed with %d merges\n", nodes_merged);
    return extractMin(minHeap);
    }

// Print huffman codes from the root of Huffman Tree
void printCodes(struct MinHeapNode* root, int arr[], int top)
    {
    if (root->left) {
        arr[top] = 0;
        printCodes(root->left, arr, top + 1);
        }

    if (root->right) {
        arr[top] = 1;
        printCodes(root->right, arr, top + 1);
        }

    if (isLeaf(root)) {
        printf("%c: ", root->data);
        for (int i = 0; i < top; ++i)
            printf("%d", arr[i]);
        printf("\n");
        }
    }

// Store huffman codes in an array for later use
void storeCodes(struct MinHeapNode* root, int arr[], int top, char* huffmanCodes[])
    {
    if (root->left) {
        arr[top] = 0;
        storeCodes(root->left, arr, top + 1, huffmanCodes);
        }

    if (root->right) {
        arr[top] = 1;
        storeCodes(root->right, arr, top + 1, huffmanCodes);
        }

    if (isLeaf(root)) {
        huffmanCodes[root->data] = (char*)malloc((top + 1) * sizeof(char));
        if (huffmanCodes[root->data] == NULL) {
            printf("ERROR: Memory allocation failed for huffman code storage\n");
            exit(1);
            }

        for (int i = 0; i < top; ++i)
            huffmanCodes[root->data][i] = arr[i] + '0';
        huffmanCodes[root->data][top] = '\0';
        }
    }

// Generate Huffman codes
void HuffmanCodes(unsigned char data[], int freq[], int size, char* huffmanCodes[])
    {
    printf("Generating Huffman codes...\n");
    struct MinHeapNode* root = buildHuffmanTree(data, freq, size);
    int arr[MAX_TREE_HT], top = 0;

    printf("Storing codes for each character...\n");
    storeCodes(root, arr, top, huffmanCodes);
    printf("Huffman codes generated successfully\n");
    }

// Count frequency of characters in a file using a buffer-based approach
void countFrequency(FILE* file, int freq[], long* fileSize)
    {
    // Clear all frequencies before starting
    for (int i = 0; i < MAX_CHARS; i++) {
        freq[i] = 0;
        }

    // Get file size from filesystem (more reliable in Linux)
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    if (file_size <= 0) {
        printf("ERROR: File appears to be empty or invalid\n");
        *fileSize = 0;
        return;
        }

    printf("File size is %ld bytes\n", file_size);
    *fileSize = file_size;

    // Use a buffer for more efficient reading
    const int BUFFER_SIZE = 8192;  // 8KB buffer
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    long total_read = 0;
    int progress = 0;

    printf("Counting character frequencies...\n");

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            freq[buffer[i]]++;
            }

        total_read += bytes_read;
        int new_progress = (total_read * 100) / file_size;

        if (new_progress / 10 > progress / 10) {
            progress = new_progress;
            printf("Progress: %d%% complete\n", progress);
            }
        }

    printf("Finished counting frequencies\n");
    rewind(file);
    }

// Validate file path
int validatePath(const char* path, int isInputFile)
    {
    if (isInputFile) {
        // For input file, check if it exists and is readable
        if (access(path, R_OK) != 0) {
            printf("Invalid path: Unable to read input file '%s'\n", path);
            return 0;
            }

        // Check if it's a regular file
        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            printf("Invalid path: '%s' is not a regular file\n", path);
            return 0;
            }

        // Check file size
        long size = getFileSize(path);
        if (size <= 0) {
            printf("Warning: File '%s' appears to be empty\n", path);
            }
        else if (size > 100000000) { // 100MB
            printf("Warning: File '%s' is very large (%ld bytes). This may take a while.\n", path, size);
            }
        }
    else {
        // For output file, check if directory is writable
        char dir_path[MAX_PATH_LEN];
        strcpy(dir_path, path);

        // Find last slash to get directory path
        char* last_slash = strrchr(dir_path, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';  // Terminate string at last slash to get directory

            // If no directory specified (just filename), use current directory
            if (strlen(dir_path) == 0) {
                strcpy(dir_path, ".");
                }
            }
        else {
            strcpy(dir_path, ".");  // No slash found, use current directory
            }

        // Check if directory is writable
        if (access(dir_path, W_OK) != 0) {
            printf("Invalid path: Unable to write to directory '%s'\n", dir_path);
            return 0;
            }
        }

    printf("Path '%s' is valid\n", path);
    return 1;
    }

// Compress the input file and write to output file
void compressFile(const char* input_file, const char* output_file)
    {
    FILE* in, * out;
    int freq[MAX_CHARS] = { 0 };
    unsigned char ch;
    int i, j, len;
    long fileSize = 0;

    printf("Starting compression...\n");
    printf("Opening input file: %s\n", input_file);

    // Check file size before opening
    long file_size = getFileSize(input_file);
    if (file_size <= 0) {
        printf("ERROR: Input file is empty or cannot be read\n");
        return;
        }
    else if (file_size > 100000000) { // 100MB
        printf("Warning: File is large (%ld bytes). Compression may take some time.\n", file_size);
        }

    // Open input file
    in = fopen(input_file, "rb");
    if (in == NULL) {
        printf("Error opening input file\n");
        return;
        }

    printf("Input file opened successfully\n");

    // Count frequency of each character
    countFrequency(in, freq, &fileSize);

    // Create array of characters and their frequencies
    unsigned char chars[MAX_CHARS];
    int freq_list[MAX_CHARS];
    int size = 0;

    printf("Building character frequency list...\n");
    for (i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            chars[size] = i;
            freq_list[size] = freq[i];
            size++;
            }
        }

    printf("Found %d unique characters\n", size);

    // Check if the file has any content
    if (size == 0 || fileSize == 0) {
        printf("ERROR: The input file is empty or no valid characters were found\n");
        fclose(in);
        return;
        }

    // Create and store Huffman codes
    char* huffmanCodes[MAX_CHARS] = { NULL };
    HuffmanCodes(chars, freq_list, size, huffmanCodes);

    printf("Opening output file: %s\n", output_file);

    // Open output file
    out = fopen(output_file, "wb");
    if (out == NULL) {
        printf("Error opening output file\n");
        fclose(in);
        return;
        }

    printf("Output file opened successfully\n");
    printf("Writing compressed data header...\n");

    // Write header: number of unique characters and their frequencies
    fwrite(&size, sizeof(int), 1, out);
    for (i = 0; i < size; i++) {
        fwrite(&chars[i], sizeof(unsigned char), 1, out);
        fwrite(&freq_list[i], sizeof(int), 1, out);
        }

    // Compress and write data
    printf("Compressing data...\n");
    rewind(in);

    // Use a buffer for more efficient reading
    const int BUFFER_SIZE = 8192;  // 8KB buffer
    unsigned char read_buffer[BUFFER_SIZE];
    size_t bytes_read;
    long total_read = 0;

    unsigned char bit_buffer = 0;
    int bit_position = 0;
    int total_bytes = 0;
    int progress = 0;

    while ((bytes_read = fread(read_buffer, 1, BUFFER_SIZE, in)) > 0) {
        for (size_t k = 0; k < bytes_read; k++) {
            ch = read_buffer[k];

            len = strlen(huffmanCodes[ch]);
            for (j = 0; j < len; j++) {
                if (huffmanCodes[ch][j] == '1')
                    bit_buffer |= (1 << bit_position);

                bit_position++;

                if (bit_position == 8) {
                    fwrite(&bit_buffer, sizeof(unsigned char), 1, out);
                    total_bytes++;
                    bit_buffer = 0;
                    bit_position = 0;
                    }
                }
            }

        total_read += bytes_read;
        int new_progress = (total_read * 100) / fileSize;

        if (new_progress / 10 > progress / 10) {
            progress = new_progress;
            printf("Compression progress: %d%% complete\n", progress);
            }
        }

    // Write remaining bits if any
    if (bit_position > 0) {
        fwrite(&bit_buffer, sizeof(unsigned char), 1, out);
        total_bytes++;
        }

    // Close files
    fclose(in);
    fclose(out);

    // Free allocated memory for codes
    for (i = 0; i < MAX_CHARS; i++)
        if (huffmanCodes[i] != NULL)
            free(huffmanCodes[i]);

    printf("File compressed successfully.\n");
    printf("Original size: %ld bytes\n", fileSize);
    int header_size = sizeof(int) + (size * (sizeof(unsigned char) + sizeof(int)));
    int compressed_size = total_bytes + header_size;
    printf("Compressed size: %d bytes (Header: %d bytes, Data: %d bytes)\n",
        compressed_size, header_size, total_bytes);

    if (fileSize > 0) {
        float ratio = (float)compressed_size / fileSize;
        printf("Compression ratio: %.2f%%\n", (1.0 - ratio) * 100);
        }
    }

// Decompress the input file and write to output file
void decompressFile(const char* input_file, const char* output_file)
    {
    FILE* in, * out;
    int size, i, j, freq;
    unsigned char ch;

    printf("Starting decompression...\n");
    printf("Opening input file: %s\n", input_file);

    // Open input file
    in = fopen(input_file, "rb");
    if (in == NULL) {
        printf("Error opening input file\n");
        return;
        }

    printf("Input file opened successfully\n");
    printf("Opening output file: %s\n", output_file);

    // Open output file
    out = fopen(output_file, "wb");
    if (out == NULL) {
        printf("Error opening output file\n");
        fclose(in);
        return;
        }

    printf("Output file opened successfully\n");
    printf("Reading header information...\n");

    // Read header
    if (fread(&size, sizeof(int), 1, in) != 1) {
        printf("ERROR: Failed to read header size\n");
        fclose(in);
        fclose(out);
        return;
        }

    if (size <= 0 || size > MAX_CHARS) {
        printf("ERROR: Invalid character count in header: %d\n", size);
        fclose(in);
        fclose(out);
        return;
        }

    printf("Found %d unique characters in header\n", size);

    unsigned char chars[MAX_CHARS];
    int freqs[MAX_CHARS];

    for (i = 0; i < size; i++) {
        if (fread(&chars[i], sizeof(unsigned char), 1, in) != 1 ||
            fread(&freqs[i], sizeof(int), 1, in) != 1) {
            printf("ERROR: Failed to read character data from header\n");
            fclose(in);
            fclose(out);
            return;
            }
        }

    printf("Rebuilding Huffman tree...\n");

    // Rebuild Huffman tree
    struct MinHeapNode* root = buildHuffmanTree(chars, freqs, size);

    // Calculate total characters to decode
    int total_chars = 0;
    for (i = 0; i < size; i++) {
        total_chars += freqs[i];
        if (chars[i] >= 32 && chars[i] <= 126) { // Printable ASCII
            printf("Character '%c' appears %d times\n", chars[i], freqs[i]);
            }
        else {
            printf("Character (ASCII %d) appears %d times\n", chars[i], freqs[i]);
            }
        }

    printf("Decompressing %d characters...\n", total_chars);

    // Decode
    int decoded_chars = 0;
    struct MinHeapNode* current = root;
    unsigned char bit;
    int progress = 0;

    while (decoded_chars < total_chars) {
        if (fread(&ch, sizeof(unsigned char), 1, in) != 1) {
            printf("WARNING: Unexpected end of compressed file\n");
            break;
            }

        for (i = 0; i < 8 && decoded_chars < total_chars; i++) {
            bit = (ch >> i) & 1;

            if (bit == 0)
                current = current->left;
            else
                current = current->right;

            if (current == NULL) {
                printf("ERROR: Invalid Huffman tree traversal\n");
                fclose(in);
                fclose(out);
                return;
                }

            if (isLeaf(current)) {
                fwrite(&current->data, sizeof(unsigned char), 1, out);
                decoded_chars++;
                current = root;

                int new_progress = (decoded_chars * 100) / total_chars;
                if (new_progress / 10 > progress / 10) {
                    progress = new_progress;
                    printf("Decompression progress: %d%% complete\n", progress);
                    }
                }
            }
        }

    // Close files
    fclose(in);
    fclose(out);

    printf("File decompressed successfully.\n");
    }

int main()
    {
    char option;
    char input_file[MAX_PATH_LEN];
    char output_file[MAX_PATH_LEN];
    int input_valid = 0;
    int output_valid = 0;

    printf("Text File Compression System\n");
    printf("----------------------------\n");

    // Get operation type
    while (1) {
        printf("Enter operation (c for compress, d for decompress): ");
        scanf(" %c", &option);

        if (option == 'c' || option == 'd')
            break;
        else
            printf("Invalid option. Please enter 'c' or 'd'.\n");
        }

    // Get input file path and validate
    while (!input_valid) {
        printf("Enter input file path: ");
        scanf("%s", input_file);
        input_valid = validatePath(input_file, 1);
        }

    // Get output file path and validate
    while (!output_valid) {
        printf("Enter output file path: ");
        scanf("%s", output_file);
        output_valid = validatePath(output_file, 0);
        }

    // Execute operation
    if (option == 'c')
        compressFile(input_file, output_file);
    else
        decompressFile(input_file, output_file);

    return 0;
    }