#pragma once

struct SnakeConfiguration {
	struct Game {
		static const int numSquares = 20;
		static const int timeUnitScore = 1;
		static const int foodScore = 1500;
		static const int foodTimeAdd = 100;
		static const bool manualPlay = false;
		static const int roundTime = 150;
	};
	struct Graphics {
		static const int windowWidth = 1000;
		static const int windowHeight = 800;
		static const int boardMarginLeft = 150;
		static const int boardMarginTop = 50;
		static const int boardMarginBottom = 50;
		static const int squareSize = std::min((SnakeConfiguration::Graphics::windowWidth - boardMarginLeft) / Game::numSquares, (SnakeConfiguration::Graphics::windowHeight - boardMarginTop - boardMarginBottom) / Game::numSquares);
		static constexpr float freezeTimeMs = 2000.0f;
	};
	struct Brain {
		static const int numHiddenLayers = 2;
		static const int hiddenLayerSize = 20;
		static const int outputLayerSize = 3;
		static const int numInputs = 24;	// Make sure it corresponds with measure()
	};
	struct Evolution {
		static const int numSnakeBrains = 1500;
		static constexpr float partOfParentsUsedForCrossover = 0.04f; // On range 0 (none) to 1 (all)
		static constexpr float mutationProbability = 0.01f;	// On range 0 - 1
		static const int numGenerations = 50;
	};
};