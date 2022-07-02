#include "game.h"

#pragma once

#include "snake.h"
#include "config.h"


void MeasureSquares::clear() {
	body.clear();
	food.clear();
	wall.clear();
}


Game::Game(SnakeBrain* brain, int tBoardWidth, int tBoardHeight, int roundTime) {
	boardWidth = tBoardWidth;
	boardHeight = tBoardHeight;
	startingPosition = Vec2i(boardWidth / 2 - 6, boardHeight / 2 + 9);
	snake = new Snake(brain, startingPosition);
	totalTimeLeft = maxTime;
	timeLeft = roundTime;
	foodPosition = generateFoodPosition();
}

Game::~Game() {
	if (snake != nullptr) {
		delete snake;
	}
}

bool Game::isCrash(Snake* snake, Vec2i pt) {
	if (pt.x < 0 || pt.x >= boardWidth) {
		return true;
	}
	if (pt.y < 0 || pt.y >= boardHeight) {
		return true;
	}

	auto iter = std::find_if(snake->body.begin(), snake->body.end(), [&pt](Vec2i v) {return v == pt; });

	auto didCrash = (iter != snake->body.end());

	return didCrash;
}

bool Game::playStep(bool isManual, SnakeMove* snakeMove, MeasureSquares* measureSquares) {
	auto measurements = measure(snake, measureSquares);

	if (isManual) {
		if (snakeMove != nullptr) {
			snake->updateDirection(*snakeMove);
		}
	}
	else {
		auto brainOutput = snake->think(measurements);
		snake->updateDirection(brainOutput);
	}

	bool didCrash = isCrash(snake, snake->nextPosition());
	snake->move();

	if (didCrash) {
		snake->isAlive = false;
		score -= SnakeConfiguration::Game::crashPenalty;
		return false;
	}
	if (snake->position == foodPosition) {
		snake->ateLastMove = true;
		foodPosition = generateFoodPosition();
		score += SnakeConfiguration::Game::foodScore;
		timeLeft += SnakeConfiguration::Game::foodTimeAdd;
	}
	totalTimeLeft--;
	timeLeft--;
	score += SnakeConfiguration::Game::timeUnitScore;

	if (totalTimeLeft <= 0 || timeLeft <= 0) {
		score -= SnakeConfiguration::Game::timeOutPenalty;
		return false;
	}

	return true;
}

Vec2i Game::getFoodPosition() {
	return foodPosition;
}

int Game::play() {
	auto done = false;

	while (!done) {
		done = !playStep(false);
	}

	return score;
}

std::vector<float> Game::measure(Snake* snake, MeasureSquares* measureSquares) {

	const int numMeasurements = 24;

	std::vector<Vec2i> posDeltas = {
		Vec2i(-1, -1),
		Vec2i(-1, 0),
		Vec2i(-1, 1),
		Vec2i(0,  1),
		Vec2i(1,  1),
		Vec2i(1,  0),
		Vec2i(1,  -1),
		Vec2i(0,  -1)
	};

	int indexOffset;

	switch (snake->direction) {
	case SnakeDirection::Up:	indexOffset = 0; break;
	case SnakeDirection::Right:	indexOffset = 2; break;
	case SnakeDirection::Down:	indexOffset = 4; break;
	case SnakeDirection::Left:	indexOffset = 6; break;
	}

	// TODO: Update what measurements we make. First, without considering where the body is. This is done by:
	//	Don't add body when eating. 
	//	Measure: angle to food, (manhattan) distance to food, distance to wall (left, right, up down). 
	// Then, add body. Measure left, right, forward. Think of a better measurement - body is the trickiest!

	auto measurements = std::vector<float>(numMeasurements, 0);

	// 8 squares, 3 measurements each
	for (int idxDir = 0; idxDir < 8; idxDir++) {
		// Make sure to use the right delta based on current snake position
		Vec2i deltaPos = posDeltas[(idxDir + indexOffset) % 8];
		Vec2i curPos = snake->position + deltaPos;
		int distance = 1;
		float food = 0;
		float body = 0;
		float wall = 0;
		while (curPos.x >= 0 && curPos.x < boardWidth && curPos.y >= 0 && curPos.y < boardHeight) {
			if (curPos == foodPosition) {
				food = 1.0f;
				if (measureSquares != nullptr) {
					measureSquares->food.push_back(curPos);
				}
			}
			// No need to check if there's already a crash
			if (body == 0.0f) {
				for (auto& bp : snake->body) {
					if (curPos == bp) {
						body = 1.0f;
						if (measureSquares != nullptr) {
							measureSquares->body.push_back(curPos);
						}
						break;
					}
				}
			}
			distance++;
			curPos = curPos + deltaPos;
		}
		if (measureSquares != nullptr) {
			measureSquares->wall.push_back(curPos);
		}
		wall = 1.0f / distance;

		// Three values, so multiply by three
		int idxStart = idxDir * 3;
		measurements[idxStart + 0] = wall;
		measurements[idxStart + 1] = food;
		measurements[idxStart + 2] = body;
	}

	return measurements;
}

Vec2i Game::generateFoodPosition() {
	const int maxIters = 10'000;

	while (maxIters > 0) {
		int x = getRandomInt(0, boardWidth);
		int y = getRandomInt(0, boardHeight);
		Vec2i pt(x, y);
		if (!isCrash(snake, pt)) {
			return pt;
		}
	}

	// This is an error; we should be able to get a random position
	return Vec2i(0, 0);
}