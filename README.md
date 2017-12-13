## Build

In order to build the project, please run the following commands:

```
mkdir build
cd build
cmake ..
make
```

## Execution

Run these commands:

```
cd build/bin
./boids
```


## Notes about the project

The project implements a flock simulation that loads multiple independent agents in
a scene. Agents exhibit flocking behavior by using  flock rules similar to the ones described
by Craig Reynolds in: http://www.cs.toronto.edu/~dt/siggraph97-course/cwr87/.

## Controls

1. Adding boid to scene: press key ‘Q’ and position the cursor in the window.
2. Adding object to scene: press key ‘R’ and position the cursor in the window.
3. Rotational camera controls:
    1. Rotating the camera: left-click the mouse and drag.
    2. Zooming in/out: right-click the mouse and drag up/down.
4. The description of other camera controls can be found in https://www.cs.utexas.edu/~theshark/courses/cs354/assignments/assignment_3.html.

## Acknowledgement 

The initial files are based on the Menger's sponge project written by
Mario Fuentes for 2018 Fall Graphics Course.

The following video shows the simulation in action: https://vimeo.com/247041482.
