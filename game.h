#pragma once

#include "snake.h"

struct MeasureSquares {
	std::vector<Vec2i> body;
	std::vector<Vec2i> food;
	std::vector<Vec2i> wall;

	void clear();
};

class Game {
public:
	// Round time is important for training: keep it pretty high so the snake can learn!
	Game(SnakeBrain* brain, int tBoardWidth, int tBoardHeight, int roundTime = 300);

	~Game();

	bool isCrash(Snake* snake, Vec2i pt);

	// Return true if we should go on
	// This is used for rendering the snake. Make sure to call setupBoard() before calling this
	bool playStep(bool isManual, SnakeMove* snakeMove = nullptr, MeasureSquares* measureSquares = nullptr);

	Vec2i getFoodPosition();

	// Returns the score
	// This is used for fast play (eg. during training)
	void play();

	int fitness();

	Snake* snake = nullptr;
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
	// 2 3 4
	// 1   5
	// 0 7 6
	//
	// We always want the first value to be relative the direction of the snake, since the thinking should be
	//	rotation invariant, so in the second step we start at an index in the array depending on the direction of the snake.
	// The example above is for going up. If we for instance go right instead, we just start at index 2, fetching all 8
	//	groups of measurements going around the circular buffer. So we get
	//
	// 0 1 2
	// 7   3
	// 6 5 4
	//
	// Returns normalized measurements
	std::vector<float> measure(Snake* snake, MeasureSquares* measureSquares);
	Vec2i generateFoodPosition();
};