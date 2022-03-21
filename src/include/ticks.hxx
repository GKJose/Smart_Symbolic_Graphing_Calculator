#ifndef TICKS_HEADER
#define TICKS_HEADER
#include<type_traits>
#include <option.hxx>
#include <rounding.hxx>
#include <algorithm_extensions.hxx>
#include <array>
#include <map>
#include <functional>
#include <cstdlib>
#include <cassert>

namespace Ticks {

	struct TickInfo {
		std::vector<double> ticks;
		double x_min;
		double x_max;
	};

	namespace LogScales{
		enum Enum{
			Ln,
			Log2,
			Log10
		};
	}

	namespace LogScaleBases{
		static std::map<LogScales::Enum, double> dict{
			{LogScales::Ln, std::exp(1.0)},
			{LogScales::Log2, 2.0},
			{LogScales::Log10, 10.0}};
	}



    /// Find the smallest order of magnitude that is larger than xspan.
	template<typename T>
	std::size_t bounding_order_of_magnitude(T xspan, T base){
		std::size_t a = 1, step = 1;
		while (xspan < std::pow(base, a))
			a -= step;

		std::size_t b = step = 1;
		while (xspan > std::pow(base, b))
			b += step;
		
		while (a + 1 < b){
			std::size_t c = a + b / 2;
			if (xspan < std::pow(base, c))
				b = c;
			else
				a = c;
		}
		return b;
	}

    /// Returns a pair of inclusive ranges that show the digit ranges of a given floating point type.
	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point<T>::value, std::pair<ssize_t, ssize_t>> float_digit_range(T){
		T min = std::numeric_limits<T>::min();
		T max = std::numeric_limits<T>::max();
		return std::pair<ssize_t, ssize_t>(
			(ssize_t)std::floor(std::log10(min)),
			(ssize_t)std::floor(std::log10(max)));
	}

	template<typename T>
	constexpr std::enable_if_t<std::is_floating_point<T>::value, ssize_t> postdecimal_digits(T x){
		std::pair<ssize_t, ssize_t> range_inclusive = float_digit_range(x);
		for (ssize_t i = range_inclusive.first; i <= range_inclusive.second; i++){
			if (x == RoundExt::jround<T, RoundExt::RoundDown>(x, (int)i))
				return i;
		}
		return -1;
	}

	TickInfo fallback_ticks(
		double x_min,
		double x_max,
		int k_min,
		int k_max
	){
		if (k_min != 2 && RoundExt::Auxiliary::isfinite(x_min) && RoundExt::Auxiliary::isfinite(x_max)){
			auto ticks = algext::linspace((double)x_min, (double)x_max, (std::size_t)k_min);
			return TickInfo {std::move(ticks), x_min, x_max};
		} else {
			std::vector<double> ticks = {x_min, x_max};
			return TickInfo {std::move(ticks), x_min, x_max};
		}

	}

	static std::vector<std::pair<double, double>> __default_q = {
		std::pair<double, double>(1.0, 1.0),
		std::pair<double, double>(5.0, 0.9),
		std::pair<double, double>(2.0, 0.7),
		std::pair<double, double>(2.5, 0.5),
		std::pair<double, double>(3.0, 0.2)
	};

	template<int k_min = 2, int k_max = 10, int k_ideal = 5>
	TickInfo optimize_ticks(
		double x_min, 
		double x_max, 
		bool extend_ticks = false,
		std::vector<std::pair<double, double>> Q = __default_q,
		double granularity_weight = 1.0/4.0,
		double simplicity_weight = 1.0/6.0,
		double coverage_weight = 1.0/3.0,
		double niceness_weight = 1.0/4.0,
		bool strict_span = true,
		Option<double> span_buffer = OptNone,
		Option<LogScales::Enum> scale = OptNone){
			const double xspan = x_max - x_min;
			if (xspan < std::numeric_limits<double>::epsilon())
				return fallback_ticks(x_min, x_max, k_min, k_max);

			auto n = Q.size();
			bool is_log_scale = false;
			if (scale)
				is_log_scale = true;
			
			const double base = is_log_scale ? LogScaleBases::dict[scale.value()] : 10.0;
			// generalizing "order of magnitude"
			ssize_t z = (ssize_t)bounding_order_of_magnitude(xspan, base);
			// find required significant digits for ticks with q * pow(base, z) spacing,
			// for q values specified in Q
			ssize_t x_digits = (ssize_t)bounding_order_of_magnitude(std::abs(x_min) > std::abs(x_max) ? std::abs(x_min) : std::abs(x_max), base);
			std::size_t q_extra_digits = 0;
			for (auto& q : Q){
				std::size_t n_digits = postdecimal_digits(q.first);
				if (n_digits > q_extra_digits)
					q_extra_digits = n_digits;
			}
			
			auto sigdigits = [&](ssize_t z){
				auto temp = x_digits - z + (ssize_t)q_extra_digits;
				return temp > 1 ? temp : 1;
			};

			auto isinteger = [](double a){
				return RoundExt::jround<double, RoundExt::RoundToZero>(a) == a;
			};

			int ib = (int)base;

			using dfunc = std::function<double(double)>;

			dfunc round_base = ( isinteger(base) ? (dfunc)[&](double v){
				return RoundExt::jround(v, OptNone, Option<int>(sigdigits(z)), Option<int>(ib));
			} : (dfunc)[&](double v){
				return RoundExt::jround(v, OptNone, Option<int>(sigdigits(z)), OptNone);
			});

			double high_score = -std::numeric_limits<double>::infinity();
			std::vector<double> S_best; // used when strict_span is true
			std::vector<double>* S_best_ref = nullptr; // used when strict_span is false
			double viewmin_best = x_min, viewmax_best = x_max;

			// We preallocate vectors that hold all required S arrays for every given
			// the k parameter, so we don't have to create them again and again, which
			// saves many allocations
			
			std::vector<std::vector<double>> prealloc_Ss;
			for (int k = k_min; k <= 2*k_max; k++){
				prealloc_Ss.push_back(std::vector<double>(extend_ticks ? 3*k : k));
			}
			std::vector<double>* S = nullptr; // holds a pointer to an element in prealloc_Ss
			int len_S; // moved outside of the while loop for later use if strict_span is false.
			while (2.0*((double)k_max) * std::pow(base, z+1) > xspan){
				for (struct {int ik, k;} ks = {0, k_min}; ks.k <= 2*k_max; ks.ik++, ks.k++){
					for (struct {std::size_t i; double q, qscore;} qs = {0, Q[0].first, Q[0].second }; qs.i < Q.size(); ++qs.i, qs.q = Q[qs.i].first, qs.qscore = Q[qs.i].second){
						double tickspan = qs.q * std::pow(base, z);
						if (tickspan < std::numeric_limits<double>::epsilon())
							continue;
						double span = (double)(ks.k - 1) * tickspan;
						if (span < xspan)
							continue;
						
						double r = (x_max - span) / tickspan;
						if (!std::isfinite(r))
							continue;
						r = std::ceil(r);

						// try to favor integer exponents for log scales
						bool nice_scale = true;
						if (is_log_scale && !isinteger(tickspan)){
							nice_scale = false;
							qs.qscore = 0.0;
						}
						
						double viewmin, viewmax;
						while (r * tickspan <= x_min){
							//S = prealloc_Ss.at(ks.ik);
							S = (prealloc_Ss.data() + ks.ik);
							//S = prealloc_Ss[ks.ik]; // reference to prealloc[ks.ik]
							assert(&prealloc_Ss[ks.ik] == (prealloc_Ss.data() + ks.ik));
							assert(S == &prealloc_Ss[ks.ik]);
							//assert(&S == (prealloc_Ss.data() + ks.ik));
							// Filter or expand ticks
							if (extend_ticks){
								for (std::size_t i = 0; i < 3*ks.k; i++){
									S->at(i) = (r * (double)i - (double)ks.k) * tickspan;
								}
								// Round only those values that end up as viewmin and viewmax
								// to save computation time
								S->at(ks.k) = round_base(S->at(ks.k));
								S->at(2*ks.k - 1) = round_base(S->at(2*ks.k - 1));
								viewmin = S->at(ks.k), viewmax = S->at(2*ks.k - 1);
							} else {
								for (std::size_t i = 0; i < ks.k; i++){
									S->at(i) = (r + (double)i) * tickspan;
								}
								// Round only those values that end up as viewmin and viewmax
								// to save computation time
								S->at(0) = round_base(S->at(0));
								S->at(ks.k - 1) = round_base(S->at(ks.k - 1));
								viewmin = S->at(0), viewmax = S->at(ks.k - 1);
							}

							int len_S = (int)S->size();

							if (strict_span){
								viewmin = viewmin > x_min ? viewmin : x_min;
								viewmax = viewmax < x_max ? viewmax : x_max;

								double buf = (span_buffer ? span_buffer.value() : 0.0) * (viewmax - viewmin);

								// filter the S array while reusing its own memory to do so
								// this works becauase S is sorted, and we will only overwrite
								// values that are not needed anymore going forward in the loop

								// we do this because it saves allocations
								std::size_t counter = 0;
								for(std::size_t i = 0; i < S->size(); i++){
									if ((viewmin - buf) <= S->at(i) && S->at(i) <= (viewmax + buf)){
										S->at(counter) = S->at(i);
										counter += 1;
									}
								}
								len_S = (int)counter;
							}

							// evaluate quality of ticks
							bool has_zero = (r <= 0.0) && (std::abs(r) < (double)ks.k);

							// simplicity
							bool s = has_zero && nice_scale;

							// granularity
							double g = ((0 < len_S) && (len_S < 2*k_ideal)) ? 
								1.0 - ((double)std::abs(len_S - k_ideal))/((double)k_ideal) :
								0.0;
							
							// coverage 
							double c = len_S > 1 ? (1.5*xspan / (((double)(len_S - 1)) * tickspan)) : 0.0;

							double score = 
								granularity_weight * g +
								simplicity_weight * (double)s +
								coverage_weight * c +
								niceness_weight * qs.qscore;

							// strict limits on coverage
							if (strict_span && span > xspan)
								score -= 10000.0;
							
							if (span >= 2.0*xspan)
								score -= 1000;

							if (score > high_score && (k_min <= len_S) && (len_S <= k_max)){
								if (strict_span){
									// make S a copy because it is a view and could
									// otherwise be mutated in the next runs.
									// -- NOT DONE IN C++ -- Could be fine as is... I am not sure.
									// std::cout << "S: ";
									// for (auto& val : S)
									// 	std::cout << val << " ";
									// std::cout << "\n";
									S_best = *S;
									S_best.resize(len_S);
								} else {
									S_best_ref = S;
								}
								
								//S_best = &S; // reference to current highest score.
								viewmin_best = viewmin;
								viewmax_best = viewmax;
								high_score = score;
							}
							r += 1.0;
						}
					}
				}
				z -= 1;
			}
			
			
		if (std::isinf(high_score)){
			if (strict_span){
				std::cout << "WARNING: NO STRICT TICKS FOUND\n";
				// This time run without strict ticks enabled.
				return optimize_ticks<k_min, k_max, k_ideal>(
					x_min,
					x_max,
					extend_ticks,
					Q,
					granularity_weight,
					simplicity_weight,
					coverage_weight,
					niceness_weight,
					false, // no strict span
					span_buffer,
					scale
				);
			} else {
				return fallback_ticks(x_min, x_max, k_min, k_max);
			}
		}
		if (strict_span){ // use non-ptr S_best.
			return TickInfo { std::move(S_best), viewmin_best, viewmax_best};
		} else { // use ptr S_best.
			S_best_ref->resize(len_S);
			return TickInfo {std::move(*S_best_ref), viewmin_best, viewmax_best};
		}
		// std::vector<double> veccy = std::move(*S_best);
		// return TickInfo { std::move(veccy), viewmin_best, viewmax_best};
	}
}


#endif