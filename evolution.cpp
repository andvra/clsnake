#include <format>
#include <thread>
#include <iostream>
#include <SDL2/SDL_timer.h>

#include "evolution.h"
#include "config.h"
#include "game.h"

namespace ClSnake {

	SnakeBrain crossOver(SnakeBrain* parent1, SnakeBrain* parent2) {
		std::vector<SnakePerceptron> perceptrons;

		for (int i = 0; i < parent1->perceptrons.size(); i++) {
			SnakeBrain* useBrain = (getRandomInt(0, 1000) > 500) ? parent1 : parent2;
			perceptrons.push_back(useBrain->perceptrons[i].clone());
		}

		return SnakeBrain(perceptrons, parent1->numInputs, parent1->numHiddenLayers, parent1->hiddenLayerSize, parent1->outputLayerSize);
	}

	// Probability for mutation, on range 0 - 1
	void mutate(SnakeBrain* brain, float probability) {

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

	SnakeBrain makeChild(SnakeBrain* parent1, SnakeBrain* parent2, float mutationProbability) {
		auto child = crossOver(parent1, parent2);
		mutate(&child, mutationProbability);

		return child;
	}

	void evolve(std::vector<SnakeBrain>& replaySnakeBrains, int& useSnakeBrainGeneration) {
		std::vector<SnakeBrain> snakeBrains;

		for (int i = 0; i < SnakeConfiguration::Evolution::numSnakeBrains; i++) {
			SnakeBrain brain(SnakeConfiguration::Brain::numInputs, SnakeConfiguration::Brain::numHiddenLayers, SnakeConfiguration::Brain::hiddenLayerSize, SnakeConfiguration::Brain::outputLayerSize);
			snakeBrains.push_back(brain);
		}

		// hardware_concurrency will return 0 when not able to detect
		const int numThreads = std::max(1u, std::thread::hardware_concurrency());

		std::cout << std::format("Running evolution with {} threads\n-----\n", numThreads);

		std::cout << std::format("Gen\tMax score\tTime (s)") << std::endl;
		auto bestGenerationScore = 0;

		for (int gen = 0; gen < SnakeConfiguration::Evolution::numGenerations; gen++) {
			auto genStartTime = SDL_GetTicks64();
			std::vector<std::tuple<int, SnakeBrain*>> brainsWithScore(snakeBrains.size(), std::tuple<int, SnakeBrain*>(0, 0));

			// Start with evaluation the fitness of each chromosome in the current generation
			for (int brainOffset = 0; brainOffset < snakeBrains.size(); brainOffset += numThreads) {
				std::vector<std::thread> threads;
				for (int idxBrain = brainOffset; idxBrain < std::min(SnakeConfiguration::Evolution::numSnakeBrains, brainOffset + numThreads); idxBrain++) {
					SnakeBrain& brain = snakeBrains[idxBrain];
					threads.push_back(std::thread([&brainsWithScore, &brain, idxBrain]() {
						Game game(&brain, SnakeConfiguration::Game::numSquares, SnakeConfiguration::Game::numSquares);
						game.play();
						auto fitness = game.fitness();
						brainsWithScore[idxBrain] = std::tuple<int, SnakeBrain*>(fitness, &brain);
						}));

				}
				for (auto& t : threads) {
					t.join();
				}
			}

			std::sort(brainsWithScore.begin(), brainsWithScore.end(), [](std::tuple<int, SnakeBrain*> a, std::tuple<int, SnakeBrain*>b) {return std::get<0>(a) > std::get<0>(b); });

			auto bestBrainInGeneration = std::get<1>(brainsWithScore.front());

			auto maxScore = std::get<0>(brainsWithScore.front());
			auto genTimeS = (SDL_GetTicks64() - genStartTime) / 1000.0f;
			std::cout << std::format("{}\t{}\t\t{}", gen + 1, maxScore, genTimeS) << std::endl;

			if (maxScore > bestGenerationScore) {
				useSnakeBrainGeneration = gen;
				bestGenerationScore = maxScore;
			}

			replaySnakeBrains.push_back(bestBrainInGeneration->clone());

			// Time to evolve!
			if (gen < SnakeConfiguration::Evolution::numGenerations - 1) {
				std::vector<SnakeBrain> newSnakeBrains;
				// Keep the best brain of each generation
				newSnakeBrains.push_back(bestBrainInGeneration->clone());
				// TODO: Think of good criteria for a parent
				int numParents = std::max(2, static_cast<int>(SnakeConfiguration::Evolution::numSnakeBrains * SnakeConfiguration::Evolution::partOfParentsUsedForCrossover));
				std::vector<SnakeBrain*> parents;
				parents.reserve(numParents);
				for (int i = 0; i < numParents; i++) {
					parents.push_back(std::get<1>(brainsWithScore[i]));
				}
				// Start at one, since we already added the currently best brain to the vector
				for (int childIdx = 1; childIdx < SnakeConfiguration::Evolution::numSnakeBrains; childIdx++) {
					auto parentIdx1 = 0;
					auto parentIdx2 = 0;
					// Make sure the parents are two different individuals
					while (parentIdx1 == parentIdx2) {
						parentIdx1 = getRandomInt(0, numParents - 1);
						parentIdx2 = getRandomInt(0, numParents - 1);
					}
					SnakeBrain child = ClSnake::makeChild(parents[parentIdx1], parents[parentIdx2], SnakeConfiguration::Evolution::mutationProbability);
					newSnakeBrains.push_back(child);
				}
				snakeBrains = newSnakeBrains;
			}
		}
	}
}