#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
std::string MakePathForward(const fs::path& path);

struct Index {
	explicit Index(
		 const fs::path& directory,
		 const fs::path& parent,
		 const std::string& index_file_name,
		 const fs::path& base
	);
	void Read();
	void CreateTable();
	void Write(const std::string& tmplate);

private:
	fs::path m_parent;
	std::string m_index_file_name;
	fs::path m_directory;
	fs::path m_base;
	std::vector<fs::path> m_files;
	std::vector<fs::path> m_directories;
	std::vector<Index> m_directory_indexers;
	std::string m_table;
	uintmax_t m_total_size = 0;
};
