#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h> 

#include "snake.h"
#include "game.h"

#undef main



int main() 
{ 

	// retutns zero on success else non-zero 
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { 
		printf("error initializing SDL: %s\n", SDL_GetError()); 
	} 
	SDL_Window* win = SDL_CreateWindow("SDL Example", // creates a window 
									SDL_WINDOWPOS_CENTERED, 
									SDL_WINDOWPOS_CENTERED, 
									1000, 1000, 0); 

	// triggers the program that controls 
	// your graphics hardware and sets flags 
	Uint32 render_flags = SDL_RENDERER_ACCELERATED; 

	// creates a renderer to render our images 
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags); 

	// creates a surface to load an image into the main memory 
	SDL_Surface* surface; 

	// please provide a path for your image 
	surface = IMG_Load("C:\\Users\\andre\\Desktop\\urpmeta-uncompressed\\TemplateData\\unity-logo-dark.png"); 

	// loads image to our graphics hardware memory. 
	SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface); 

	// clears main-memory 
	SDL_FreeSurface(surface); 

	// let us control our image position 
	// so that we can move it with our keyboard. 
	SDL_Rect dest; 

	// connects our texture with dest to control position 
	SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h); 

	// adjust height and width of our image box. 
	dest.w /= 6; 
	dest.h /= 6; 

	// sets initial x-position of object 
	dest.x = (1000 - dest.w) / 2; 

	// sets initial y-position of object 
	dest.y = (1000 - dest.h) / 2; 

	// controls annimation loop 
	int close = 0; 

	// speed of box 
	int speed = 300; 

	const int numSnakeBrains = 20;
	std::vector<SnakeBrain> snakeBrains;
	std::vector<SnakeBrain> replaySnakeBrains;
	const int numHiddenLayers = 2;
	const int hiddenLayerSize = 20;
	const int outputLayerSize = 3;
	int numSquares = 30;

	for (int i = 0; i < numSnakeBrains; i++) {
		SnakeBrain brain;
		brain.init(numHiddenLayers, hiddenLayerSize, outputLayerSize);
		snakeBrains.push_back(brain);
	}

	int maxScore = 0;
	int minScore = 100000;

	for (auto& brain : snakeBrains) {
		Game game(numSquares, numSquares);
		auto score = game.play(&brain);
		if (score > maxScore) {
			maxScore = score;
			replaySnakeBrains.push_back(brain.clone());
		}
		minScore = std::min(minScore, score);
	}


	// annimation loop 
	while (!close) {
		Snake snake(&replaySnakeBrains.front(), Vec2i(numSquares / 2, numSquares / 2), SnakeDirection::Down);
		Game game(numSquares, numSquares);
		game.setupBoard(snake);

		auto lastTimeMs = SDL_GetTicks();
		bool roundDone = false;
		// Check for shutdown again..
		while (!close && !roundDone) {

			if (SDL_GetTicks() - lastTimeMs > 100) {
				roundDone = !game.playStep(snake);
				lastTimeMs = SDL_GetTicks();
			}

			SDL_Event event;

			// Events mangement 
			while (SDL_PollEvent(&event)) {
				switch (event.type) {

				case SDL_QUIT:
					// handling of close button 
					close = 1;
					break;

				case SDL_KEYDOWN:
					// keyboard API for key pressed 
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						dest.y -= speed / 30;
						break;
					case SDL_SCANCODE_A:
					case SDL_SCANCODE_LEFT:
						dest.x -= speed / 30;
						break;
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						dest.y += speed / 30;
						break;
					case SDL_SCANCODE_D:
					case SDL_SCANCODE_RIGHT:
						dest.x += speed / 30;
						break;
					case SDL_SCANCODE_ESCAPE:
						close = 1;
						break;
					}
				}
			}

			// right boundary 
			if (dest.x + dest.w > 1000)
				dest.x = 1000 - dest.w;

			// left boundary 
			if (dest.x < 0)
				dest.x = 0;

			// bottom boundary 
			if (dest.y + dest.h > 1000)
				dest.y = 1000 - dest.h;

			// upper boundary 
			if (dest.y < 0)
				dest.y = 0;

			SDL_SetRenderDrawColor(rend, 10, 20, 20, 0);

			// clears the screen 
			SDL_RenderClear(rend);

			int squareSize = 30;
			int boardMarginLeft = 50;
			int boardMarginTop = 10;

			SDL_SetRenderDrawColor(rend, 30, 20, 20, 0);
			for (int col = 0; col < numSquares; col++) {
				for (int row = 0; row < numSquares; row++) {
					SDL_Rect r;
					r.x = boardMarginLeft + col * squareSize;
					r.y = boardMarginTop + row * squareSize;
					r.w = squareSize;
					r.h = squareSize;
					SDL_RenderDrawRect(rend, &r);
				}
			}

			SDL_SetRenderDrawColor(rend, 200, 20, 180, 0);
			for (auto& sp : snake.body) {
				SDL_Rect r;
				r.x = boardMarginLeft + sp.x * squareSize;
				r.y = boardMarginTop + sp.y * squareSize;
				r.w = squareSize;
				r.h = squareSize;
				SDL_RenderFillRect(rend, &r);
			}

			SDL_SetRenderDrawColor(rend, 200, 130, 100, 0);
			SDL_Rect r;
			auto p = game.getFoodPosition();
			r.x = boardMarginLeft + p.x * squareSize;
			r.y = boardMarginTop + p.y * squareSize;
			r.w = squareSize;
			r.h = squareSize;
			SDL_RenderFillRect(rend, &r);

			SDL_RenderCopy(rend, tex, NULL, &dest);

			// triggers the double buffers 
			// for multiple rendering 
			SDL_RenderPresent(rend);

			// calculates to 60 fps 
			SDL_Delay(1000 / 60);
		}
	} 

	// destroy texture 
	SDL_DestroyTexture(tex); 

	// destroy renderer 
	SDL_DestroyRenderer(rend); 

	// destroy window 
	SDL_DestroyWindow(win); 
	return 0; 
} 