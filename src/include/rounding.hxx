#ifndef ROUNDING_HEADER
#define ROUNDING_HEADER
#include<type_traits>
#include<cmath>
#include <option.hxx>

namespace RoundExt{

    namespace Auxiliary{
        template<typename T>
	    std::enable_if_t<std::is_floating_point<T>::value, bool> isfinite(T v){
            using lims = std::numeric_limits<T>;
            
            if (std::isinf(v))
                return false;
            if (std::isinf(-v))
                return false;
            if (std::isnan(v))
                return false;
            return true;
        }
        template<typename T>
        std::enable_if_t<std::is_floating_point<T>::value, int> exponent(T x){
            int exp;
            std::frexp(x, &exp);
            return exp - 1;
        }
        auto fld(int D, int d){
            auto res = std::div(D, d);
            int corr = (res.rem != 0 && ((res.rem < 0) != (d < 0)));
            return res.quot - corr;
        }
        auto cld(int D, int d){
            return (D + d - 1) / d;
        }
        int bit_ndigits0z(std::size_t x){
            static const long long unsigned int powers_of_ten[] = {
            0x0000000000000001, 0x000000000000000a, 0x0000000000000064, 0x00000000000003e8,
            0x0000000000002710, 0x00000000000186a0, 0x00000000000f4240, 0x0000000000989680,
            0x0000000005f5e100, 0x000000003b9aca00, 0x00000002540be400, 0x000000174876e800,
            0x000000e8d4a51000, 0x000009184e72a000, 0x00005af3107a4000, 0x00038d7ea4c68000,
            0x002386f26fc10000, 0x016345785d8a0000, 0x0de0b6b3a7640000, 0x8ac7230489e80000,
            };
            std::size_t lz = (sizeof(x)<<3)-__builtin_clz(x);
            int nd = ((1233*lz)>>12)+1;
            nd -= (int)(x < powers_of_ten[nd]);
            return nd;
        }
        template<typename T>
        std::enable_if_t<std::is_integral<T>::value, int> ndigits0znb(T x, int b){
            int d = 0;
            if (std::is_unsigned<T>::value){
                d += (x != 0);
                x = -fld(x, -b);
            }
            while (x != 0){
                x = cld(x, b);
                d += 1;
            }
            return d;
        }
        template<typename T>
        std::enable_if_t<std::is_integral<T>::value, int> ndigits0zpb(T x, int b){
            if (x == 0) return 0;
            x = std::abs(x);
            // __builtin_clz(x) = # of leading zeros
            // __builtin_ctz(x) = # of trailing zeros
            // __builtin_popcount(x) = # of bits set
            switch (b){
                case 2:
                    return (sizeof(x)<<3) - __builtin_clz(x);
                case 8:
                    return ((sizeof(x)<<3) - __builtin_clz(x) + 2) / 3;
                case 16:
                    return (sizeof(x)<<1) - __builtin_clz(x)>>2;
                case 10:
                    return bit_ndigits0z((std::size_t)x);
            }
            if (__builtin_popcount(x) == 1) { // is a power of 2
                int dv = ((sizeof(x)<<3) - __builtin_clz(x)) / __builtin_ctz(x);
                int rm = ((sizeof(x)<<3) - __builtin_clz(x)) % __builtin_ctz(x);
                return iszero(rm) ? dv : dv + 1;
            }

            int d = 0;
            while(x > std::numeric_limits<T>::max()){
                x = x / b;
                d += 1;
            }
            x = x/b;
            d += 1;

            int m = 1;
            while (m <= x){
                m *= b;
                d += 1;
            }
            return d;
        }
        template<typename T>
        std::enable_if_t<std::is_integral<T>::value, int> ndigits0z(T x, int b){
            if (b < -1)
                return ndigits0znb(x, b);
            else if (b > 1)
                return ndigits0zpb(x, b);
            else 
                throw std::runtime_error("Domain Error: The base must not be in `{-1, 0, 1}`.");
        }
    }

    enum RoundingMode{
		RoundNearest,
		RoundNearestTiesAway,
		RoundNearestTiesUp,
		RoundToZero,
		RoundFromZero,
		RoundUp,
		RoundDown
	};

    template<typename T, RoundingMode Mode = RoundDown>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val);

    template<typename T, RoundingMode Mode = RoundDown>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val, int digits);

    template<typename T, RoundingMode Mode = RoundDown>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val, int digits, int sigdigits);

    template<typename T, RoundingMode Mode = RoundDown>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val, Option<int> digits, Option<int> sigdigits, Option<int> base);

    template<RoundingMode Mode, typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, T> _round_invstep(T x, double invstep){
		//jround<Mode>(4.234, nullptr, nullptr, nullptr);
		T y = jround<T, Mode>(x * invstep) / invstep;
		if (!Auxiliary::isfinite((double)y))
			return x;
		return y;
    }

    template<RoundingMode Mode, typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, T> _round_invstepsqrt(T x, double invstepsqrt){
        T y = jround<T, Mode>((x * invstepsqrt) * invstepsqrt) / invstepsqrt / invstepsqrt;
		if (!Auxiliary::isfinite(y))
			return x;
		return y;
    }

    template<RoundingMode Mode, typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, T> _round_step(T x, double step){
        using namespace Auxiliary;
        T y = jround<T, Mode>(x/step) * step;
		if (!isfinite(y)){
				if (x > (T)0){
					return (Mode == RoundUp ? std::numeric_limits<T>::infinity() : (T)0);
				} else if (x < (T)0){
					return (Mode == RoundDown ? -std::numeric_limits<T>::infinity() : (T)0);
				} else{
					return x;
				}
		}
		return y;
    }

    template<typename T>
	std::enable_if_t<std::is_integral<T>::value> hidigit(T x, int base){
        return ndigits0z(x, base);
    }

    template<typename T>
	std::enable_if_t<std::is_floating_point<T>::value, int> hidigit(T x, int base){
        if (iszero(x))
			return 0;
		if (base == 10)
			return 1 + (int)std::floor(std::log10(std::abs(x)));
		else if (base == 2)
			return 1 + Auxiliary::exponent(x);
		else
			return 1 + (int)std::floor(std::log(std::abs(x)) / std::log((double)base));
    }

    template<RoundingMode Mode, typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, T> _round_digits(T x, int digits, int base){
        auto fx = (double)x;
		if (digits >= 0){
			auto invstep = std::pow((double)base, (double)digits);
			if (Auxiliary::isfinite(invstep)){
				return (T)_round_invstep<Mode>(fx, invstep);
			} else {
				invstep = std::pow((double)base, ((double)(digits))/2.0);
				return (T)_round_invstepsqrt<Mode>(fx, invstep);
			}
		} else {
			auto step = std::pow((double)base, (double)(-digits));
			return (T)_round_step<Mode>(fx, step);
		}
    }

    template<RoundingMode Mode, typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, T> _round_sigdigits(T x, int sigdigits, int base){
        auto h = hidigit(x, base);
		return _round_digits<Mode>(x, sigdigits-h, base);
    }

    template<RoundingMode Mode, typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, T> round_generic(T val){
        return val;
    }

    template<typename T, RoundingMode Mode>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val){
        if (Mode == RoundToZero){
            return std::trunc(val);
        } else if (Mode == RoundDown){
            return std::floor(val);
        } else if (Mode == RoundUp){
            return std::ceil(val);
        } else if (Mode == RoundNearest){
            return std::rint(val);
        } else if (Mode == RoundNearestTiesAway){
            return std::round(val);
        } else if (Mode == RoundNearestTiesUp) {
            throw std::runtime_error("Not implemented.");
        } else {
            return jround<T, Mode>(val, OptNone, OptNone, OptNone);
        }

    }

    template<typename T, RoundingMode Mode = RoundDown>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val, int digits){
        return jround<T, Mode>(val, Option<int>(digits), OptNone, OptNone);
    }

    template<typename T, RoundingMode Mode = RoundDown>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val, int digits, int sigdigits){
        return jround<T, Mode>(val, Option<int>(digits), Option<int>(sigdigits), OptNone);
    }

    template<typename T, RoundingMode Mode>
	std::enable_if_t<std::is_arithmetic<T>::value, T> jround(T val, 
        Option<int> digits, 
        Option<int> sigdigits, 
        Option<int> base){
            using namespace Auxiliary;

            if (!digits){
			if (!sigdigits){
				if (!base){
					throw std::runtime_error("MethodError in jround(). Avoiding recursive calls.");
				} else {
					return jround<T, Mode>(val);
				}
			} else {
				if (!isfinite(val))
					return (double)val;
				return _round_sigdigits<Mode>(val, sigdigits.value(), base.is_empty() ? 10 : base.value());
			}
		} else {
			if (!sigdigits){
				if (!isfinite(val))
					return (double)val;
				return _round_digits<Mode>(val, digits.value(), base.is_empty() ? 10 : base.value());
			} else {
				throw std::invalid_argument("`jround` cannot use both `digits` and `sigdigits` arguments.");
			}
		}
        }
}





#endif