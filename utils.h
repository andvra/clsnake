#pragma once
#include <random>

struct Vec2i {
	int x;
	int y;

	Vec2i(int tx, int ty) {
		x = tx;
		y = ty;
	}

	Vec2i() : Vec2i(0, 0) {	}

	Vec2i operator+(const Vec2i& other) {
		Vec2i r;
		
		r.x = x + other.x;
		r.y = y + other.y;

		return r;
	}

	Vec2i operator-(const Vec2i& other) {
		Vec2i r;

		r.x = x - other.x;
		r.y = y - other.y;

		return r;
	}

	int manhattanDistance(const Vec2i& other) {
		return std::abs(x - other.x) + std::abs(y - other.y);
	}

	std::string toString() {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
};

bool operator== (const Vec2i& v1, const Vec2i& v2)
{
	return (v1.x == v2.x) && (v1.y == v2.y);
}

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
