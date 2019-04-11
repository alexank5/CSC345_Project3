//project 3 - FIFO Implementation
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <alloca.h>

#define FRAME_SIZE 256        // size of the frame
#define TOTAL_NUMBER_OF_FRAMES 256  // in Physical memory, this is total number of frames
#define ADDRESS_MASK  0xFFFF  //mask all but the address
#define OFFSET_MASK  0xFF //mask all but the offset
#define TLB_SIZE 16       // size of the TLB
#define PAGE_TABLE_SIZE 256  // size of the page table
// number of characters to read for each line from input file

#define BUFFER_SIZE   10

// number of bytes to read

#define BLOCK   256


int table_num_page[PAGE_TABLE_SIZE];  // this array holds the page numbers in the page table

int frame_table_page[PAGE_TABLE_SIZE];   // this array holds the frame numbers in the page table

int page_num_TLB[TLB_SIZE];  // this array holds the page numbers in the TLB

int frame_num_TLB[TLB_SIZE]; // this array holds the frame numbers in the TLB

int phys_memory[TOTAL_NUMBER_OF_FRAMES][FRAME_SIZE]; //Here is physical memory 2D array

int faults_of_page = 0;   // counter in order to track page faults

int hits_of_TLB = 0;      // counter in order to track TLB hits

int init_avail_frame = 0;  // counter in order to track the first available frame

int init_avail_page_table_num = 0;  // counter in order to track the first available page table entry

int TLB_entries_numbers = 0;             // counter in order to track the number of entries in the TLB

int logicalAddresses[1000];
int physicalAddresses[1000];
int8_t byteValue[1000];


// Below is the input file and backing store

FILE    *address_file;
FILE    *backing_store;

// Below is how the program store reads from input file

char    address[BUFFER_SIZE];

int   logical_address;

// the buffer containing reads from backing store

signed char   buffer[BLOCK];

// the value of the byte (int8_t) in memory

int8_t   value;

// The headers for the functions used below 

void retrieve_page(int address, int index);

void read_from_backing_store(int pageNumber);

void insert_to_TLB_with_FIFO(int pageNumber, int number_of_frame);

// This function serves to take the logical address as well as obtain the physical address and value stored at that address

void retrieve_page(int logical_address, int arrayIndex){

   // obtain the page number and offset from  logical address

   int pageNumber = ((logical_address & ADDRESS_MASK)>>8);

   int offset = (logical_address & OFFSET_MASK);

   // first, obtain page from TLB

   int number_of_frame = -1; // initialized to -1 to indicate if it's valid in the conditionals below

   int i;  // search through TLB for a match

   for(i = 0; i < TLB_SIZE; i++){

       if(page_num_TLB[i] == pageNumber){   // if the TLB index is equal to (=) the page number

           number_of_frame = frame_num_TLB[i];  // if equal, then the frame number is extracted

               hits_of_TLB++;                // also if equal, then TLBHit counter is incremented
       }
   }

   // if the number_of_frame was not found

   if(number_of_frame == -1){

       int i;   // walk the contents of the page table

       for(i = 0; i < init_avail_page_table_num; i++){

           if(table_num_page[i] == pageNumber){         // if the page is located in those contents

               number_of_frame = frame_table_page[i];          // extract the number_of_frame from its twin array
           }
       }

       if(number_of_frame == -1){                     // if the page is not found in those contents

           read_from_backing_store(pageNumber);             // page fault, call to read_from_backing_store to get the frame into physical memory and the page table

           faults_of_page++;                          // increment the number of page faults

           number_of_frame = init_avail_frame - 1;  // and set the number_of_frame to the current init_avail_frame index

       }
   }

   insert_to_TLB_with_FIFO(pageNumber, number_of_frame);  // Her is the call to function to insert the page number and frame number into the TLB

   value = phys_memory[number_of_frame][offset];  // frame number and offset is used in order to obtain the signed value stored at that address

    //output the frame number and offset to the console

    //printf("frame number: %d\n", number_of_frame); 

    //printf("offset: %d\n", offset);

   // Next, output the virtual address, physical address and value of the signed char to the console

   //printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, (number_of_frame << 8) | offset, value);
   logicalAddresses[arrayIndex] = logical_address;
   physicalAddresses[arrayIndex] = (number_of_frame << 8) | offset;
   byteValue[arrayIndex] = value;
}

// This function serves to insert a page number and frame number into the TLB with a FIFO replacement implementation

void insert_to_TLB_with_FIFO(int pageNumber, int number_of_frame){

   int i;  // if it's already in the TLB, then prompt program to break

   for(i = 0; i < TLB_entries_numbers; i++){

       if(page_num_TLB[i] == pageNumber){

           break;
       }
   }

   // if the number of entries is equal to (=) the index

   if(i == TLB_entries_numbers){

       if(TLB_entries_numbers < TLB_SIZE){  // Also the TLB still has space in it

           page_num_TLB[TLB_entries_numbers] = pageNumber;    // insert the page and frame at the end

           frame_num_TLB[TLB_entries_numbers] = number_of_frame;
       }

       else{          // If not equal then move everything over

           for(i = 0; i < TLB_SIZE - 1; i++){

               page_num_TLB[i] = page_num_TLB[i + 1];

               frame_num_TLB[i] = frame_num_TLB[i + 1];
           }

           page_num_TLB[TLB_entries_numbers-1] = pageNumber;  // and then also insert the page and frame on the end

           frame_num_TLB[TLB_entries_numbers-1] = number_of_frame;
       }       
   }

   // if the index is not equal to (=) the number of entries

   else{

       for(i = i; i < TLB_entries_numbers - 1; i++){      // Here we iterate through up to one less than the number of entries

           page_num_TLB[i] = page_num_TLB[i + 1];      // Shift everything over in the arrays

           frame_num_TLB[i] = frame_num_TLB[i + 1];
       }


       if(TLB_entries_numbers < TLB_SIZE){                // if there is still space in the array, then place the page and frame on the end

           page_num_TLB[TLB_entries_numbers] = pageNumber;

           frame_num_TLB[TLB_entries_numbers] = number_of_frame;
       }

       else{                                             // otherwise put the page and frame on the number of entries - 1
           page_num_TLB[TLB_entries_numbers-1] = pageNumber;

           frame_num_TLB[TLB_entries_numbers-1] = number_of_frame;
       }
   }

   if(TLB_entries_numbers < TLB_SIZE){                    // if there is still space in the arrays, then increment the number of entries

       TLB_entries_numbers++;
   }   
}
// Here this function serves to read from the backing store and bring the frame into physical memory and the page table

void read_from_backing_store(int pageNumber){

   // first seek to byte BLOCK in the backing store

   // SEEK_SET in fseek() seeks from the beginning of the file

   if (fseek(backing_store, pageNumber * BLOCK, SEEK_SET) != 0) {

       fprintf(stderr, "Error seeking in backing store\n");
   }

   // Below now reads BLOCK bytes from the backing store to the buffer

   if (fread(buffer, sizeof(signed char), BLOCK, backing_store) == 0) {

       fprintf(stderr, "Error reading from backing store\n");       
   }

   // Here the program will load the bits into the first available frame in the physical memory 2D array

   int i;

   for(i = 0; i < BLOCK; i++){

       phys_memory[init_avail_frame][i] = buffer[i];
   }
   // and then here we load the frame number into the page table in the first available frame

   table_num_page[init_avail_page_table_num] = pageNumber;

   frame_table_page[init_avail_page_table_num] = init_avail_frame;

   // increment both counters that track the next available frames

   init_avail_frame++;

   init_avail_page_table_num++;
}

// main will open necessary files and then call on retrieve_page for every entry that is located in the addresses file

int main(int argc, char *argv[])
{

   // error checking/handling is performed

   if (argc != 2) {

       fprintf(stderr,"Usage: ./a.out [input file]\n");

       return -1;
   }

   // open the file that contains the backing store

   backing_store = fopen("BACKING_STORE.bin", "rb");

   if (backing_store == NULL) {

       fprintf(stderr, "Error opening BACKING_STORE.bin %s\n","BACKING_STORE.bin");

       return -1;
   }

   // open the file that contains the logical addresses

   address_file = fopen(argv[1], "r");

   if (address_file == NULL) {

       fprintf(stderr, "Error opening addresses.txt %s\n",argv[1]);

       return -1;

   }

   int numberOfTranslatedAddresses = 0;

   // read through the entire input file and output each logical address found

   while ( fgets(address, BUFFER_SIZE, address_file) != NULL) {

       logical_address = atoi(address);

       // get the physical address and then value stored at that address

       retrieve_page(logical_address,numberOfTranslatedAddresses);
       //printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddresses[numberOfTranslatedAddresses], physicalAddresses[numberOfTranslatedAddresses], byteValue[numberOfTranslatedAddresses]);

       numberOfTranslatedAddresses++;  // Here the program increments the number of translated addresses       
   }

   // calculate and print out the statistics the program obtained

   //printf("Number of translated addresses = %d\n", numberOfTranslatedAddresses);

   double pfRate = faults_of_page / (double)numberOfTranslatedAddresses;

   double TLBRate = hits_of_TLB / (double)numberOfTranslatedAddresses;

   //printf("Page Faults = %d\n", faults_of_page);

   printf("Page Fault Rate = %.3f%%\n",pfRate*100);

   //printf("TLB Hits = %d\n", hits_of_TLB);

   printf("TLB Hit Rate = %.3f%%\n", TLBRate*100);

    
   
   // Write Page Faults, Page Fault Rate, TLB Hits and TLB Hit Rate to out3.txt

   FILE *f1 = fopen("out1.txt", "wt");
   FILE *f2 = fopen("out2.txt", "wt");
   FILE *f3 = fopen("out3.txt", "wt");

   for(int a = 0; a < 1000; a++){
      fprintf(f1, "%d\n", logicalAddresses[a]);
      fprintf(f2, "%d\n", physicalAddresses[a]);
      fprintf(f3, "%d\n", byteValue[a]);
   }
   
   // close the input and backing store files

   fclose(address_file);
   fclose(backing_store);
   fclose(f1);
   fclose(f2);
   fclose(f3);
   return 0;

}

