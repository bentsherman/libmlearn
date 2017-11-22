/**
 * @file data/directory.h
 *
 * Interface definitions for the directory type.
 */
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <vector>

namespace ML {

class Directory {
private:
	std::string _path;
	std::vector<std::string> _entries;

public:
	Directory(const std::string& path);
	~Directory() {};

	const std::string& path() const { return _path; }
	const std::vector<std::string>& entries() const { return _entries; }
};

}

#endif