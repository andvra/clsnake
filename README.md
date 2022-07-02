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
- In our solution, each gene is represented by a float number, since we decide to make decisions based on inference through a simple articical neural network