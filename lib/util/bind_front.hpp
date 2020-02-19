#ifndef EO_BIND_FRONT_HPP
#define EO_BIND_FRONT_HPP

// Replacement for C++20 bind_front

#include <type_traits>
#include <utility>

namespace util
{
	template <class F>
	auto bind_front(F&& f)
	{
		return std::forward<F&&>(f);
	}

	template <class F, class FirstBindArg, class... BindArgs>
	auto bind_front(F&& f, FirstBindArg&& first_bind_arg, BindArgs&&... bind_args)
	{
		// Special case for binding this-ptr
		if constexpr (std::is_member_function_pointer_v<F>)
		{
			return [f, first_bind_arg, bind_args...](auto&&... args)
			{
				return (first_bind_arg->*f)(
					std::forward<BindArgs&&...>(bind_args)...,
					std::forward<decltype(args)&&...>(args)...
				);
			};
		}
		else
		{
			return [f, first_bind_arg, bind_args...](auto&&... args)
			{
				return f(
					std::forward<FirstBindArg&&>(first_bind_arg),
					std::forward<BindArgs&&...>(bind_args)...,
					std::forward<decltype(args)&&...>(args)...
				);
			};
		}
	}
}

#endif // EO_BIND_FRONT_HPP
