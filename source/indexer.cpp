#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void PrintHelp(const std::string& program_name) {
	std::cout << "Usage: \n"
				 << "   " << fs::path(program_name).filename().string()
				 << " [-h/--help] [DIRECTORY]\n"
					 "\n"
					 "-h/--help: Prints this help text.\n"
					 "DIRECTORY: Directory to be indexed and where index.html files should be generated.\n";
}

int main(int argc, char* argv[]) {
	if (argc <= 1 or std::string(argv[1]) == "-h" or std::string(argv[1]) == "--help") {
		PrintHelp(argv[0]);
		return EXIT_SUCCESS;
	}
	if (argv[1][0] == '-') {
		std::cerr << "Unrecognized flag: " << std::quoted(argv[1]) << "\n";
		return EXIT_FAILURE;
	}
	std::cout << std::string(argv[1]) << "\n";
	return EXIT_SUCCESS;
}
