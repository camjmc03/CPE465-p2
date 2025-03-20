# executable created by make
EXECUTABLE="build/of_controller"

# ensure dependencies are installed
echo "Checking dependencies..."
# cmake
if ! [ -x "$(command -v cmake)" ]; then
    echo "Error: cmake is not installed." >&2
    exit 1
fi
# make
if ! [ -x "$(command -v make)" ]; then
    echo "Error: make is not installed." >&2
    exit 1
fi
# g++
if ! [ -x "$(command -v g++)" ]; then
    echo "Error: g++ is not installed." >&2
    exit 1
fi
# git
if ! [ -x "$(command -v git)" ]; then
    echo "Error: git is not installed." >&2
    exit 1
fi

# Parse command-line arguments
FORCE_REBUILD=false
while getopts "f" opt; do
    case $opt in
        f)
            FORCE_REBUILD=true
            ;;
        *)
            echo "Usage: $0 [-f]" >&2
            exit 1
            ;;
    esac
done

# Remove build directory if -f flag is provided
if [ "$FORCE_REBUILD" = true ]; then
    echo "Removing existing build directory..."
    rm -rf build
fi

# Create build directory if it doesn't exist
if [ ! -d build ]; then
    mkdir build
fi
cd build
echo "Building..."
echo "Running cmake"
cmake ..
echo "Running make"
make -j$(sysctl -n hw.ncpu)
cd ..
echo "creating run.sh"
echo "#!/bin/bash" > run.sh
echo "./$EXECUTABLE \"\$@\"" >> run.sh
chmod +x run.sh
echo "Build complete"
echo "----------------------------------------"
echo "To run the program, execute ./run.sh"
echo "----------------------------------------"