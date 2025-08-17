#ifndef SMITH_WATERMAN_H
#define SMITH_WATERMAN_H

#include <string>
#include <vector>

std::pair<std::string, std::string> smithWaterman(const char *seq1, size_t size1, const char *seq2, size_t size2);

#endif