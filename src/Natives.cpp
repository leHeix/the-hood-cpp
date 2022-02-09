#include "main.hpp"

float VectorSize(float x, float y, float z) noexcept
{
	return std::sqrt(x * x + y * y + z * z);
}