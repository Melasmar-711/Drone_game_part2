#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <subscriber|publisher|both>"
    exit 1
fi

# Assign the argument to a variable
MODE=$1

# Validate the argument
if [ "$MODE" != "subscriber" ] && [ "$MODE" != "publisher" ] && [ "$MODE" != "both" ]; then
    echo "Invalid argument: $MODE. Use 'subscriber', 'publisher', or 'both'."
    exit 1
fi

# Clean up any existing blackboard pipe (if it exists)
if [ -e "blackboard_pipe" ]; then
    echo "Removing existing blackboard_pipe..."
    rm blackboard_pipe
fi

# Define the log directory (update the path if necessary)
LOG_DIR="Logs"  # Assuming your log files are in a directory called "logs"

# Create the log directory if it doesn't exist
mkdir -p "$LOG_DIR"

# Define the log files
LOG_FILES=(
    "BlackBoardServer.log"
    "DroneDynamicsManager.log"
    "GameWindow.log"
    "KeyboardManager.log"
    "ObstacleGenerator.log"
    "TargetsGenerator.log"
)

# Create log files if they don't exist
for log_file in "${LOG_FILES[@]}"; do
    if [ ! -e "$LOG_DIR/$log_file" ]; then
        echo "Creating $LOG_DIR/$log_file..."
        touch "$LOG_DIR/$log_file"
    fi
done

# Clean up log files (delete all .log files in the logs directory)
if [ -d "$LOG_DIR" ]; then
    echo "Cleaning up existing log files in $LOG_DIR..."
    rm -f "$LOG_DIR"/*.log  # Remove all .log files in the logs folder
fi

# Define the build directory
BUILD_DIR="build"

# Clean previous build files if necessary
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning up previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create a fresh build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure the project with CMake
echo "Configuring the project with CMake..."
cmake ..

# Build the project using the generated Makefiles
echo "Building the project..."
cmake --build .

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

cleanup() {
    echo "Terminating background processes..."
    pkill -P $$  # Kill all child processes of this script
    exit 0
}

# Trap SIGINT (Ctrl+C) and call the cleanup function
trap cleanup SIGINT

# Start the processes (you can specify paths for log files or they will be generated later)

start_publisher() {
    gnome-terminal --geometry=80x24+0+0 -- ./Targets_publisher_subscriber/Targets "publisher" &
    gnome-terminal --geometry=80x24+800+0 -- ./Obstacles_publisher_subscriber/Obstacles "publisher" &
    gnome-terminal --geometry=80x24+0+600 -- ./Targets_Generator/Targets_Generator "publisher" &
    gnome-terminal --geometry=80x24+800+600 -- ./Obstacle_Generator/Obstacle_Generator "publisher" &
}

start_subscriber() {
    gnome-terminal --geometry=80x24+0+0 -- ./BlackBoardServer/BlackBoardServer &
    gnome-terminal --geometry=80x24+960+0 -- ./DroneDynamicsManager/DroneDynamicsManager &
    gnome-terminal --geometry=110x40+0+540 -- bash -c "./GameWindow/GameWindow; exec bash" &
    gnome-terminal --geometry=80x24+960+540 -- ./KeyboardManager/KeyboardManager "subscriber" &
    gnome-terminal --geometry=80x24+0+1080 -- ./Targets_publisher_subscriber/Targets "subscriber" &
    gnome-terminal --geometry=80x24+960+1080 -- ./Obstacles_publisher_subscriber/Obstacles "subscriber" &
    gnome-terminal --geometry=80x24+0+1620 -- ./Targets_Generator/Targets_Generator "subscriber" &
    gnome-terminal --geometry=80x24+960+1620 -- ./Obstacle_Generator/Obstacle_Generator "subscriber" &
}

if [ "$MODE" == "publisher" ]; then
    start_publisher
    wait
elif [ "$MODE" == "subscriber" ]; then
    start_subscriber
    wait
elif [ "$MODE" == "both" ]; then
    start_publisher
    start_subscriber
    wait
fi


