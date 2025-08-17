# MAKE SURE TO SOURCE THE FILE IN THE ROOT OF THE REPO`
This contains 3 implementations of Smith-Waterman. There are scripts to automate simulation/testbenching

EVERYTHING is run from the build directory

# Automated Csim/Csynth/Cosim:
1. `mkdir build`
2. `cd build`
3. `../auto_scripts/cosim_X` where X is the implementation + testbench data that is to be run

# Automated latency retrieval:
1. `cd build`
2. `../auto_scripts/getLatench_X` where X is the implementation + testbench data that is to be retrieved


export PATH=/usr/local/cuda/bin:$PATH