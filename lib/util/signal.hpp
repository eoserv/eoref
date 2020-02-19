#ifndef EO_UTIL_SIGNAL_HPP
#define EO_UTIL_SIGNAL_HPP

#include <functional>
#include <list>
#include <mutex>
#include <type_traits>
#include <utility>

// Minimalist sigc::signal replacement, without slots (disconnection) or return values.

namespace util
{
	template <class T> class signal;

	template <class R, class... Args> class signal<R(Args...)>
	{
		static_assert(std::is_same<R, void>::value, "Return type must be void");

		private:
			std::mutex m_mutex;
			std::list<std::function<R(Args...)>> m_slots;

		public:
			signal() = default;

			// no copy/move/assign
			signal(const signal&) = delete;
			const signal& operator=(const signal&) = delete;

			void connect(std::function<R(Args...)>&& fn)
			{
				//std::unique_lock lock(m_mutex);
				m_slots.push_back(std::move(fn));
			}

			void connect(const std::function<R(Args...)>& fn)
			{
				//std::unique_lock lock(m_mutex);
				m_slots.push_back(fn);
			}

			template <class... CallArgs>
			void operator()(CallArgs&&... args) const
			{
				//std::unique_lock lock(m_mutex);

				for (auto&& slot : m_slots)
					slot(std::forward<CallArgs>(args)...);
			}
	};
}

#endif // EO_UTIL_SIGNAL_HPP
