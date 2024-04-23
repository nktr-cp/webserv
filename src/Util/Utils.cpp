#include "Util.hpp"

bool	isNumber(const std::string& str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}