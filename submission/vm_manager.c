#include <stdio.h>
#include <stdlib.h>

#define PM_SIZE 524288
#define D_SEGS 1024
#define FRAME_SIZE 512
#define MAX_FRAMES 1024

int PM[PM_SIZE];
int D[D_SEGS][FRAME_SIZE];
int frameUsed[MAX_FRAMES] = {0};
int nextFreeFrame = 0;

int getFreeFrame() {

    while (frameUsed[nextFreeFrame])
        nextFreeFrame++;

    frameUsed[nextFreeFrame] = 1;

    return nextFreeFrame++;
}

// initialize memory and disk

void initializeMemory() {

    for (int i = 0; i < PM_SIZE; i++)
        PM[i] = -1;

    // initialize disk so block k contains predictable data
    for (int i = 0; i < D_SEGS; i++)
        for (int j = 0; j < FRAME_SIZE; j++)
            D[i][j] = i * FRAME_SIZE + j;
}

// load initialization file

void loadInitialization(FILE *initFile) {

    char line[10000];

    // read segment table
    fgets(line, sizeof(line), initFile);

    int s, f, len;
    char *ptr = line;

    while (sscanf(ptr, "%d %d %d", &s, &f, &len) == 3) {

        PM[s] = f;  // store page table base

        if (f >= 0 && f < MAX_FRAMES)
            frameUsed[f] = 1;

        // move pointer forward
        for (int i = 0; i < 3; i++) {
            while (*ptr && *ptr != ' ') ptr++;
            while (*ptr == ' ') ptr++;
        }
    }

    // read page table entries
    fgets(line, sizeof(line), initFile);

    int seg, page, frame;
    ptr = line;

    while (sscanf(ptr, "%d %d %d", &seg, &page, &frame) == 3) {

        int ptBase = PM[seg];

        if (ptBase >= 0) {
            PM[ptBase + page] = frame;

            // mark frame as used
            if (frame >= 0 && frame < MAX_FRAMES)
                frameUsed[frame] = 1;
        }

        // move pointer forward
        for (int i = 0; i < 3; i++) {
            while (*ptr && *ptr != ' ') ptr++;
            while (*ptr == ' ') ptr++;
        }
    }
}

// detect first unused frame

void initializeFreeFramePointer() {

    int maxFrame = -1;

    for (int i = 0; i < MAX_FRAMES; i++) {
        if (frameUsed[i] && i > maxFrame)
            maxFrame = i;
    }

    nextFreeFrame = maxFrame + 1;
}

// translate va to pa

int translate(int VA, int demandPaging) {

    int s = VA >> 18;
    int p = (VA >> 9) & 511;
    int w = VA & 511;

    if (PM[s] == -1)
        return -1;

    int ptBase = PM[s];

    // page table on disk
    if (ptBase < 0) {
        if (!demandPaging)
            return -1;

        int block = -ptBase;
        int newFrame = getFreeFrame();
        int newBase = newFrame * FRAME_SIZE;

        // copy from disk to memory
        for (int i = 0; i < FRAME_SIZE; i++)
            PM[newBase + i] = D[block][i];

        PM[s] = newBase;
        ptBase = newBase;
    }

    if (PM[ptBase + p] == -1)
        return -1;

    int frame = PM[ptBase + p];

    // page on disk
    if (frame < 0) {
        if (!demandPaging)
            return -1;

        int block = -frame;
        int newFrame = getFreeFrame();

        // copy from disk to memory
        for (int i = 0; i < FRAME_SIZE; i++)
            PM[newFrame * FRAME_SIZE + i] = D[block][i];

        PM[ptBase + p] = newFrame;
        frame = newFrame;
    }

    return frame * FRAME_SIZE + w;
}

// main

int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Usage: %s init.txt input.txt output.txt dp_flag\n", argv[0]);
        printf("dp_flag: 0 = no demand paging, 1 = demand paging\n");
        return 1;
    }

    int demandPaging = atoi(argv[4]);

    initializeMemory();

    FILE *initFile = fopen(argv[1], "r");
    FILE *inputFile = fopen(argv[2], "r");
    FILE *outputFile = fopen(argv[3], "w");

    if (!initFile || !inputFile || !outputFile) {
        printf("File error.\n");
        return 1;
    }

    loadInitialization(initFile);
    initializeFreeFramePointer();

    int VA;
    while (fscanf(inputFile, "%d", &VA) == 1) {
        int PA = translate(VA, demandPaging);
        fprintf(outputFile, "%d ", PA);
    }

    fclose(initFile);
    fclose(inputFile);
    fclose(outputFile);

    return 0;
}