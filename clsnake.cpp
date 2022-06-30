#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h> 
#include <SDL2/SDL_ttf.h>

#include <format>
#include <thread>

#include "snake.h"
#include "game.h"
#include "evolution.h"
#include "config.h"

// Needed for SDL2
#undef main

TTF_Font* loadFont(const char* fontPath, int fontSize) {
	if (TTF_Init() < 0) {
		std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
	}

	TTF_Font* font = TTF_OpenFont(fontPath, fontSize);

	if (!font) {
		std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
		return nullptr;
	}

	return font;
}

int main() 
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { 
		std::cout << "Error initializing SDL: %" << SDL_GetError() << std::endl;
	} 

	SDL_Window* win = SDL_CreateWindow("Genetic Snake",
									SDL_WINDOWPOS_CENTERED, 
									SDL_WINDOWPOS_CENTERED, 
									SnakeConfiguration::Graphics::windowWidth,
									SnakeConfiguration::Graphics::windowHeight,
									0); 

	Uint32 render_flags = SDL_RENDERER_ACCELERATED; 

	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);


	const auto fontPath = "C:\\Windows\\Fonts\\arial.ttf";
	const auto fontSize = 24;

	auto font = loadFont(fontPath, fontSize);

	if (font == nullptr) {
		return 1;
	}

	SDL_Color textColor = { 200, 190, 205 };
	int close = 0;

	std::vector<SnakeBrain> replaySnakeBrains;
	int useSnakeBrainGeneration;

	ClSnake::evolve(replaySnakeBrains, useSnakeBrainGeneration);

	Game* game = nullptr;

	auto lastTimeMs = SDL_GetTicks64();
	auto freezeUntil = 0.0f;
	auto waitingForRestart = true;

	auto boardPosToRenderRect = [](Vec2i p) {
		SDL_Rect r;
		r.x = SnakeConfiguration::Graphics::boardMarginLeft + p.x * SnakeConfiguration::Graphics::squareSize;
		r.y = SnakeConfiguration::Graphics::boardMarginTop + p.y * SnakeConfiguration::Graphics::squareSize;
		r.w = SnakeConfiguration::Graphics::squareSize;
		r.h = SnakeConfiguration::Graphics::squareSize;
		return r;
	};

	bool readSnakeKeyDown = true;
	SnakeMove snakeMove = SnakeMove::Forward;
	MeasureSquares measureSquares;

	while (!close) {
		if (waitingForRestart) {
			if (SDL_GetTicks64() > freezeUntil) {
				if (game != nullptr) {
					delete game;
				}
				game = new Game(&replaySnakeBrains[useSnakeBrainGeneration], SnakeConfiguration::Game::numSquares, SnakeConfiguration::Game::numSquares, SnakeConfiguration::Game::roundTime);
				waitingForRestart = false;
			}
		}
		else {
			if (SDL_GetTicks64() - lastTimeMs > 100) {
				bool roundDone = false;
				measureSquares.clear();
				if (SnakeConfiguration::Game::manualPlay) {
					roundDone = !game->playStep(true, &snakeMove, &measureSquares);
				}
				else {
					roundDone = !game->playStep(false, nullptr, &measureSquares);
				}
				snakeMove = SnakeMove::Forward;// Reset when it has been sent - now we wait for a new keypress
				if (roundDone) {
					waitingForRestart = true;
					freezeUntil = SDL_GetTicks64() + SnakeConfiguration::Graphics::freezeTimeMs;
				}
				lastTimeMs = SDL_GetTicks64();
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
		for (int col = 0; col < SnakeConfiguration::Game::numSquares; col++) {
			for (int row = 0; row < SnakeConfiguration::Game::numSquares; row++) {
				auto r = boardPosToRenderRect(Vec2i(col, row));
				SDL_RenderDrawRect(rend, &r);
			}
		}

		if (game != nullptr) {
			if (waitingForRestart) {
				float t = 1.0f - (freezeUntil - SDL_GetTicks64()) / SnakeConfiguration::Graphics::freezeTimeMs;
				int r = static_cast<int>(std::lerp(200, 0, t));
				int g = static_cast<int>(std::lerp(20, 20, t));
				int b = static_cast<int>(std::lerp(180, 180, t));
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

			SDL_SetRenderDrawColor(rend, 180, 80, 80, 0);
			for (auto& bp : measureSquares.body) {
				r = boardPosToRenderRect(bp);
				SDL_RenderFillRect(rend, &r);
			}
			SDL_SetRenderDrawColor(rend, 80, 80, 250, 0);
			SDL_RenderSetScale(rend, 3.0f, 3.0f);
			for (auto& fp : measureSquares.food) {
				r = boardPosToRenderRect(fp);
				r.x = static_cast<int>(r.x / 3.0f);
				r.y = static_cast<int>(r.y / 3.0f);
				r.w = static_cast<int>(r.w / 2.6f);
				r.h = static_cast<int>(r.h / 2.5f);
				SDL_RenderDrawRect(rend, &r);
			}
			SDL_RenderSetScale(rend, 1, 1);
			SDL_SetRenderDrawColor(rend, 80, 180, 80, 0);
			for (auto& wp : measureSquares.wall) {
				r = boardPosToRenderRect(wp);
				SDL_RenderDrawRect(rend, &r);
			}

			SDL_Surface* text = nullptr;
			SDL_Texture* text_texture;

			text = TTF_RenderText_Solid(font, std::format("Gen: {} Pos: {} Score: {} Time left: {}", useSnakeBrainGeneration + 1, game->snake->position.toString(), game->score, game->timeLeft).c_str(), textColor);

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