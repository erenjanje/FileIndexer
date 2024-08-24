#include <filesystem>
#include <iostream>
#include <string>

#include "Index.hpp"

using namespace std::string_literals;

namespace fs = std::filesystem;

void PrintHelp(const std::string& program_name) {
	std::cout << "Usage: \n"
				 << "   " << fs::path(program_name).filename().string()
				 << " [-h/--help] DIRECTORY BASE [TITLE]\n"
					 "\n"
					 "-h/--help: Prints this help text.\n"
					 "DIRECTORY: Directory to be indexed and where index.html files should be generated.\n"
					 "BASE: The base directory that should be the base of the links.\n"
					 "TITLE: Title of the created pages, the name of the directory appended after a dash (-). Default is "
					 "\"Files\".\n";
}

int main(int argc, char* argv[]) {
	if (argc <= 2 or std::string(argv[1]) == "-h" or std::string(argv[1]) == "--help") {
		PrintHelp(argv[0]);
		return EXIT_SUCCESS;
	}
	if (argv[1][0] == '-') {
		std::cerr << "Unrecognized flag: " << std::quoted(argv[1]) << "\n";
		return EXIT_FAILURE;
	}
	const auto title = (argc <= 3) ? ("Files"s) : (argv[3]);

	auto index = Index(fs::path(argv[1]), fs::path(argv[1]), "index.html", fs::path(argv[2]));
	index.Read();
	index.CreateTable();
	index.Write(
		 "<!DOCTYPE html>\n"
		 "<html>\n"
		 "   <head>\n"
		 "      <meta charset=\"utf-8\"/>\n"
		 "      <title>" +
		 title +
		 " - $DIRNAME$</title>\n"
		 "      <link rel=\"stylesheet\" href=\"/" +
		 MakePathForward(fs::relative(argv[1], argv[2]) / "style.css") +
		 "\"/>\n"
		 "   </head>\n"
		 "   <body>\n"
		 "      $DIRTABLE$\n"
		 "   </body>\n"
		 "</html>\n"
	);

	return EXIT_SUCCESS;
}
