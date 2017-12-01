## Build

In order to build the project, please run the following commands:

```
mkdir build
cd build
cmake ..
make -j8
```

## Execution

Code execution commands:

```
cd build/bin
./menger
```


## Notes about the project

The project implements a flock simulation that loads multiple independent agents in
a scene. Agents exhibit flocking behavior by using the simulated flock rules described
by Craig Reynolds in: http://www.cs.toronto.edu/~dt/siggraph97-course/cwr87/.

# Acknowledgement 

The initial files are based on the Menger's sponge project written by
Mario Fuentes for 2018 Fall Graphics Course.

The project's report is located in: https://github.com/msf1013/graphics-final-project.
