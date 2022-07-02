## Prerequisites

To run this, the following packages need to be installed:

SDL2, SDL2_image, SDL2_ttf


To install with vcpkg (NB make sure to install with your prefered triplet)

```
vcpkg install sdl2 sdl2-ttf sdl2-image
```


TODO: 
- Only move left/right, rotate board 90 deg to work with this
- Perceptrons, how does it work?
- Evolution, how does it work?
- "We can see the problem as encoded with a number of float values corresponding to (input+hidden layers) genes
- One snake brain = "chromosome"
- In our solution, each gene is represented by a float number, since we decide to make decisions based on inference through a simple feed-forward network
- Boxes in the graphics, what are these? Food, wall, snake body

## Introduction

This is an example of how genetic algorithms can be used to play a game, Snake. The snake brain is modelled as a feed-forward network, making decisions whether to turn or keep going forward.

The algorithm can easily be adjusted for oter applications.

## Thinking

Before each move, some measurements are made and fed into the snake brain. The measurements are made in eight directions, with 45 degrees between, centered around the snake head. For each direction, the snake measures the distance to a wall, whether or not there is food in that direction and if there is collision with the snakes' tail. With three measurements in each direction, there is a total of 24 measurements made. The order of the measurements are made in relation to the direction of the snake. First measurement is made to the bottom left of the snake, the next to the left, third to the top left and so on. Since the board is square, this is possible.

These measurements are feed into a feed-forward network with three outputs. The outputs decides the next move: forward, left or right, all relative to the current direction of the snake.

## Evolution

Crossover, mutation, chromosome, gene..

## Graphics

The points of measurements are rendered on the game board.