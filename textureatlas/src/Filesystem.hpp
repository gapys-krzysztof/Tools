#ifndef __FILESYSTEM_HPP__
#define __FILESYSTEM_HPP__

#include <string>
#include <vector>

std::vector<std::string> ListDirectory( const std::string& path );
bool Exists( const std::string& file );

#endif
