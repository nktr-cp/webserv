#include "Utils.hpp"

namespace ft {
	/*
	* isNumber: check if a string is composed purely of digits
	* @param str: string to check
	* @return: true if str is composed purely of digits, false otherwise
	*/
	bool isNumber(const std::string& str) {
		for (size_t i = 0; i < str.size(); i++) {
			if (!isdigit(str[i]))
				return false;
		}
		return true;
	}

	/*
	* stoui: convert a string to an unsigned integer
	* @param str: string to convert
	* @param range: range of the integer, inclusive
	* @return: the integer represented by str
	* @throw invalid_argument: if str is not composed purely of digits
	* @throw out_of_range: if the integer is out of range
	*/
	unsigned int stoui(const std::string& str, const unsigned int range[2]) {
		unsigned n = 0;

		if (!ft::isNumber(str) || range[0] > range[1])
			throw std::invalid_argument("stoui: invalid argument");
		for (size_t i = 0; i < str.size(); i++) {
			n = n * 10 + str[i] - '0';
		}
		if (n < range[0] || range[1] < n)
			throw std::out_of_range("stoui: out of range");
		return n;
	}
}
