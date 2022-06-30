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
	std::vector<float> w;
	float b;

	SnakePerceptron(int tNumWeights) {
		w = getRandomFloats(-1.0f, 1.0f, tNumWeights);
		b = getRandomFloat(-1.0f, 1.0f);
	}

	float inference(float input) {

	}

	SnakePerceptron clone() {
		SnakePerceptron ret(static_cast<int>(w.size()));

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
		
		layerSizes.push_back(numInputs);
		for (int i = 0; i < tNumHiddenLayers; i++) {
			layerSizes.push_back(hiddenLayerSize);
		}
		layerSizes.push_back(outputLayerSize);

		perceptrons.clear();

		// Includes the output layer. Remember, there are no perceptrons going into the first layer (= input layer)
		for (int idxLayerSize = 0; idxLayerSize < layerSizes.size() - 1; idxLayerSize++) {
			std::vector<SnakePerceptron> newPerceptrons;
			for (int idxPerceptron = 0; idxPerceptron < layerSizes[idxLayerSize + 1]; idxPerceptron++) {
				newPerceptrons.push_back(SnakePerceptron(layerSizes[idxLayerSize]));
			}
			perceptrons.insert(perceptrons.end(), newPerceptrons.begin(), newPerceptrons.end());
			int a = 3;
		}
		int a = 3;
	}

	std::vector<float> think(const std::vector<float>& inputs) {
		int perceptronOffsetIn = 0;
		int perceptronOffsetOut = 0;

		std::vector<float> activations;
		for (auto& i : inputs) {
			activations.push_back(i);
		}

		// TODO: Should we use softmax for the output layer?

		int perceptronOffset = 0;
		for (int idxLayerSize = 1; idxLayerSize < layerSizes.size(); idxLayerSize++) {
			std::vector<float> newActivations;
			for (int idxPerceptron = perceptronOffset; idxPerceptron < perceptronOffset + layerSizes[idxLayerSize]; idxPerceptron++) {
				float activation = 0;
				SnakePerceptron* perceptron = &perceptrons[idxPerceptron];
				for (int idxActivation = 0; idxActivation < activations.size(); idxActivation++) {
					activation += activations[idxActivation] * perceptron->w[idxActivation];
				}
				activation = relu(activation + perceptron->b);
				newActivations.push_back(activation);
			}
			perceptronOffset += layerSizes[idxLayerSize];
			activations = newActivations;
		}

		return activations;
	}

	SnakeBrain clone() {
		return SnakeBrain(perceptrons, numInputs, numHiddenLayers, hiddenLayerSize, outputLayerSize);
	}


private:
	std::vector<int> layerSizes;

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
			ret += layerSizes[i] * layerSizes[i + 1];
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
		body.push_back(tPos + Vec2i(0, -2));
		body.push_back(tPos + Vec2i(0, -1));
		body.push_back(tPos);
		isAlive = true;
		ateLastMove = false;
		snakeBrain = tSnakeBrain;
	}

	SnakeMove think(std::vector<float> input) {
		auto outputs = snakeBrain->think(input);
		auto maxElementIndex = std::distance(std::begin(outputs), std::max_element(std::begin(outputs), std::end(outputs)));

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
			{SnakeDirection::Down, Vec2i(0,1)},
			{SnakeDirection::Left,Vec2i(-1,0)},
			{SnakeDirection::Right, Vec2i(1,0)},
			{SnakeDirection::Up, Vec2i(0,-1)}
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