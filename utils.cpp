#include <random>
#include "utils.h"

std::random_device rd;
std::mt19937 gen(rd());

std::vector<int> getRandomInts(int tMin, int tMax, int tNum) {

	std::uniform_int_distribution<> dist(tMin, tMax);

	std::vector<int> vals;
	vals.reserve(tNum);

	for (int i = 0; i < tNum; i++) {
		vals.push_back(dist(gen));
	}

	return vals;
}

int getRandomInt(int tMin, int tMax) {
	return getRandomInts(tMin, tMax, 1)[0];
}

std::vector<float> getRandomFloats(float tMin, float tMax, int tNum) {
	std::uniform_real_distribution<> dist(tMin, tMax);

	std::vector<float> vals;
	vals.reserve(tNum);

	for (int i = 0; i < tNum; i++) {
		vals.push_back(static_cast<float>(dist(gen)));
	}

	return vals;
}

float getRandomFloat(float tMin, float tMax) {
	return getRandomFloats(tMin, tMax, 1)[0];
}
