#ifndef FILTFILT_H
#define FILTFILT_H

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <exception>
#include <iostream>
#include <vector>

#include "Arduino.h"

typedef std::vector<int> vectori;
typedef std::vector<double> vectord;
void add_index_range(vectori &indices, int beg, int end, int inc = 1);
void add_index_const(vectori &indices, int value, size_t numel);
void append_vector(vectord &vec, const vectord &tail);
vectord subvector_reverse(const vectord &vec, int idx_end, int idx_start);
void filter(vectord B, vectord A, const vectord &X, vectord &Y, vectord &Zi);
void filtfilt(vectord B, vectord A, const vectord &X, vectord &Y);
bool compare(const vectord &original, const vectord &expected, double tolerance = DBL_EPSILON);

#endif