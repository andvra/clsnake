#pragma once

#include "snake.h"

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

		return (iter != snake->body.end());
	}

	// Return true if we should go on
	// This is used for rendering the snake. Make sure to call setupBoard() before calling this
	bool playStep() {
		const int timeUnitScore = 5;
		const int foodScore = 100;
		const int foodTimeAdd = 500;
		auto measurements = measure(snake);
		auto brainOutput = snake->think(measurements);
		snake->updateDirection(brainOutput);
		bool didCrash = isCrash(snake, snake->nextPosition());
		snake->move();
		if (didCrash) {
			snake->isAlive = false;
			score += snake->position.manhattanDistance(startingPosition) * 10;
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
			done = !playStep();
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
	std::vector<float> measure(Snake* snake) {

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
		float maxDistanceWall = std::ceil(std::min(boardWidth, boardHeight) / 2.0f);
		float maxDistanceFood = static_cast<float>(Vec2i(0, 0).manhattanDistance(Vec2i(boardWidth, boardHeight)));
		float maxDistanceBody = 2.0f;	// TODO: Maybe we should measure "distance clear infront of body" instead?

		// 8 squares, 3 measurements each
		for (int idxDir = 0; idxDir < 8; idxDir++) {
			Vec2i deltaPos = posDeltas[idxDir];
			Vec2i curPos = snake->position + deltaPos;
			int distance = 1;
			float food = 0;
			float body = 0;
			float wall = 0;
			while (curPos.x >= 0 && curPos.x < boardWidth && curPos.y >= 0 && curPos.y < boardHeight) {
				if (curPos == foodPosition) {
					food = 1.0f;
				}
				for (auto& bp : snake->body) {
					if (curPos == bp) {
						body = 1.0f;
						break;
					}
				}
				distance++;
				curPos = curPos + deltaPos;
			}
			wall = 1.0f / distance;

			int idxStart = ((idxDir + indexOffset) * 3) % 24;
			measurements[idxStart + 0] = wall;
			measurements[idxStart + 1] = food;
			measurements[idxStart + 2] = body;

			//Vec2i p = snake->position + posDeltas[i];
			//int wallDistanceX = std::min(p.x + 1, boardWidth - p.x);
			//int wallDistanceY = std::min(p.y + 1, boardHeight - p.y);
			//int wallDistance = std::min(wallDistanceX, wallDistanceY);
			//int foodDistance = p.manhattanDistance(foodPosition);
			//int bodyDistance = 10000;
			//for (auto& bp : snake->body) {
			//	bodyDistance = std::min(bodyDistance, p.manhattanDistance(bp));
			//}
			//// There are three measurements per square
			//int idxStart = ((i + indexOffset) * 3)%24;

			//measurements[idxStart + 0] = wallDistance / maxDistanceWall;
			//measurements[idxStart + 1] = foodDistance / maxDistanceFood;
			//measurements[idxStart + 2] = bodyDistance / maxDistanceBody;

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