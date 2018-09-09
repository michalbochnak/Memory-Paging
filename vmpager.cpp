//
//  Michal Bochnak
//  Netid: mbochn2
//  Lab: Tuesday 4pm
//  CS 361 Homework #3
//  Oct 26, 2017
//

#include <iostream>
#include <stdlib.h>   // atoi
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>

using namespace std;
    
typedef struct {
    uint8_t pid;
    uint8_t page;
} MemoryAccess;

typedef struct {
    int frame;      // -1 if not currently loaded
    int frameFifo;  // used for FIFO memory
    int pageHits;
    int pageMisses;
} PageTableEntry;  

typedef struct {
    uint8_t pid;
    uint8_t page;
    bool vacant;
} FrameTableEntry;

// function prototypes
void showIdentyfyingInfo();


int main (int argc, char* argv[]) {

    char* filename;
    int memoryAccesses = 0, frameTableSize = 256,
        fileDescriptor, fileSize, memAccessesArrSize,
        totalPageTableHits = 0, totalPageTableMisses = 0,
        totalFrameTableHits = 0, totalFrameTableMisses = 0;
    struct stat statBuffer;
    MemoryAccess* memAccess = NULL;
    PageTableEntry pageTables[256][256];

    if (argc == 1) {
        cout << "Please, provide filename:" << endl;
        cout << "Usage: program_name file_name [#_memory_accesses frame_table_size]" << endl;
        exit(-1);
    }

    showIdentyfyingInfo();

    // process command line arguments
    if (argc == 2) {
        filename = argv[1];
    }
    if (argc == 3) {
        filename = argv[1];
        memoryAccesses = atoi(argv[2]);
    }
    if (argc == 4) {
        filename = argv[1];
        memoryAccesses = atoi(argv[2]);
        frameTableSize = atoi(argv[3]);
    }

    // open file
    if((fileDescriptor = open(filename, O_RDWR, 0)) < 0) {
        perror("opening file");
        exit(-2);
    }
    // get stats
    if(fstat(fileDescriptor, &statBuffer) < 0) {
        perror("fstat");
        exit(-3);
    }
    // determine file size 
    fileSize = statBuffer.st_size;
    // map the file
    if ((memAccess = (MemoryAccess*)mmap(NULL, fileSize, PROT_READ | PROT_WRITE,
		MAP_SHARED, fileDescriptor, 0)) == MAP_FAILED) {
            perror("mmap");
            exit(-4);
    }

    // set memory accesses size
    FrameTableEntry frameTable[frameTableSize];
    memAccessesArrSize = fileSize / sizeof(MemoryAccess);
    
    // set the memory accesses number to max if specified as 0
    if (memoryAccesses == 0) {
        memoryAccesses = memAccessesArrSize;
    }

    // initialize all to -1
    for (int i = 0; i < 256; ++i) {
        for (int k = 0; k < 256; ++k) {
            pageTables[i][k].frame = -1;
            pageTables[i][k].frameFifo = -1;
        }
    }

    // initialize vacants to true
    for (int i = 0; i < frameTableSize; ++i) {
        frameTable[i].vacant = true;
    }

    int frameIndex = 0;
    for (int i = 0; i < memoryAccesses; ++i) {

        // infinite memory
        if (pageTables[memAccess[i].pid][memAccess[i].page].frame >= 0) {
            totalPageTableHits++;
            pageTables[memAccess[i].pid][memAccess[i].page].pageHits++;
        }
        else {
            totalPageTableMisses++;
            pageTables[memAccess[i].pid][memAccess[i].page].pageMisses++;
            pageTables[memAccess[i].pid][memAccess[i].page].frame = 1;
        }   

        // FIFO memory
        if (pageTables[memAccess[i].pid][memAccess[i].page].frameFifo >= 0) {
            totalFrameTableHits++;
            pageTables[memAccess[i].pid][memAccess[i].page].pageHits++;
        }
        else {
            totalFrameTableMisses++;
            pageTables[memAccess[i].pid][memAccess[i].page].pageMisses++;
            if (frameTable[frameIndex].vacant == false) {
                pageTables[frameTable[frameIndex].pid][frameTable[frameIndex].page].frameFifo = -1;
            }
            frameTable[frameIndex].pid = memAccess[i].pid;
            frameTable[frameIndex].page = memAccess[i].page;
            pageTables[memAccess[i].pid][memAccess[i].page].frameFifo = frameIndex;
            frameTable[frameIndex].vacant = false;
            frameIndex = (frameIndex + 1) % frameTableSize;
        }        
    } 

    // print command line arguments
    cout << "Command line arguments provided: ";
    for (int i=1; i < argc; ++i) {
        if (i < argc - 1) 
            cout << argv[i] << ", ";
        else
            cout << argv[i] << ".";
    }
    cout << endl << endl;

    // print results
    cout << "# of memory accesses:    " << memoryAccesses << endl;
    cout << "Infinite memory hits:    " << totalPageTableHits << endl;
    cout << "Infinite memory misses:  " << totalPageTableMisses << endl;
    cout << "FIFO memory hits:        " << totalFrameTableHits << endl;
    cout << "FIFO memory misses:      " << totalFrameTableMisses << endl;
    cout << endl;

    return 0;
}   // end of main


// print author information
void showIdentyfyingInfo() {
    cout << "--------------------" << endl;
    cout << "   Michal Bochnak" << endl;
    cout << "   Netid: mbochn2" << endl;
    cout << "   Oct 26, 2017" << endl;
    cout << "--------------------" << endl << endl;
}
