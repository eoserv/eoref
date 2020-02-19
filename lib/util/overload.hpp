#ifndef EO_UTIL_OVERLOAD_HPP
#define EO_UTIL_OVERLOAD_HPP

namespace util
{
	template <class... Args> struct overloaded : Args...
	{
		using Args::operator()...;
	};


	template <class... Args>
	auto overload(Args&& ...args)
	{
		return overloaded<Args...>{static_cast<Args&&>(args)...};
	}
}

#endif // EO_UTIL_OVERLOAD_HPP
