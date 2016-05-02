#include <iostream>
#include "constexpr_string.hpp"

int main() {
	using namespace constexpr_string;
	constexpr auto a = make_string("Hello");
	constexpr auto b = make_string(" ");
	constexpr auto c = make_string("World");
	
	constexpr auto str = a + b + c;
	
	constexpr auto str_lower = str.to_lower();
	std::cout << str_lower() << std::endl;
	
	constexpr auto str_upper = str.to_upper();
	std::cout << str_upper() << std::endl;
	
	constexpr auto str_hello = str.substr<0, str.find(' ')>();
	std::cout << str_hello() << std::endl;
	
	constexpr auto str_world = str.substr<str.find(' ') + 1, str.length - str.rfind(make_string(" ")) - 1>();
	std::cout << str_world() << std::endl;
	
	return 0;
}