#pragma once

#include "snake.h"

namespace ClSnake {

	BrainLayer mixBrainLayers(BrainLayer* b1, BrainLayer* b2) {
		BrainLayer b;

		for (int wIdx = 0; wIdx < b1->weights.size(); wIdx++) {
			if (getRandomInt(0, 1000) > 500) {
				b.weights.push_back(b1->weights[wIdx]);
			}
			else {
				b.weights.push_back(b2->weights[wIdx]);
			}
		}

		return b;

	}

	SnakeBrain crossOver(SnakeBrain* parent1, SnakeBrain* parent2) {
		SnakeBrain child;

		for (int idx = 0; idx < parent1->hiddenLayers.size(); idx++) {
			auto& brainLayer1 = parent1->hiddenLayers[idx];
			auto& brainLayer2 = parent2->hiddenLayers[idx];
			child.hiddenLayers.push_back(mixBrainLayers(&brainLayer1, &brainLayer2));
		}

		auto& outputLayer1 = parent1->outputLayer;
		auto& outputLayer2 = parent2->outputLayer;
		child.outputLayer = mixBrainLayers(&outputLayer1, &outputLayer2);

		return child;
	}

	void mutate(SnakeBrain* brain) {
		for (auto& layer : brain->hiddenLayers) {
			for (int i = 0; i < layer.weights.size(); i++) {
				if (getRandomInt(0, 1000) < 50) {
					layer.weights[i] = getRandomFloat(-1.0f, 1.0f);
				}
			}
		}
	}

	SnakeBrain makeChild(SnakeBrain* parent1, SnakeBrain* parent2) {
		auto child = crossOver(parent1, parent2);
		mutate(&child);

		return child;
	}
}