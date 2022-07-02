#pragma once

#include <string>
#include <cmath>

const float PI = std::acos(0.0f) * 2.0f;

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

	bool operator== (const Vec2i& other)
	{
		return (x == other.x) && (y == other.y);
	}


	int manhattanDistance(const Vec2i& other) {
		return std::abs(x - other.x) + std::abs(y - other.y);
	}

	std::string toString() {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
};

std::vector<int> getRandomInts(int tMin, int tMax, int tNum);
int getRandomInt(int tMin, int tMax);
std::vector<float> getRandomFloats(float tMin, float tMax, int tNum);
float getRandomFloat(float tMin, float tMax);