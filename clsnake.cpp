#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h> 
#include <SDL2/SDL_ttf.h>

#include "snake.h"
#include "game.h"
#include "evolution.h"
#include <format>
#include <thread>

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

	if (TTF_Init() < 0) {
		std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
	}
	TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);

	if (!font) {
		std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
	}

	SDL_Color color = { 200, 190, 205 };

	int close = 0; 
	const int numSnakeBrains = 1000;
	std::vector<SnakeBrain> snakeBrains;
	std::vector<SnakeBrain> replaySnakeBrains;
	const int numHiddenLayers = 2;
	const int hiddenLayerSize = 20;
	const int outputLayerSize = 3;
	int numSquares = 20;
	const int squareSize = 20;
	const int boardMarginLeft = 150;
	const int boardMarginTop = 50;
	const int numInputs = 24;	// Nake sure it corresponds with measure()
	const float mutationProbability = 0.05;	// On range 0 - 1

	for (int i = 0; i < numSnakeBrains; i++) {
		SnakeBrain brain(numInputs, numHiddenLayers, hiddenLayerSize, outputLayerSize);
		snakeBrains.push_back(brain);
	}

	const int numGenerations = 15;
	std::cout << std::format("Gen\tMax score\tTime (s)") << std::endl;
	for (int gen = 0; gen < numGenerations; gen++) {
		auto genStartTime = SDL_GetTicks();
		std::vector<std::tuple<int, SnakeBrain*>> brainsWithScore(snakeBrains.size(), std::tuple<int, SnakeBrain*>(0, 0));

		const int numThreads = 12;
		for (int brainOffset = 0; brainOffset < snakeBrains.size(); brainOffset += numThreads) {
			std::vector<std::thread> threads;
			for (int idxBrain = brainOffset; idxBrain < std::min(numSnakeBrains, brainOffset + numThreads); idxBrain++) {
				SnakeBrain& brain = snakeBrains[idxBrain];
				threads.push_back(std::thread([&brainsWithScore, &brain, &numSquares, idxBrain]() {
					Game game(&brain, numSquares, numSquares);
					auto score = game.play(&brain);
					brainsWithScore[idxBrain] = std::tuple<int, SnakeBrain*>(score, &brain);
				}));
			}
			for (auto& t : threads) {
				t.join();
			}
		}

		std::sort(brainsWithScore.begin(), brainsWithScore.end(), [](std::tuple<int, SnakeBrain*> a, std::tuple<int, SnakeBrain*>b) {return std::get<0>(a) > std::get<0>(b); });

		auto bestBrainInGeneration = std::get<1>(brainsWithScore.front());

		auto maxScore = std::get<0>(brainsWithScore.front());
		auto genTimeS = (SDL_GetTicks() - genStartTime) / 1000.0f;
		std::cout << std::format("{}\t{}\t\t{}", gen + 1, maxScore, genTimeS) << std::endl;

		replaySnakeBrains.push_back(bestBrainInGeneration->clone());

		// Time to evolve!
		if (gen < numGenerations - 1) {
			std::vector<SnakeBrain> newSnakeBrains;
			// Keep the best brain of each generation
			newSnakeBrains.push_back(bestBrainInGeneration->clone());
			// TODO: Think of good criteria for a parent
			int numParents = std::max(2, numSnakeBrains / 20);
			std::vector<SnakeBrain*> parents;
			parents.reserve(numParents);
			for (int i = 0; i < numParents; i++) {
				parents.push_back(std::get<1>(brainsWithScore[i]));
			}
			// Start at one, since we already added the currently best brain to the vector
			for (int childIdx = 1; childIdx < numSnakeBrains; childIdx++) {
				auto parentIdx1 = getRandomInt(0, numParents - 1);
				auto parentIdx2 = getRandomInt(0, numParents - 1);
				SnakeBrain child = ClSnake::makeChild(parents[parentIdx1], parents[parentIdx2], mutationProbability);
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

	bool readSnakeKeyDown = true;
	SnakeMove snakeMove = SnakeMove::Forward;
	MeasureSquares measureSquares;

	bool manualPlay = false;

	int useSnakeBrainGeneration = replaySnakeBrains.size() - 1;

	while (!close) {
		if (waitingForRestart) {
			if (SDL_GetTicks() > freezeUntil) {
				if (game != nullptr) {
					delete game;
				}
				game = new Game(&replaySnakeBrains[useSnakeBrainGeneration], numSquares, numSquares, 150);
				waitingForRestart = false;
			}
		}
		else {
			if (SDL_GetTicks() - lastTimeMs > 100) {
				bool roundDone = false;
				if (manualPlay) {
					measureSquares.clear();
					roundDone = !game->playStep(true, &snakeMove, &measureSquares);
				}
				else {
					roundDone = !game->playStep(false);
				}
				snakeMove = SnakeMove::Forward;// Reset when it has been sent - now we wait for a new keypress
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
				case SDL_SCANCODE_LEFT:
					snakeMove = SnakeMove::Left;
					readSnakeKeyDown = false;
					break;
				case SDL_SCANCODE_RIGHT:
					snakeMove = SnakeMove::Right;
					readSnakeKeyDown = false;
					break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_RIGHT:
					readSnakeKeyDown = true;
					break;
				case SDL_SCANCODE_G:
					waitingForRestart = true;
					useSnakeBrainGeneration = (useSnakeBrainGeneration + 1) % replaySnakeBrains.size();
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

			if (manualPlay) {
				SDL_Rect r;
				SDL_SetRenderDrawColor(rend, 180, 80, 80, 0);
				for (auto& bp : measureSquares.body) {
					r = boardPosToRenderRect(bp);
					SDL_RenderFillRect(rend, &r);
				}
				SDL_SetRenderDrawColor(rend, 80, 80, 180, 0);
				for (auto& fp : measureSquares.food) {
					r = boardPosToRenderRect(fp);
					SDL_RenderDrawRect(rend, &r);
				}
				SDL_SetRenderDrawColor(rend, 80, 180, 80, 0);
				for (auto& wp : measureSquares.wall) {
					r = boardPosToRenderRect(wp);
					SDL_RenderDrawRect(rend, &r);
				}
			}

			SDL_Surface* text = nullptr;
			SDL_Texture* text_texture;

			text = TTF_RenderText_Solid(font, std::format("Gen: {} Pos: {} Score: {} Time left: {}", useSnakeBrainGeneration + 1, game->snake->position.toString(), game->score, game->timeLeft).c_str(), color);

			if (text == nullptr) {
				std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
			}
			else {
				text_texture = SDL_CreateTextureFromSurface(rend, text);

				SDL_Rect dest = { 0, 0, text->w, text->h };

				SDL_RenderCopy(rend, text_texture, nullptr, &dest);
				SDL_DestroyTexture(text_texture);
				SDL_FreeSurface(text);
			}
		}

		SDL_RenderPresent(rend);

		// calculates to 60 fps 
		SDL_Delay(1000 / 60);
	}

	TTF_CloseFont(font);
	TTF_Quit();

	// destroy renderer 
	SDL_DestroyRenderer(rend); 

	// destroy window 
	SDL_DestroyWindow(win); 
	return 0; 
} 