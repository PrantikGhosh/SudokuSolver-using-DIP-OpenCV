# SudokuSolver-using-DIP-OpenCV
This project uses computer vision and machine learning techniques to detect and solve Sudoku puzzles from images uploaded by the user.

This repository includes the code and resources needed to detect Sudoku grids from images, recognize and extract digits from the grid, solve the Sudoku puzzle, and overlay the solution onto the original image. Additionally, it features a live project video description showcasing the program in action, along with an architecture diagram detailing the workflow.

The project offers a menu-driven interface that allows easy selection between file input or webcam capture, and it automatically manages and cleans up temporary files. It employs adaptive thresholding to enhance grid detection under varying lighting conditions, robust contour detection to reliably identify Sudoku grids in complex environments, and perspective transformation to standardize the grid for consistent digit recognition. The solution process is enhanced with an external C++ solver for efficient puzzle-solving and comprehensive error handling for robustness. The final solution is accurately overlaid on the original image.

# How to Run
1. Clone the repository Github: https://github.com/PrantikGhosh/SudokuSolver-using-DIP-OpenCV
2. For 9X9 Sudoku puzzles: python sudoku_vision_9by9.py
3. Ensure you have a C++ compiler installed for the external Sudoku solver.


# Architecture Diagram
![Architecture Diagram](https://github.com/PrantikGhosh/SudokuSolver-using-DIP-OpenCV/assets/84172492/89f2dcd8-3feb-4dde-a183-a4fc5fdb778b)


# Demonstration
https://github.com/PrantikGhosh/SudokuSolver-using-DIP-OpenCV/assets/84172492/3b632e24-c6a0-4ee6-9af2-fe3d2f03622b

---

Feel free to contribute to this project by forking the repository and submitting pull requests. If you encounter any issues or have suggestions for improvements, please open an issue on GitHub.
