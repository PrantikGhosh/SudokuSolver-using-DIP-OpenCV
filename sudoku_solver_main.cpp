#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "sudoku_solver.h"

using namespace std;

int main(int argc, char *argv[]){
	//check arguments
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " input_file_name.txt" << endl;
        return 1;
    }
    string inputFile = argv[1];

	//read input file 
	ifstream fileSize(inputFile);
	if (!fileSize.is_open()) {
		cerr << "Error opening file: " << inputFile << endl;
		return 1;
	}

	//read first line to get size of puzzle to determine which solver to use
	string firstLine;
	getline(fileSize, firstLine);
	istringstream iss(firstLine);
    int number;
	int count = 0;
    while (iss >> number) {
        count++;
    }
	const int SIZE = count;
	fileSize.close();
	
	//open input file again to read puzzle into 2D array
	ifstream file(inputFile);
	if (!file.is_open()) {
		cerr << "Error opening file: " << inputFile << endl;
		return 1;
	}

	//call appropriate solver based on puzzle size
    if (SIZE == 9) {
		//read input file and store in 2D array
		int Puzzle[9][9] = { {0} };
		for (int i = 0; i < 9; ++i) {
			for (int j = 0; j < 9; ++j) {
				file >> Puzzle[i][j];
			}
		}
		// print initial grid
		cout << "Input Puzzle:" << endl;
        PrintGrid_9(Puzzle);
		// solve puzzle
        SolveSudoku_9(Puzzle,inputFile);

    } else if (SIZE == 16) {
		//read input file and store in 2D array
		int Puzzle[16][16] = { {0} };
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				file >> Puzzle[i][j];
			}
		}
		// print initial grid	
        PrintGrid_16(Puzzle);
		// solve puzzle
        SolveSudoku_16(Puzzle,inputFile);
		
    } else
        cout << "Invalid puzzle size" << endl;
	
	file.close();
	return 0;
}