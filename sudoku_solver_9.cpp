#include "sudoku_solver.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>
#include <string>

using namespace std;

#define MAX_K 1000

#define SIZE 9

string inputFileN_9;

// The square of the puzzle size
const int SIZE_SQUARED = SIZE*SIZE;
// The square root of the puzzle size
const int SIZE_SQRT = sqrt((double)SIZE);
// The number of columns in the exact cover matrix
const int ROW_NB = SIZE*SIZE*SIZE;
// The number of rows in the exact cover matrix
const int COL_NB = 4 * SIZE*SIZE;

// Node structure for the toroidal doubly linked list
// An instance of struct Node serving as the header for the linked list
struct Node Head_9;
// A pointer to the header node
struct Node* HeadNode_9 = &Head_9;

// An array of pointers to nodes representing the solution to the Sudoku puzzle
struct Node* solution_9[MAX_K];
// An array of pointers to nodes representing the original values present in the Sudoku grid
struct Node* orig_values_9[MAX_K];

// A 2D array representing the sparse matrix for the exact cover problem
bool matrix_9[ROW_NB][COL_NB] = { { 0 } };
// A Boolean flag indicating whether the Sudoku puzzle has been solved
bool isSolved_9 = false;

void MapSolutionToGrid_9(int Sudoku[][SIZE]);
void PrintGrid_9(int Sudoku[][SIZE]);

clock_t timer_9, timer2_9;

// Updates the linked list to exclude the covered column and its associated nodes
void coverColumn_9(Node* col) {
	col->left->right = col->right;
	col->right->left = col->left;
	for (Node* node = col->down; node != col; node = node->down) {
		for (Node* temp = node->right; temp != node; temp = temp->right) {
			temp->down->up = temp->up;
			temp->up->down = temp->down;
			temp->head->size--;
		}
	}
}

// Updates the linked list to include the uncovered column and its associated nodes
void uncoverColumn_9(Node* col) {
	for (Node* node = col->up; node != col; node = node->up) {
		for (Node* temp = node->left; temp != node; temp = temp->left) {
			temp->head->size++;
			temp->down->up = temp;
			temp->up->down = temp;
		}
	}
	col->left->right = col;
	col->right->left = col;
}

// Recursive function to solve the exact cover problem
// k is the current depth of the search tree
void search_9(int k) {

	//If the right of the header node points to itself, then we have covered all the columns and found a solution
	if (HeadNode_9->right == HeadNode_9) {
		timer2_9 = clock() - timer_9;
		int Grid[SIZE][SIZE] = { {0} };
		MapSolutionToGrid_9(Grid);
		string outputFileN_9 = inputFileN_9.substr(0, inputFileN_9.find(".txt")) + "_output.txt";
		ofstream outputFile(outputFileN_9);
		if (!outputFile.is_open()) {
			cerr << "Error opening file: " << outputFileN_9 << endl;
			return;
		}
		for (int i = 0; i < SIZE; ++i) {
			for (int j = 0; j < SIZE; ++j) {
				outputFile << Grid[i][j] << " ";
			}
			outputFile << endl;
		}
		outputFile.close();
		cout << "File created and written successfully: " << inputFileN_9.substr(0, inputFileN_9.find(".txt")) + "_output.txt" << endl << endl;
		cout << "Solved Puzzle:" << endl;
		PrintGrid_9(Grid);
		cout << "Time Elapsed: " << (float)timer2_9 / CLOCKS_PER_SEC << " seconds.\n\n";
		timer_9 = clock();
		isSolved_9 = true;
		return;
	}

	// Find the column with the smallest size and cover it (choose the column deterministically)
	Node* Col = HeadNode_9->right;
	for (Node* temp = Col->right; temp != HeadNode_9; temp = temp->right)
		if (temp->size < Col->size)
			Col = temp;

	// Cover the column
	coverColumn_9(Col);

	// For each row in the column, cover all columns that overlap with the row
	for (Node* temp = Col->down; temp != Col; temp = temp->down) {
		solution_9[k] = temp;
		for (Node* node = temp->right; node != temp; node = node->right) {
			coverColumn_9(node->head);
		}

		// Recursively call the search function
		search_9(k + 1);

		// backtrack and uncover the column
		temp = solution_9[k];
		solution_9[k] = NULL;
		Col = temp->head;
		for (Node* node = temp->left; node != temp; node = node->left) {
			uncoverColumn_9(node->head);
		}
	}

	uncoverColumn_9(Col);
}

// Map the solution to the grid
void MapSolutionToGrid_9(int Sudoku[][SIZE]) {
	for (int i = 0; solution_9[i] != NULL; i++) {
			Sudoku[solution_9[i]->rowID[1]-1][solution_9[i]->rowID[2]-1] = solution_9[i]->rowID[0];
	}
	for (int i = 0; orig_values_9[i] != NULL; i++) {
		Sudoku[orig_values_9[i]->rowID[1] - 1][orig_values_9[i]->rowID[2] - 1] = orig_values_9[i]->rowID[0];
	}
}

// Build the sparse matrix representation of the exact cover problem
void BuildSparseMatrix_9(bool matrix_9[ROW_NB][COL_NB]) {

	//Constraint 1: There can only be one value in any given cell
	int j = 0, counter = 0;
	for (int i = 0; i < ROW_NB; i++) { 
		matrix_9[i][j] = 1;
		counter++;
		if (counter >= SIZE) {
			j++;
			counter = 0;
		}
	}

	//Constraint 2: There can only be one instance of a number in any given row
	int x = 0;
	counter = 1;
	for (j = SIZE_SQUARED; j < 2 * SIZE_SQUARED; j++) {
		for (int i = x; i < counter*SIZE_SQUARED; i += SIZE)
			matrix_9[i][j] = 1;

		if ((j + 1) % SIZE == 0) {
			x = counter*SIZE_SQUARED;
			counter++;
		}
		else
			x++;
	}

	//Constraint 3: There can only be one instance of a number in any given column
	j = 2 * SIZE_SQUARED;
	for (int i = 0; i < ROW_NB; i++)
	{
		matrix_9[i][j] = 1;
		j++;
		if (j >= 3 * SIZE_SQUARED)
			j = 2 * SIZE_SQUARED;
	}

	//Constraint 4: There can only be one instance of a number in any given region
	x = 0;
	for (j = 3 * SIZE_SQUARED; j < COL_NB; j++) {

		for (int l = 0; l < SIZE_SQRT; l++) {
			for (int k = 0; k<SIZE_SQRT; k++)
				matrix_9[x + l*SIZE + k*SIZE_SQUARED][j] = 1;
		}

		int temp = j + 1 - 3 * SIZE_SQUARED;

		if (temp % (int)(SIZE_SQRT * SIZE) == 0)
			x += (SIZE_SQRT - 1)*SIZE_SQUARED + (SIZE_SQRT - 1)*SIZE + 1;
		else if (temp % SIZE == 0)
			x += SIZE*(SIZE_SQRT - 1) + 1;
		else
			x++;
	}
}

// Build the toroidal doubly linked list representation of the exact cover problem
void BuildLinkedList_9(bool matrix_9[ROW_NB][COL_NB]) {

	// Create the header node and initialize it to point to itself in all directions and 
	// set its size to -1 to distinguish it from other Column Nodes
	Node* header = new Node;
	header->left = header;
	header->right = header;
	header->down = header;
	header->up = header;
	header->size = -1;
	header->head = header;
	Node* temp = header;

	// Create all Column Nodes and initialize them to point to the header node in all directions 
	for (int i = 0; i < COL_NB; i++) {
		Node* newNode = new Node;
		newNode->size = 0;
		newNode->up = newNode;
		newNode->down = newNode;
		newNode->head = newNode;
		newNode->right = header;
		newNode->left = temp;
		temp->right = newNode;
		temp = newNode;
	}
	
	int ID[3] = { 0,1,1 };
	// Create all Row Nodes and initialize them to point to the Column Node they belong to in the head pointer
	// and to themselves in all other directions
	for (int i = 0; i < ROW_NB; i++) {
		Node* top = header->right;
		Node* prev = NULL;

		if (i != 0 && i%SIZE_SQUARED == 0) {
			ID[0] -= SIZE - 1;
			ID[1]++;
			ID[2] -= SIZE - 1;
		}
		else if (i!= 0 && i%SIZE == 0) {
			ID[0] -= SIZE - 1;
			ID[2]++;
		}
		else {
			ID[0]++;
		}

		// Create the Row Node and initialize it to point to the Column Node it belongs to in the head pointer
		for (int j = 0; j < COL_NB; j++, top = top->right) {
			if (matrix_9[i][j]) {
				Node* newNode = new Node;
				newNode->rowID[0] = ID[0];
				newNode->rowID[1] = ID[1];
				newNode->rowID[2] = ID[2];
				if (prev == NULL) {
					prev = newNode;
					prev->right = newNode;
				}
				newNode->left = prev;
				newNode->right = prev->right;
				newNode->right->left = newNode;
				prev->right = newNode;
				newNode->head = top;
				newNode->down = top;
				newNode->up = top->up;
				top->up->down = newNode;
				top->size++;
				top->up = newNode;
				if (top->down == top)
					top->down = newNode;
				prev = newNode;
			}
		}
	}
	HeadNode_9 = header;
}

// Cover values that are already present in the grid
void TransformListToCurrentGrid_9(int Puzzle[][SIZE]) {
	int index = 0;
	for(int i = 0 ; i<SIZE; i++ )
		for(int j = 0 ; j<SIZE; j++)
		// If the cell is not empty, cover the corresponding row
			if (Puzzle[i][j] > 0) {
				Node* Col = NULL;
				Node* temp = NULL;
				// Find the column that corresponds to the cell
				for (Col = HeadNode_9->right; Col != HeadNode_9; Col = Col->right) {
					for (temp = Col->down; temp != Col; temp = temp->down)
						if (temp->rowID[0] == Puzzle[i][j] && (temp->rowID[1] - 1) == i && (temp->rowID[2] - 1) == j)
							goto ExitLoops;
				}
				// Cover the column
ExitLoops:		coverColumn_9(Col);
				// Add the node to the array of original values
				orig_values_9[index] = temp;
				index++;
				// Cover all columns that overlap with the row
				for (Node* node = temp->right; node != temp; node = node->right) {
					coverColumn_9(node->head);
				}
			}
}

void SolveSudoku_9(int Sudoku[][SIZE], string inputFileName) {
	// start timer
	timer_9 = clock();
	//Functions to turn a Sudoku grid into an Exact Cover problem
	BuildSparseMatrix_9(matrix_9);
	// Build the toroidal doubly linked list
	BuildLinkedList_9(matrix_9);
	// Cover values that are already present in the grid
	TransformListToCurrentGrid_9(Sudoku);

	inputFileN_9 = inputFileName;

	// Solve the Exact Cover problem
	search_9(0);

	if (!isSolved_9)
		cout << "No Solution!" << endl;
	isSolved_9 = false;
}

// Print the grid
void PrintGrid_9(int Sudoku[][SIZE]){
	string extBorder = "+", intBorder = "|";
	int counter = 1;
	int additional = 0;
	if (SIZE > 9)
		additional = SIZE;
	for (int i = 0; i < ((SIZE +SIZE_SQRT - 1) * 2 +additional+ 1); i++) {
		extBorder += '-';
		if (i > 0 && i % ((SIZE_SQRT*2+SIZE_SQRT*(SIZE>9)+1)*counter + counter-1) == 0) {
			intBorder += '+';
			counter++;
		}
		else
			intBorder += '-';
	}
	extBorder += '+';
	intBorder += "|";
	cout << extBorder << endl;
	for (int i = 0; i<SIZE; i++){
		cout << "| ";
		for (int j = 0; j<SIZE; j++){
			if (Sudoku[i][j] == 0)
				cout << ". ";
			else
				cout << Sudoku[i][j] << " ";
			if (additional > 0 && Sudoku[i][j]<10)
				cout << " ";
			if ((j+1)%SIZE_SQRT == 0)
				cout << "| ";
		}
		cout << endl;
		if ((i + 1) % SIZE_SQRT == 0 && (i+1)<SIZE)
			cout << intBorder << endl;
	}
	cout << extBorder << endl << endl;
}