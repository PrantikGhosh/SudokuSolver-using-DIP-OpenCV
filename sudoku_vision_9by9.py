import os
import cv2
import numpy as np
import operator
import subprocess
from keras.models import load_model

size = 9
margin = 4
cell_size = 28 + 2 * margin
grid_size = size * cell_size

classifier = load_model("Model/digit_model.h5")

# Function to convert the image to a binary image
def binary_image(img):
    # Convert the image to grayscale
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # Apply Gaussian blur to the image to remove noise
    gray = cv2.GaussianBlur(gray, (7, 7), 0)
    # Apply adaptive thresholding to get a binary image
    thresh = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 9, 2)
    return thresh

# Function to find the grid contour
def contour(img):
    contours, hierarchy = cv2.findContours(img, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    grid_contour = None
    max_area = 0

    # Find the contour with the maximum area which is the contour of the grid
    for contour in contours:
        area = cv2.contourArea(contour)
        if area > 25000:
            arc_length = cv2.arcLength(contour, True)
            polygone = cv2.approxPolyDP(contour, 0.01 * arc_length, True)
            if area > max_area and len(polygone) == 4:
                grid_contour = polygone
                max_area = area
    
    return grid_contour

# Function to warp the image to get a top-down view of the grid
def wrap_perspective(frame, grid_contour):
    # Sort the corners of the grid from top to bottom and left to right
    points = np.vstack(grid_contour).squeeze()
    points = sorted(points, key=operator.itemgetter(1))
    if points[0][0] < points[1][0]:
        if points[3][0] < points[2][0]:
            pts1 = np.float32([points[0], points[1], points[3], points[2]])
        else:
            pts1 = np.float32([points[0], points[1], points[2], points[3]])
    else:
        if points[3][0] < points[2][0]:
            pts1 = np.float32([points[1], points[0], points[3], points[2]])
        else:
            pts1 = np.float32([points[1], points[0], points[2], points[3]])
    pts2 = np.float32([[0, 0], [grid_size, 0], [0, grid_size], [
                        grid_size, grid_size]])
    
    # Warp the image to get a top-down view of the grid
    M = cv2.getPerspectiveTransform(pts1, pts2)
    grid = cv2.warpPerspective(frame, M, (grid_size, grid_size))

    return grid, pts1, pts2

# Function to get the value of each cell in the grid
def get_cell_value( grid, sudoku_grid_unsolved):
    print("Extracting digits from the grid...")
    # create folder to store the cells
    if not os.path.exists("cells"):
        os.makedirs("cells")
    for y in range(size):
        row = []
        for x in range(size):
            y2min = y * cell_size + margin
            y2max = (y + 1) * cell_size - margin
            x2min = x * cell_size + margin
            x2max = (x + 1) * cell_size - margin
            img = grid[y2min:y2max, x2min:x2max]
            cv2.imwrite(f"cells\cell_{y}_{x}.jpg", img)
            # Resize the image to 28x28
            img = cv2.resize(img, (28, 28))

            # Reshape the image to (1, 28, 28, 1)
            img = img.reshape(1, 28, 28, 1)

            # Normalize the pixel values to be between 0 and 1
            img = img.astype('float32') / 255.0

            # Make a prediction
            predict = classifier.predict(img)
            predicted_class = np.argmax(predict[0])
            row.append(predicted_class)

        sudoku_grid_unsolved.append(row)

    return sudoku_grid_unsolved

def run_solver(sudoku_grid_solved, isSolved = False):
    # Command to compile the C++ code
    compile_command = "g++ sudoku_solver_main.cpp sudoku_solver_16.cpp sudoku_solver_9.cpp -o dancingLinks"

    # Command to run the compiled program with an input file
    run_command = "dancingLinks grid.txt"

    # Compile the C++ code
    compile_process = subprocess.run(compile_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Check if the compilation was successful
    if compile_process.returncode == 0:
        print("Compilation successful")
        
        # Run the compiled program
        run_process = subprocess.run(run_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        # Print the output of the program
        print("Program output:")
        print(run_process.stdout.decode())

        if run_process.returncode == 0:    
            if os.path.exists("grid_output.txt"):
                isSolved = True
                with open("grid_output.txt", 'r') as file:
                    for line in file:
                        # Split the line into individual numbers and convert them to integers
                        row = [int(num) for num in line.split()]
                        # Append the row to the 2D array
                        sudoku_grid_solved.append(row)
            else:
                print("No Solution Found")
                return None
        else:
            print("Program failed:")
            print(run_process.stderr.decode())
        
        # Print any errors or messages from the program
        if run_process.stderr:
            print("Error or message from the program:")
            print(run_process.stderr.decode())
    else:
        print("Compilation failed:")
        print(compile_process.stderr.decode())

    return sudoku_grid_solved, isSolved

# Function to overlay the solved numbers on the original image
def overlay_grid(frame, pts1, pts2, sudoku_grid_solved, sudoku_grid_unsolved):
    copy_img = np.zeros(shape=(grid_size, grid_size, 3), dtype=np.float32)

    # Iterate through the cells in the solved puzzle array and overlay the numbers on the photo
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 1
    font_thickness = 2

    for i in range(size):
        for j in range(size):
            if sudoku_grid_unsolved[i][j] == 0:
                # Calculate the position to overlay the number
                x_position = j * cell_size + cell_size // 2 - 12
                y_position = i * cell_size + cell_size // 2 + 12

                # Overlay the solved number only inside the puzzle area
                cv2.putText(copy_img, str(sudoku_grid_solved[i][j]), (x_position, y_position),
                            font, font_scale, (0, 0, 255), font_thickness)

    M = cv2.getPerspectiveTransform(pts2, pts1)
    h, w, c = frame.shape
    copy_img_p = cv2.warpPerspective(copy_img, M, (w, h))
    img2gray = cv2.cvtColor(copy_img_p, cv2.COLOR_BGR2GRAY)
    ret, mask = cv2.threshold(img2gray, 10, 255, cv2.THRESH_BINARY)
    mask = mask.astype('uint8')
    mask_inv = cv2.bitwise_not(mask)
    img1_bg = cv2.bitwise_and(frame, frame, mask=mask_inv)
    img2_fg = cv2.bitwise_and(copy_img_p, copy_img_p, mask=mask).astype('uint8')
    dst = cv2.add(img1_bg, img2_fg)
    dst = cv2.resize(dst, (1080, 620))
    cv2.imshow("output", dst)
    cv2.waitKey(0)
    cv2.imwrite("output.jpg", dst)

# Function to extract the digits from the grid
def extract_digit(frame, grid_contour):
    # Draw the grid contour on the original image
    cv2.drawContours(frame, [grid_contour], 0, (0, 255, 0), 2)

    # Warp the image to get a top-down view of the grid
    grid, pts1, pts2 = wrap_perspective(frame, grid_contour)

    # Convert the image to grayscale and apply adaptive thresholding
    grid = cv2.cvtColor(grid, cv2.COLOR_BGR2GRAY)
    grid = cv2.adaptiveThreshold(grid, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 7, 3)

    sudoku_grid_solved = []
    sudoku_grid_unsolved = []

    # Get the value of each cell in the grid
    sudoku_grid_unsolved = get_cell_value(grid, sudoku_grid_unsolved)

    # save the grid to a text file
    with open("grid.txt", "w") as f:
        for row in sudoku_grid_unsolved:
            for x in row:
                f.write(str(x)+" ")
            f.write("\n")

    # Run the solver
    sudoku_grid_solved, isSolved = run_solver(sudoku_grid_solved)
    
    if isSolved:
        overlay_grid(frame, pts1, pts2, sudoku_grid_solved, sudoku_grid_unsolved)
    else:
        print("Error: Could not solve the puzzle.")

# extract the Sudoku grid from the image
def extract_grid(frame):
    ret = frame

    # get the binary image
    thresh = binary_image(frame)

    # find the grid contour
    grid_contour = contour(thresh)

    if grid_contour is not None:
        extract_digit(frame, grid_contour)
    else:
        print("Error: Could not find the Sudoku grid.")

# Function to select an image from the files
def select_image_from_files():
    file_path = input("Enter the path of the image file: ")

    # Check if the file exists
    if os.path.exists(file_path):
        image = cv2.imread(file_path)
        print("Image loaded successfully.")
        extract_grid(image)
    else:
        print(f"Error: {file_path} not found.")

# Function to capture an image from the camera
def capture_image_from_camera():
    # Open the webcam 
    cap = cv2.VideoCapture(0)

    # Check if the webcam is opened successfully
    if not cap.isOpened():
        print("Error: Could not open webcam.")
        return

    print("Press 'Space' to capture an image.")

    while True:
        # Capture a single frame
        ret, frame = cap.read()

        # Check if the frame was captured successfully
        if not ret:
            print("Error: Could not read frame.")
            break

        # Display the frame
        cv2.imshow("Webcam", frame)

        # Wait for a key press (1 ms delay to allow imshow to refresh)
        key = cv2.waitKey(1)

        # Check if the key pressed is 'Space' (key code 32)
        if key == 32:
            # Save the captured frame to an image file
            cv2.imwrite("captured_image.jpg", frame)
            print("Image captured successfully.")
            extract_grid(frame)
            break
        elif key == 27:  # Check if the key pressed is 'Esc' (key code 27)
            print("Image capture cancelled.")
            break

    cap.release()

if __name__ == "__main__":
    while True:
        if os.path.exists("output.jpg"):
            os.remove("output.jpg")
        if os.path.exists("grid.txt"):
            os.remove("grid.txt")
        if os.path.exists("grid_output.txt"):
            os.remove("grid_output.txt")
        if os.path.exists("dancingLinks.exe"):
            os.remove("dancingLinks.exe")
        if os.path.exists("captured_image.jpg"):
            os.remove("captured_image.jpg")
        if os.path.exists("cells"):
            for file in os.listdir("cells"):
                os.remove(f"cells\{file}")
            os.rmdir("cells")

        print("Select an option:")
        print("1. Select Image from files")
        print("2. Capture Image from camera (Press 'Space' to capture, 'Esc' to cancel)")
        print("3. Quit")

        choice = input("Enter your choice : ")

        if choice == "1":
            select_image_from_files()
        elif choice == "2":
            capture_image_from_camera()
        elif choice == "3":
            print("Exiting program.")
            break
        else:
            print("Invalid choice. Please enter 1, 2, or 3.")
    
    cv2.destroyAllWindows()