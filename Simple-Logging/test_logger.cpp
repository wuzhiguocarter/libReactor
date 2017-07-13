#include "logger.h"

using std::endl;

int test_logger(){
	std::cout << "This a logger test. \n" << std::endl;
	LOG(INFO) << "This a logger test";
}

std::string info_log_filename("info.txt");
std::string warn_log_filename("warn.txt");
std::string error_log_filename("error.txt");

int main(){
	test_logger();
	return 0;
}