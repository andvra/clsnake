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
	Game(SnakeBrain* brain, int tBoardWidth, int tBoardHeight, int roundTime = 500) {
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
		const int timeUnitScore = 5;
		const int foodScore = 150;
		const int foodTimeAdd = 500;
		const int crashPenalty = 100;
		const int timeOutPenalty = 100;
		auto measurements = measure(snake);
		auto brainOutput = snake->think(measurements);
		snake->updateDirection(brainOutput);
		bool didCrash = isCrash(snake, snake->nextPosition());
		snake->move();
		if (didCrash) {
			snake->isAlive = false;
			score -= crashPenalty;
			return false;
		}
		if (snake->position == foodPosition) {
			std::cout << "Snake ate food at position " << foodPosition.toString() << std::endl;
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

private:
	int boardWidth;
	int boardHeight;
	int score;
	int timeLeft;
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

		auto measurements = std::vector<float>(24, 0);

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
		int maxIters = 10000;

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