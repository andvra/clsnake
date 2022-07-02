#pragma once

#include "snake.h"
#include "config.h"

namespace ClSnake {

	// Performs uniform crossover from two parents
	SnakeBrain crossOver(SnakeBrain* parent1, SnakeBrain* parent2);
	// Probability for mutation, on range 0 - 1
	void mutate(SnakeBrain* brain, float probability);
	// Probability for mutation, on range 0 - 1
	SnakeBrain makeChild(SnakeBrain* parent1, SnakeBrain* parent2, float mutationProbability);
	void evolve(std::vector<SnakeBrain>& replaySnakeBrains, int& useSnakeBrainGeneration);
}