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

float relu(float v);
float linear(float v);

struct SnakePerceptron {
	std::vector<float> w;
	float b;

	SnakePerceptron(int tNumWeights);
	SnakePerceptron clone();
};

class SnakeBrain {
public:
	SnakeBrain(int tNumInputs, int tNumHiddenLayers, int tHiddenLayerSize, int tOutputLayerSize);
	SnakeBrain(std::vector<SnakePerceptron> tPerceptrons, int tNumInputs, int tNumHiddenLayers, int tHiddenLayerSize, int tOutputLayerSize);
	std::vector<SnakePerceptron> perceptrons;
	int numHiddenLayers;
	int hiddenLayerSize;
	int outputLayerSize;
	int numInputs;

	void init(int tNumInputs, int tNumHiddenLayers, int tHiddenLayerSize, int tOutputLayerSize);
	std::vector<float> think(const std::vector<float>& inputs);
	SnakeBrain clone();
private:
	std::vector<int> layerSizes;
	std::vector<float> processLayer(std::vector<float> inActivations, std::vector<float> weights, float (*activationFunction)(float));
	int layerIdToPerceptronId(int layerIdx, int localIdx);
};

class Snake {
public:
	Snake(SnakeBrain* tSnakeBrain, Vec2i tPos);
	SnakeMove think(std::vector<float> input);
	void updateDirection(SnakeMove move);
	Vec2i nextPosition();
	void move();
	SnakeBrain* snakeBrain;
	Vec2i position;
	SnakeDirection direction;
	std::vector<Vec2i> body;
	bool ateLastMove;
	bool isAlive;
};