#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h> 

#include "snake.h"
#include "game.h"
#include "evolution.h"

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
									1200, 1000, 0); 

	// triggers the program that controls 
	// your graphics hardware and sets flags 
	Uint32 render_flags = SDL_RENDERER_ACCELERATED; 

	// creates a renderer to render our images 
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags); 

	int close = 0; 
	const int numSnakeBrains = 500;
	std::vector<SnakeBrain> snakeBrains;
	std::vector<SnakeBrain> replaySnakeBrains;
	const int numHiddenLayers = 2;
	const int hiddenLayerSize = 20;
	const int outputLayerSize = 3;
	int numSquares = 30;
	const int squareSize = 30;
	const int boardMarginLeft = 150;
	const int boardMarginTop = 50;

	for (int i = 0; i < numSnakeBrains; i++) {
		SnakeBrain brain;
		brain.init(numHiddenLayers, hiddenLayerSize, outputLayerSize);
		snakeBrains.push_back(brain);
	}

	const int numGenerations = 5;
	for (int gen = 0; gen < numGenerations; gen++) {
		std::vector<std::tuple<int, SnakeBrain*>> brainsWithScore;

		for (auto& brain : snakeBrains) {
			Game game(&brain, numSquares, numSquares);
			auto score = game.play(&brain);
			//std::cout << "Snake got score " << score << std::endl;
			brainsWithScore.push_back(std::tuple<int, SnakeBrain*>(score, &brain));
		}

		std::sort(brainsWithScore.begin(), brainsWithScore.end(), [](std::tuple<int, SnakeBrain*> a, std::tuple<int, SnakeBrain*>b) {return std::get<0>(a) > std::get<0>(b); });

		auto bestBrainInGeneration = std::get<1>(brainsWithScore.front());

		std::cout << "Best brain of gen #" << std::to_string(gen + 1) << ": " << std::get<0>(brainsWithScore.front()) << std::endl;

		replaySnakeBrains.push_back(bestBrainInGeneration->clone());

		// Time to evolve!
		if (gen < numGenerations - 1) {
			std::vector<SnakeBrain> newSnakeBrains;
			// Keep the best brain of each generation
			newSnakeBrains.push_back(bestBrainInGeneration->clone());
			// TODO: Think of good criteria for a parent
			int numParents = numSnakeBrains / 20;
			std::vector<SnakeBrain*> parents;
			parents.reserve(numParents);
			for (int i = 0; i < numParents; i++) {
				parents.push_back(std::get<1>(brainsWithScore[i]));
			}
			// Start at one, since we already added the currently best brain to the vector
			for (int childIdx = 1; childIdx < numSnakeBrains; childIdx++) {
				auto parentIdx1 = getRandomInt(0, numParents - 1);
				auto parentIdx2 = getRandomInt(0, numParents - 1);
				SnakeBrain child = ClSnake::makeChild(parents[parentIdx1], parents[parentIdx2]);
				newSnakeBrains.push_back(child);
			}
			snakeBrains = newSnakeBrains;
		}
	}

	Game* game = nullptr;

	auto lastTimeMs = SDL_GetTicks();
	const float freezeTimeMs = 2000.0f;
	auto freezeUntil = 0.0f;
	auto waitingForRestart = true;

	auto boardPosToRenderRect = [&squareSize, &boardMarginLeft, &boardMarginTop](Vec2i p) {
		SDL_Rect r;
		r.x = boardMarginLeft + p.x * squareSize;
		r.y = boardMarginTop + p.y * squareSize;
		r.w = squareSize;
		r.h = squareSize;
		return r;
	};

	while (!close) {
		if (waitingForRestart) {
			if (SDL_GetTicks() > freezeUntil) {
				if (game != nullptr) {
					delete game;
				}
				game = new Game(&replaySnakeBrains.back(), numSquares, numSquares, 150);
				waitingForRestart = false;
			}
		}
		else {
			if (SDL_GetTicks() - lastTimeMs > 100) {
				auto roundDone = !game->playStep();
				if (roundDone) {
					waitingForRestart = true;
					freezeUntil = SDL_GetTicks() + freezeTimeMs;
				}
				lastTimeMs = SDL_GetTicks();
			}
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
				case SDL_SCANCODE_ESCAPE:
					close = 1;
					break;
				}
			}
		}

		SDL_SetRenderDrawColor(rend, 10, 20, 20, 0);

		SDL_RenderClear(rend);
		SDL_SetRenderDrawColor(rend, 130, 120, 120, 0);
		for (int col = 0; col < numSquares; col++) {
			for (int row = 0; row < numSquares; row++) {
				auto r = boardPosToRenderRect(Vec2i(col, row));
				SDL_RenderDrawRect(rend, &r);
			}
		}

		if (game != nullptr) {
			if (waitingForRestart) {
				float t = 1.0f - (freezeUntil - SDL_GetTicks()) / freezeTimeMs;
				int r = std::lerp(200, 0, t);
				int g = std::lerp(20, 20, t);
				int b = std::lerp(180, 180, t);
				SDL_SetRenderDrawColor(rend, r, g, b, 0);
			}
			else {
				SDL_SetRenderDrawColor(rend, 200, 20, 180, 0);
			}

			for (auto& sp : game->snake->body) {
				SDL_Rect r = boardPosToRenderRect(sp);
				SDL_RenderFillRect(rend, &r);
			}

			SDL_SetRenderDrawColor(rend, 200, 130, 100, 0);
			SDL_Rect r = boardPosToRenderRect(game->getFoodPosition());
			SDL_RenderFillRect(rend, &r);
		}

		SDL_RenderPresent(rend);

		// calculates to 60 fps 
		SDL_Delay(1000 / 60);
	}


	// destroy renderer 
	SDL_DestroyRenderer(rend); 

	// destroy window 
	SDL_DestroyWindow(win); 
	return 0; 
} 