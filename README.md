# Camera function reproduction based on OpenCv + driver and control interface provided by DVSense Driver

This repository contains C++ applications developed using the official DVSense driver library for interacting with DVS (Dynamic Vision Sensor) cameras. The project consists of two main components:

1.  **Event Acquirer (`dvsense_driver_test_12.7`)**: Connects to the DVS camera using the DVSense SDK to capture and save the raw event stream data.
2.  **Event Processor/Visualizer (`dvsense_driver_test2`)**: Processes DVS event data (likely from raw files or potentially a live stream) and uses the OpenCV library to visualize or convert the event stream into standard `.avi` video files.

This project uses CMake for build management.

## Features

* Connects to and initializes DVSense-compatible DVS cameras (via Component 1).
* **Component 1 (`dvsense_driver_test_12.7`)**: Acquires and saves the camera's raw event stream data into `.raw` files.
* **Component 2 (`dvsense_driver_test2`)**: Processes DVS event data and visualizes/records it into video files (`.avi` files) using OpenCV.
* Cross-platform build configuration (using CMake).

## Dependencies

1.  **DVSense Driver SDK**: **Core Dependency**. You **must** first install the DVSense driver and SDK (including headers and libraries) on your system according to the official DVSense instructions. The CMake build process needs to be able to find this SDK.
    * **Installation Guide**: [https://sdk.dvsense.com/zh/v1.0.0/install_zh.html](https://sdk.dvsense.com/zh/v1.0.0/install_zh.html) (Link provided is for v1.0.0 Chinese version).
2.  **OpenCV**: **Required for Component 2 (`dvsense_driver_test2`)**. The event processing and visualization component requires the OpenCV library (a recent version like 4.x is recommended). Please install OpenCV on your system (e.g., via package manager, build from source, or download pre-built libraries from [opencv.org](https://opencv.org/)). CMake needs to be able to find your OpenCV installation.

## Building from Source

If you have the source code and want to compile the executable(s) on your own machine, follow these steps:

**1. Prerequisites:**

* **Git**: To clone this repository.
* **CMake**: Version 3.10 or higher recommended. Download from [cmake.org](https://cmake.org/download/).
* **C++ Compiler**: Supporting C++11 or later (e.g., Visual Studio).
* **Installed DVSense Driver SDK**: Mandatory. Follow the official guide: [https://sdk.dvsense.com/zh/v1.0.0/install_zh.html]
* **Installed OpenCV Library**: Mandatory for Component 2. Ensure CMake can find it (often automatic if installed system-wide, or use CMake options like `-DOpenCV_DIR=/path/to/opencv/build` or similar).

**2. Build Steps:**

1.  **Clone the Repository**:
    ```bash
    git clone git@github.com:DokiforU/DV_Repetition.git # Use your repository's SSH URL
    cd DV_Repetition
    ```

2.  **Create a Build Directory**: Recommended practice.
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure Project (Run CMake)**: From within the `build` directory.
    ```bash
    # Basic command
    cmake ..
    ```
    Adjust the CMake command based on your environment, generators, and where your dependencies (DVSense SDK, OpenCV) are installed if CMake cannot find them automatically. Check `CMakeLists.txt` for specific variable names if needed. Look for "Configuring done" and "Generating done".

4.  **Compile Project**: Still within the `build` directory.
    ```bash
    # Generic build command
    cmake --build .
    ```
    A successful build might create one or more executable files within the `build` directory or its subdirectories (e.g., `build/Debug/`, `build/Release/`), depending on how the `CMakeLists.txt` files are set up to handle the two components.

## Running the Application

1.  **Locate Executable(s)**: After a successful build, find the compiled executable(s) (e.g., potentially `dvsense_event_acquirer`, `dvsense_event_processor`, or similar names defined in your CMake configuration) inside the build directory (`build/`, `build/Debug/`, `build/Release/`).
2.  **Connect DVS Camera**: Ensure the camera is connected if you intend to run the acquisition component.
3.  **Run**: Open your terminal/command prompt, navigate to the directory containing the desired executable, and run it.
    * **Windows**: `.\your_executable_name.exe`
    You may need to run the acquisition component first to generate `.raw` files before running the processing component, or they might be designed to run independently or communicate differently. Check the source code (`main.cpp` files in both directories) or console output for usage instructions.

## Output Files

Based on the project structure and your description:

* `.raw` files (raw event stream data from Component 1) are saved in: `get_File(.raw)/` (relative to the project root).
* `.avi` files (video recordings/visualizations from Component 2) are saved in: `get_File(.avi)/` (relative to the project root).

Please check the program's console output or source code (`main.cpp` in respective directories) for specific filename conventions.
