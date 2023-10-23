# MeshDecimation


## How to Run the Code
Run the following commands to download the necessary submodules:

    git submodule add https://github.com/darrenmothersele/embed-resource.git lib/embed-resource
    git submodule add https://github.com/wjakob/nanogui.git lib/nanogui --recursive

Then, run the following commands to compile the code:

    mkdir build
    cd build
    cmake ..
    make

Then, cd back to the main folder and run the applicaion:

    cd ..
    ./mcaq