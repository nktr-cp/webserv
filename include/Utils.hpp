#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <string>

namespace ft {
	bool			isNumber(const std::string& str);
	unsigned int	stoui(const std::string& str, const unsigned int range[2]);
}

#endif