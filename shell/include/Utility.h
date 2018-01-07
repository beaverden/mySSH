#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <string.h>

/**
 * \brief Trims a string of spaces and tabs
 *
 * \param[in,out] str The target string to be trimmed
*/
void trim(std::string& str);

/**
 * \brief Trims a string of spaces and tabs and the specified characters in 
to_remove
 *
 * \param[in,out] str The target string to be trimmed
 * \param[in] toRemove Characters to be trimmed from the string ends
*/
void trim(std::string& str, std::string toRemove);

#endif