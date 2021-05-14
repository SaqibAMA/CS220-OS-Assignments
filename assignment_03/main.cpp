#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <semaphore.h>
#include <chrono>

using namespace std;

// Getting number of processors
auto P = thread::hardware_concurrency();

// Constants
#define ROW vector<int>
#define MATRIX vector<ROW>


// Creating mutex semaphore
sem_t mutex;


// Stores the product of the matrix.
MATRIX product;

// Helper functions

// Matrix generation function
void generateMatrix(const char* filename, const int rows, const int cols) {


  // Opening file
  ofstream fout;
  fout.open(filename);

  // NULL seed.
  srand(time(NULL));



  // Writing rows and cols
  fout << rows << endl;
  fout << cols << endl;



  // Writing the matrix
  for (int i = 0; i < rows; i++) {

    for (int j = 0; j < cols; j++) {


      // Any number between 0 and 500
      fout << rand() % 500 << " ";

    }

    fout << endl;

  }



  // Let's get out
  fout.close();

}

// Def: Reads a file.
// Ret: A matrix.
MATRIX readFromFile (const char* filename) {


  ifstream fin;
  fin.open(filename);


  if (!fin) throw runtime_error("Could not open file!");


  int rows = 0;
  int col = 0;

  fin >> rows;
  fin >> col;


  MATRIX m(rows);


  for (auto & i : m) {

    i.resize(col);

    for (int & j : i ) {

      fin >> j;

    }

  }


  fin.close();

  return m;


}

// Def: Prints the matrix.
// Ret: void.
void print(const MATRIX& m) {

  cout << "m => " << endl;

  for (auto i : m) {
    for (int j : i) {
      cout << setw(5) << j << " ";
    }
    cout << endl;
  }

  cout << "\n\n";

}


// Def: Take a row and computes it's product with the entire matrix.
// Ret: void.
void getRowProduct(const ROW& r1, const MATRIX& m2, const int rowNumber) {

  ROW result(m2[0].size());
  int resultCol = 0;


  for (int m2Col = 0; m2Col < m2[0].size(); m2Col++) {

    int currentValueToPut = 0;

    for (int r1Index = 0; r1Index < r1.size(); r1Index++) {

      currentValueToPut += r1[r1Index] * m2[r1Index][m2Col];

    }

    result[resultCol] = currentValueToPut;
    ++resultCol;

  }


  // -- Critical section --

  sem_wait(&mutex);

  for (int i = 0; i < result.size(); i++)
    product[rowNumber][i] = result[i];

  sem_post(&mutex);

  // -- End of critical section --

}

// Core function
// Def: Multithreaded multiplication.
// Ret: void.
void multiply_mt(const MATRIX& m1, const MATRIX& m2) {

  // Incompatible
  if (m1[0].size() != m2.size())
    throw runtime_error("Invalid matrix size!");

  // Determining the amount of threads we need
  if (m1.size() < P)
    P = m1.size();

  // Allocating product matrix
  product.resize(m1.size());
  for (auto & i : product)
    i.resize(m2[0].size());

  // Keeps a track of how many rows we have processed.
  int rowsProcessed = 0;
  int rowsJoined = 0;

  // All threads are being queued in this
  vector <thread*> threadQueue;

  while (rowsProcessed < m1.size()) {

    for (int i = 0; i < P && rowsProcessed < m1.size(); i++) {
      threadQueue.push_back(new thread(getRowProduct, m1[rowsProcessed], m2, rowsProcessed));
      ++rowsProcessed;
    }
  
  }

}


// Def: Non-Multithreaded multiplication.
// Ret: void.
void multiply(const MATRIX& m1, const MATRIX& m2) {

  // Incompatible
  if (m1[0].size() != m2.size())
    throw runtime_error("Invalid matrix size!");

  // Determining the amount of threads we need
  if (m1.size() < P)
    P = m1.size();

  // Allocating product matrix
  product.resize(m1.size());
  for (auto & i : product)
    i.resize(m2[0].size());


  // Computing product
  for (int i = 0; i < m1.size(); i++) {
    getRowProduct(m1[i], m2, i);
  }

}

int main() {


  // Generating random matrices
  // filename, rows, cols
  generateMatrix("m1.txt", 1000, 1000);
  generateMatrix("m2.txt", 1000, 1000);


  // Creating a semaphore -- helpful when writing data
  sem_init(&mutex, 1, 1);

  // Reading matrix from the file.
  MATRIX m1 = readFromFile("m1.txt");
  MATRIX m2 = readFromFile("m2.txt");

  cout << "m1 order: " << m1.size() << "x" << m1[0].size() << endl;
  cout << "m2 order: " << m2.size() << "x" << m2[0].size() << endl;


  // Use these to check matrices.
  // print(m1);
  // print(m2);

  // -- MULTITHREAD -- 
  auto start = chrono::high_resolution_clock::now();
  ios_base::sync_with_stdio(false);


  multiply_mt(m1, m2);


  auto end = chrono::high_resolution_clock::now();

  double timeTaken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  timeTaken *= 1e-9;

  cout << "Time taken by multithreaded multiplication: " << timeTaken << setprecision(9) << "seconds. ";
  cout << "No of processors: " << P << endl;

  // Use this to print the product
  // print(product);


  // ---- NON-MULTITHREADED ----
  start = chrono::high_resolution_clock::now();
  ios_base::sync_with_stdio(false);

  multiply(m1, m2);

  end = chrono::high_resolution_clock::now();
  timeTaken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  timeTaken *= 1e-9;

  cout << "Time taken by single threaded multiplication: " << timeTaken << setprecision(9) << "seconds. ";
  cout << "No of processors: 1" << endl;
  

  sem_destroy(&mutex);

  return 0;

}