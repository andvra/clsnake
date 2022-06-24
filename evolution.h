#pragma once

#include "snake.h"

namespace ClSnake {

	SnakeBrain crossOver(SnakeBrain* parent1, SnakeBrain* parent2) {
		std::vector<SnakePerceptron> perceptrons;

		// TODO: For know, we copy both weight and bias at once.
		//	Should we split it into two separate steps?
		for (int i = 0; i < parent1->perceptrons.size(); i++) {
			SnakeBrain* useBrain = (getRandomInt(0, 1000) > 500) ? parent1 : parent2;
			perceptrons.push_back(useBrain->perceptrons[i].clone());
		}

		return SnakeBrain(perceptrons, parent1->numInputs, parent1->numHiddenLayers, parent1->hiddenLayerSize, parent1->outputLayerSize);
	}

	// Probability for mutation, on range 0 - 1
	void mutate(SnakeBrain* brain, float probability) {

		// TODO: Should be mutate bias as well, as we do currently?
		for (auto& p : brain->perceptrons) {
			for (int i = 0; i < p.w.size(); i++) {
				if (getRandomFloat(0.0f, 1.0f) < probability) {
					p.w[i] = getRandomFloat(-1.0f, 1.0f);
				}
			}
			if (getRandomFloat(0.0f, 1.0f) < probability) {
				p.b = getRandomFloat(-1.0f, 1.0f);
			}
		}
	}

	// Probability for mutation, on range 0 - 1
	SnakeBrain makeChild(SnakeBrain* parent1, SnakeBrain* parent2, float mutationProbability) {
		auto child = crossOver(parent1, parent2);
		mutate(&child, mutationProbability);

		return child;
	}
}