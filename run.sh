#!/bin/bash



# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <subscriber|publisher>"
    exit 1
fi

# Assign the argument to a variable
MODE=$1

# Validate the argument
if [ "$MODE" != "subscriber" ] && [ "$MODE" != "publisher" ]; then
    echo "Invalid argument: $MODE. Use 'subscriber' or 'publisher'."
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

# Start the processes (you can specify paths for log files or they will be generated later)
gnome-terminal -- ./BlackBoardServer/BlackBoardServer &
gnome-terminal -- ./DroneDynamicsManager/DroneDynamicsManager &
gnome-terminal --geometry=110x40+0+0 -- bash -c "./GameWindow/GameWindow; exec bash" &
gnome-terminal -- ./KeyboardManager/KeyboardManager "$MODE" &

if [ "$MODE" == "publisher" ]; then
    gnome-terminal -- ./Targets_publisher_subscriber/Targets "$MODE" &

fi

if [ "$MODE" == "subscriber" ]; then
    gnome-terminal -- ./Targets_publisher_subscriber/Targets "$MODE" &
    gnome-terminal -- ./Obstacles_publisher_subscriber/Obstacles "$MODE" &
fi




gnome-terminal -- ./Targets_Generator/Targets_Generator "$MODE" &
gnome-terminal -- ./Obstacle_Generator/Obstacle_Generator "$MODE" &



#if the mdoe is publiser run the publisher




sleep 2
gnome-terminal -- ./WatchDog/WatchDog &
