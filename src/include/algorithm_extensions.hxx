#include <vector>
#include <cmath>
#include <limits>

#ifndef ALGORITHM_EXTENSIONS_HEADER
#define ALGORITHM_EXTENSIONS_HEADER

namespace algext{
	/// Returns a vector of the same length containing the index of each value sorted from least to greatest.
	/// Ex:     v = {1,4,5,9,3,10}
	/// Returns:    {0,2,3,4,1,5}
	template<typename T>
	std::vector<size_t> sortperm(std::vector<T>& v){
		using pair = std::pair<T, size_t>;
		std::vector<size_t> indices(v.size());
		std::vector<pair> pair_v(v.size());

		// enumerate values.
		for (size_t i = 0; i < v.size(); i++)
			pair_v[i] = pair(v[i], i);
		// sort values from least to greatest
		if (pair_v.size() > 0)
			std::sort(pair_v.begin(), pair_v.end(), [](pair& a, pair& b){ return a.first < b.first;});
		// copy indices of sorted values to new vector
		for (size_t i = 0; i < v.size(); i++)
			indices[i] = pair_v[i].second;
		// return vector filled with indices of values in original vector
		return indices;
	}

	/// Returns a linearly spaced vector.
	std::vector<double> linspace(double min, double max, std::size_t n_elem){
		std::vector<double> vals(n_elem);
		size_t iterator = 0;
		
		for (size_t i = 0; i <= n_elem-2; i++)
			vals[iterator++] = min + i*(max-min)/(std::floor((double)n_elem) - 1);

		vals[iterator] = max;
		return vals;
	}

	/// Returns a vector with a given minumum, step, and maximum value.
	template<typename T>
	std::vector<T> range(double min, double step, double max){
		size_t n_elems = std::floor((max-min)/step)+1;
		std::vector<T> result(n_elems);
		for (auto& val : result){
			val = (T)min;
			min += step;
		}
		return result;
	}

	/// Applies reduce to a given function.
	template<typename T>
	constexpr T reduce(std::vector<T> const& vec, T init = (T)0, T(*func)(T,T) = [](T acc, T x){return acc + x;}){
		T result = init;
		for (auto const& v: vec){
			result = func(result, v);
		}
		return result;
	}

	template<typename T, typename R>
	std::vector<R> map(std::vector<T> const& vec, R(*f)(T)){
		std::vector<R> result(vec.size());
		for (std::size_t i = 0; i < vec.size(); i++)
			result[i] = f(vec[i]);
		return result;
	}

	std::pair<double, double> extrema(std::vector<double> const& vec){
		double min = std::numeric_limits<double>::infinity();
		double max = -std::numeric_limits<double>::infinity();
		for (auto& v : vec){
			if (v < min){
				min = v;
			} else if (v > max){
				max = v;
			}
		}
		return std::pair<double, double>(min, max);
	}
}

#endif