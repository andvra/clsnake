#pragma once

#include "snake.h"

struct MeasureSquares {
	std::vector<Vec2i> body;
	std::vector<Vec2i> food;
	std::vector<Vec2i> wall;

	void clear() {
		body.clear();
		food.clear();
		wall.clear();
	}
};

class Game {
public:
	// Round time is important for training: keep it pretty high so the snake can learn!
	Game(SnakeBrain* brain, int tBoardWidth, int tBoardHeight, int roundTime = 300) {
		boardWidth = tBoardWidth;
		boardHeight = tBoardHeight;
		startingPosition = Vec2i(boardWidth / 2 - 6, boardHeight / 2 + 9);
		snake = new Snake(brain, startingPosition);
		totalTimeLeft = maxTime;
		timeLeft = roundTime;
		foodPosition = generateFoodPosition();
	}

	~Game() {
		if (snake != nullptr) {
			delete snake;
		}
	}

	bool isCrash(Snake* snake, Vec2i pt) {
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

	// Return true if we should go on
	// This is used for rendering the snake. Make sure to call setupBoard() before calling this
	bool playStep(bool isManual, SnakeMove* snakeMove = nullptr, MeasureSquares* measureSquares = nullptr ) {
		const int timeUnitScore = 1;
		const int foodScore = 1500;
		const int foodTimeAdd = 100;
		const int crashPenalty = 500;
		const int timeOutPenalty = 500;

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
			score -= crashPenalty;
			return false;
		}
		if (snake->position == foodPosition) {
			snake->ateLastMove = true;
			foodPosition = generateFoodPosition();
			score += foodScore;
			timeLeft += foodTimeAdd;
		}
		totalTimeLeft--;
		timeLeft--;
		score += timeUnitScore;

		if (totalTimeLeft <= 0 || timeLeft <= 0) {
			score -= timeOutPenalty;
			return false;
		}

		return true;
	}

	Vec2i getFoodPosition() {
		return foodPosition;
	}

	// Returns the score
	// This is used for fast play (eg. during training)
	int play(SnakeBrain* snakeBrain) {
		auto done = false;

		while (!done) {
			done = !playStep(false);
		}

		return score;
	}

	Snake* snake = nullptr;
	int score;
	int timeLeft;
private:
	int boardWidth;
	int boardHeight;
	// To make sure to stop the game if the snake is "too good"
	const int maxTime = 50000;
	int totalTimeLeft;
	Vec2i foodPosition;
	Vec2i startingPosition;

	// First, measure from the squares around starting with the bottom left, going to the upper left and then around.
	//
	// 3 4 5
	// 2   6
	// 1 8 7
	//
	// We always want the first value to be relative the direction of the snake, since the thinking should be
	//	rotation invariant, so in the second step we start at an index in the array depending on the direction of the snake.
	// The example above is for going up. If we for instance go right instead, we just start at index 2, fetching all 8
	//	groups of measurements going around the circular buffer. So we get
	//
	// 1 2 3
	// 8   4
	// 7 6 5
	//
	// Returns normalized measurements
	std::vector<float> measure(Snake* snake, MeasureSquares* measureSquares) {

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

	Vec2i generateFoodPosition() {
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
};