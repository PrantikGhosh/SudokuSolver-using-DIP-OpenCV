// header guards (#ifndef, #define, #endif) to prevent multiple inclusions
// in the same compilation unit, avoiding issues with duplicate definitions.
#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include <iostream>
#include <string>

// Represents nodes in a toroidal doubly linked list. 
struct Node {
	Node *left;
	Node *right;
	Node *up;
	Node *down;
	Node *head;
	
	int size;		//used for Column header
	int rowID[3];	//used to identify row in order to map solutions to a sudoku grid
};

void PrintGrid_9(int Sudoku[][9]);
void SolveSudoku_9(int Sudoku[][9], std::string inputFile);

void PrintGrid_16(int Sudoku[][16]);
void SolveSudoku_16(int Sudoku[][16], std::string inputFile);

#endif