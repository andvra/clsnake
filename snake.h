#pragma once

#include <vector>
#include <ctime>
#include <boost/random.hpp>

#include "utils.h"
#include <map>

enum class SnakeDirection {
	Left,
	Right,
	Up,
	Down
};

// Direction relative to current direction
enum class SnakeMove {
	Left,
	Right,
	Forward
};

class BrainLayer {
public:
	std::vector<float> weights;

	void initRandom(int numNodes) {
		weights.reserve(numNodes);
		const int maxUniqueNumbers = 100000;
		auto randomInts = getRandomInts(0, maxUniqueNumbers, numNodes);
		for (int idx = 0; idx < numNodes; idx++) {
			weights.push_back(randomInts[idx] / static_cast<float>(maxUniqueNumbers));
		}
	}

	BrainLayer clone() {
		BrainLayer c;
		c.weights = weights;

		return c;
	}
};

float relu(float v) {
	return std::max(0.0f, v);
}

float linear(float v) {
	return v;
}

class SnakeBrain {
public:
	SnakeBrain() {}

	void init(int numHiddenLayers, int hiddenLayerSize, int outputLayerSize) {
		for (int idx = 0; idx < numHiddenLayers; idx++) {
			BrainLayer layer;
			layer.initRandom(hiddenLayerSize);
			hiddenLayers.push_back(layer);
		}
		outputLayer.initRandom(outputLayerSize);
	}

	std::vector<float> think(std::vector<float> inputs) {
		std::vector<int> sizes;
		std::vector<float> activations = inputs;

		for (auto& h : hiddenLayers) {
			activations = processLayer(activations, h.weights, &relu);
		}
		
		activations = processLayer(activations, outputLayer.weights, &linear);

		return activations;
	}

	SnakeBrain clone() {
		return SnakeBrain(hiddenLayers, outputLayer);
	}

private:
	std::vector<BrainLayer> hiddenLayers;
	BrainLayer outputLayer;

	SnakeBrain(std::vector<BrainLayer> tHiddenLayers, BrainLayer tOutputLayer) {
		for (auto& t : tHiddenLayers) {
			hiddenLayers.push_back(t.clone());
		}
		outputLayer = tOutputLayer.clone();
	}

	std::vector<float> processLayer(std::vector<float> inActivations, std::vector<float> weights, float (*activationFunction)(float)) {
		std::vector<float> outActivations(weights.size(), 0.0f);

		for (int i = 0; i < weights.size(); i++) {
			float sum = 0;
			for (auto a : inActivations) {
				sum += a * weights[i];
			}
			outActivations[i] = activationFunction(sum);
		}
		return outActivations;
	}
};

class Snake {

public:
	Snake(SnakeBrain* tSnakeBrain, Vec2i tPos, SnakeDirection tDir) {
		position = tPos;
		direction = tDir;
		body.push_back(tPos);
		isAlive = true;
		ateLastMove = false;
		snakeBrain = tSnakeBrain;
	}

	SnakeMove think(std::vector<float> input) {
		auto outputs = snakeBrain->think(input);
		int maxElementIndex = std::max_element(outputs.begin(), outputs.end()) - outputs.begin();

		SnakeMove dir = SnakeMove::Forward;

		// Order here doesn't matter; as long as the order is always the same,
		//	the brain will evolve accordingly
		switch (maxElementIndex) {
		case 0:dir = SnakeMove::Forward; break;
		case 1:dir = SnakeMove::Left; break;
		case 2:dir = SnakeMove::Right; break;
		default: std::cout << "maxElementIndex out of bounds: " << maxElementIndex << std::endl;
		}

		return dir;
	}

	void updateDirection(SnakeMove move) {
		if (move == SnakeMove::Forward) {
			return;
		}

		std::map<SnakeDirection, SnakeDirection> leftMove = {
			{ SnakeDirection::Down, SnakeDirection::Right},
			{ SnakeDirection::Left, SnakeDirection::Down},
			{ SnakeDirection::Right, SnakeDirection::Up},
			{ SnakeDirection::Up, SnakeDirection::Left},
		};

		std::map<SnakeDirection, SnakeDirection> rightMove = {
			{ SnakeDirection::Down, SnakeDirection::Left},
			{ SnakeDirection::Left, SnakeDirection::Up},
			{ SnakeDirection::Right, SnakeDirection::Down},
			{ SnakeDirection::Up, SnakeDirection::Right},
		};

		if (move == SnakeMove::Left) {
			direction = leftMove[direction];
		}
		else {
			direction = rightMove[direction];
		}
	}

	Vec2i nextPosition() {
		std::map<SnakeDirection, Vec2i> pDeltaMap = {
			{SnakeDirection::Down, Vec2i(0,-1)},
			{SnakeDirection::Left,Vec2i(-1,0)},
			{SnakeDirection::Right, Vec2i(1,0)},
			{SnakeDirection::Up, Vec2i(0,1)}
		};

		return position + pDeltaMap[direction];
	}

	void move() {
		position = nextPosition();

		body.push_back(position);

		// Remove oldest body part, given we didn't eat last round
		if (!ateLastMove) {
			body.erase(body.begin());
		}

		ateLastMove = false;

	}

	SnakeBrain* snakeBrain;
	Vec2i position;
	SnakeDirection direction;
	std::vector<Vec2i> body;
	bool ateLastMove;
	bool isAlive;
};