/*
 *  prodcons header
 *  Function prototypes, data, and constants for producer/consumer module
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

// Data structure for the bounding buffer
// bigmatrix - the bounding buffer
// head - the index of the first matrix to get
// tail - the index of the last matrix added
// length - the length of the bounding buffer
typedef struct buffer {
  Matrix ** bigmatrix;
  int head;
  int tail;
  int length;
} buffer;


// PRODUCER-CONSUMER put() get() function prototypes
int put(Matrix * value);
Matrix * get();

// Data structure to track matrix production / consumption stats
// sumtotal - total of all elements produced or consumed
// multtotal - total number of matrices multipled
// matrixtotal - total number of matrces produced or consumed
typedef struct prodcons {
  int sumtotal;
  int multtotal;
  int matrixtotal;
} ProdConsStats;

// PRODUCER-CONSUMER thread method function prototypes
void *prod_worker(void *arg);
void *cons_worker(void *arg);

// Routines to add and remove matrices from the bounded buffer
int put(Matrix *value);
Matrix * get();