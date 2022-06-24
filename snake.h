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

float relu(float v) {
	return std::max(0.0f, v);
}

float linear(float v) {
	return v;
}

struct SnakePerceptron {
	float w;
	float b;

	SnakePerceptron() {
		w = getRandomFloat(-1.0f, 1.0f);
		b = getRandomFloat(-1.0f, 1.0f);
	}

	float inference(float input) {

	}

	SnakePerceptron clone() {
		SnakePerceptron ret;

		ret.w = w;
		ret.b = b;

		return ret;
	}
};

class SnakeBrain {
public:
	SnakeBrain(int tNumInputs, int tNumHiddenLayers, int tHiddenLayerSize, int tOutputLayerSize) {
		init(tNumInputs, tNumHiddenLayers, tHiddenLayerSize, tOutputLayerSize);
	}

	SnakeBrain(std::vector<SnakePerceptron> tPerceptrons, int tNumInputs, int tNumHiddenLayers, int tHiddenLayerSize, int tOutputLayerSize) {
		init(tNumInputs, tNumHiddenLayers, tHiddenLayerSize, tOutputLayerSize);

		perceptrons.clear();

		for (auto& p : tPerceptrons) {
			perceptrons.push_back(p.clone());
		}
	}

	std::vector<SnakePerceptron> perceptrons;
	int numHiddenLayers;
	int hiddenLayerSize;
	int outputLayerSize;
	int numInputs;

	void init(int tNumInputs, int tNumHiddenLayers, int tHiddenLayerSize, int tOutputLayerSize) {
		numInputs = tNumInputs;
		numHiddenLayers = tNumHiddenLayers;
		hiddenLayerSize = tHiddenLayerSize;
		outputLayerSize = tOutputLayerSize;
		
		sizes.push_back(numInputs);
		for (auto& _ : { numHiddenLayers }) {
			sizes.push_back(hiddenLayerSize);
		}
		sizes.push_back(outputLayerSize);
		int numPerceptrons = 0;
		for (int i = 0; i < sizes.size() - 1; i++) {
			numPerceptrons += sizes[i] * sizes[i + 1];
		}
		perceptrons = std::vector<SnakePerceptron>(numPerceptrons, SnakePerceptron());

	}

	std::vector<float> think(const std::vector<float>& inputs) {
		int offsetIn = 0;
		int offsetOut = 0;

		std::vector<float> activations;
		for (auto& i : inputs) {
			activations.push_back(i);
		}

		// TODO: Calculate softmax for output
		// Is ReLU used before softmax? Otherwise we should only loop
		//	until sizes.size() - 2 below!

		// This loop connects the current layer with the next
		for (int idxLayer = 0; idxLayer < sizes.size() - 1; idxLayer++) {
			offsetOut += sizes[idxLayer];
			std::vector<float> newActivations;
			for (int idxOut = 0; idxOut < sizes[idxLayer + 1]; idxOut++) {
				float activation = 0.0f;
				for (int idxIn = 0; idxIn < activations.size(); idxIn++) {
					auto perceptronIdx = layerIdToPerceptronId(idxLayer, idxOut * activations.size() + idxIn);
					auto& perceptron = perceptrons[perceptronIdx];
					activation += relu(activations[idxIn] * perceptron.w + perceptron.b);
				}
				newActivations.push_back(activation);
			}
			// Calculated activations becomes inputs for the next layer
			activations = newActivations;
			// Output turn input in next step
			offsetIn += sizes[idxLayer];
		}

		return activations;
	}

	SnakeBrain clone() {
		return SnakeBrain(perceptrons, numInputs, numHiddenLayers, hiddenLayerSize, outputLayerSize);
	}


private:
	std::vector<int> sizes;

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

	int layerIdToPerceptronId(int layerIdx, int localIdx) {
		int ret = 0;

		for (int i = 0; i < layerIdx; i++) {
			ret += sizes[i] * sizes[i + 1];
		}

		ret += localIdx;

		return ret;
	}
};

class Snake {

public:
	Snake(SnakeBrain* tSnakeBrain, Vec2i tPos) {
		position = tPos;
		direction = SnakeDirection::Down;
		body.push_back(tPos - Vec2i(0, 2));
		body.push_back(tPos - Vec2i(0, 1));
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