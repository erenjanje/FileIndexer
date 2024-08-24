#include "Index.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std::string_literals;

Index::Index(
	 const fs::path& directory,
	 const fs::path& parent,
	 const std::string& index_file_name,
	 const fs::path& base
)
	 : m_parent(parent), m_index_file_name(index_file_name), m_directory(directory), m_base(base) {}

static std::string HumanReadableFileSize(uintmax_t file_size) {
	auto ss = std::stringstream();
	if (file_size >= 1024) {
		ss << std::fixed << std::setprecision(2);
	}
	if (file_size < 1024) {
		ss << file_size << " B";
	} else if (file_size < 1024ull * 1024) {
		ss << ((double)file_size / 1024ull) << " KB";
	} else if (file_size < 1024ull * 1024 * 1024) {
		ss << ((double)file_size / (1024ull * 1024)) << " MB";
	} else if (file_size < 1024ull * 1024 * 1024 * 1024) {
		ss << ((double)file_size / (1024ull * 1024 * 1024)) << " GB";
	} else {
		ss << ((double)file_size / (1024ull * 1024 * 1024 * 1024)) << " TB";
	}
	return ss.str();
}

std::string MakePathForward(const fs::path& path) {
	auto pathstr = path.string();
	if (fs::path::preferred_separator == '\\') {
		std::replace(pathstr.begin(), pathstr.end(), '\\', '/');
	}
	return pathstr;
}

static void ReplaceAll(std::string& s, const std::string& search, const std::string& replace) {
	for (size_t pos = 0;; pos += replace.length()) {
		// Locate the substring to replace
		pos = s.find(search, pos);
		if (pos == std::string::npos)
			break;
		// Replace by erasing and inserting
		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}

void Index::Read() {
	for (const auto& entry : fs::directory_iterator(m_directory)) {
		const auto filepath = entry.path();
		const auto filename = filepath.filename();
		if (filename.string()[0] != '.' and filename.string() != m_index_file_name) {
			if (entry.is_directory()) {
				m_directories.push_back(filepath);
				m_directory_indexers.push_back(Index(filepath, m_directory, m_index_file_name, m_base));
				m_directory_indexers.back().Read();
				m_total_size += m_directory_indexers.back().m_total_size;
			} else {
				m_files.push_back(filepath);
				m_total_size += entry.file_size();
			}
		}
	}
}

void Index::CreateTable() {
	m_table =
		 "<table>\n"
		 "         <tr>\n"
		 "            <th>Name</th>\n"
		 "            <th>Size (Human Readable)</th>\n"
		 "            <th>Last Write Time</th>\n"
		 "            <th>Size (Bytes)</th>\n"
		 "            <th>Type</th>\n"
		 "         </tr>\n";

	m_table += "         <tr>\n";
	m_table +=
		 "            <td><a href=\"/" + MakePathForward(fs::relative(m_directory, m_base)) + "\">.</a></td>\n";	 // Name
	m_table += "            <td><hr/></td>\n";		// Human Readable Size
	m_table += "            <td><hr/></td>\n";		// Last Write Time
	m_table += "            <td><hr/></td>\n";		// Bytes Size
	m_table += "            <td>Directory</td>\n";	// Type
	m_table += "         </tr>\n";

	m_table += "         <tr>\n";
	m_table +=
		 "            <td><a href=\"/" + MakePathForward(fs::relative(m_parent, m_base)) + "\">..</a></td>\n";  // Name
	m_table += "            <td><hr/></td>\n";		// Human Readable Size
	m_table += "            <td><hr/></td>\n";		// Last Write Time
	m_table += "            <td><hr/></td>\n";		// Bytes Size
	m_table += "            <td>Directory</td>\n";	// Type
	m_table += "         </tr>\n";

	for (size_t i = 0; i < m_directories.size(); i++) {
		const auto& dir = m_directories[i];
		const auto& idxer = m_directory_indexers[i];
		m_table += "         <tr>\n";
		m_table += "            <td><a href=\"/" + MakePathForward(fs::relative(dir, m_base).string()) + "\">" +
					  dir.filename().string() + "/" + "</a></td>\n";										 // Name
		m_table += "            <td>" + HumanReadableFileSize(idxer.m_total_size) + "</td>\n";	 // Human Readable Size
		m_table += "            <td><hr/></td>\n";															 // Last Write Time
		m_table += "            <td>" + std::to_string(idxer.m_total_size) + "</td>\n";			 // Bytes Size
		m_table += "            <td>Directory</td>\n";														 // Type
		m_table += "         </tr>\n";
	}

	for (const auto& file : m_files) {
		const auto modification_time = fs::last_write_time(file);
		const auto fnow = decltype(modification_time)::clock::now();
		const auto snow = std::chrono::system_clock::now();
		const auto modtime_sysclock =
			 std::chrono::time_point_cast<std::chrono::system_clock::duration>(modification_time - fnow + snow);
		const auto modtime_timet = std::chrono::system_clock::to_time_t(modtime_sysclock);
		const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(snow.time_since_epoch()) % 1000;
		const auto* gmt = std::gmtime(&modtime_timet);
		auto ss = std::stringstream();
		ss << std::put_time(gmt, "%F %T") << '.' << std::setfill('0') << std::setw(3) << ms.count();
		const auto timestr = ss.str();
		m_table += "         <tr>\n";
		m_table += "            <td><a href=\"/" + MakePathForward(fs::relative(file, m_base).string()) + "\">" +
					  file.filename().string() + "</a></td>\n";												  // Name
		m_table += "            <td>" + HumanReadableFileSize(fs::file_size(file)) + "</td>\n";  // Human Readable Size
		m_table += "            <td>" + timestr + "</td>\n";												  // Last Write Time
		m_table += "            <td>" + std::to_string(fs::file_size(file)) + "</td>\n";			  // Bytes Size
		m_table += "            <td>File</td>\n";																  // Type
		m_table += "         </tr>\n";
	}
	m_table += "</table>";

	for (auto& sub_indexer : m_directory_indexers) {
		sub_indexer.CreateTable();
	}
}

void Index::Write(const std::string& tmplate) {
	auto os = std::ofstream(m_directory / m_index_file_name, std::ios::out | std::ios::trunc);
	auto filled = std::string(tmplate);
	ReplaceAll(filled, "$DIRNAME$", fs::relative(m_directory, m_base).string());
	ReplaceAll(filled, "$DIRTABLE$", m_table);
	os << filled;
	for (auto& idxer : m_directory_indexers) {
		idxer.Write(tmplate);
	}
}
