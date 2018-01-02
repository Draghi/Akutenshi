#include <iostream>
#include <memory>
#include <string>

#include "ak/data/Image.hpp"

#if defined(__linux)
#define BACKWARD_HAS_BFD 1
#include "backward.hpp"
namespace backward { static backward::SignalHandling sh; }
#endif

#include "cxxopts.hpp"

extern int akGameMain();
extern int akResourceMain();

int main(int argc, char* argv[]) {
	cxxopts::Options options("Akutenshi", "Akutenshi game engine");
	options.add_options()
		("h,help", "Display help", cxxopts::value<bool>())
		("m,mode", "Select mode ('game' or 'resource')", cxxopts::value<std::string>()->default_value("game"));


	try {
		auto opts = options.parse(argc, argv);

		if (opts.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}

		if (opts["m"].as<std::string>() == "game") {
			return akGameMain();
		} else if (opts["m"].as<std::string>() == "resource") {
			return akResourceMain();
		} else {
			std::cout << "Invalid option selected for -m/--mode, use -h or --help for more inforamtion." << std::endl;
			return -1;
		}

	} catch(const cxxopts::option_not_exists_exception&) {
		std::cout << "Unrecognized option on command line, use -h or --help for more information on command line arguments" << std::endl;
		return -1;
	}
}



