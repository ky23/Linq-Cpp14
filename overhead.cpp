#include <cstdlib>
#include <iostream>
#include <ctime>

#include "linq/linq.h"
#include "assert.h"


struct Object : public std::tuple<int, int, int, int> {
	Object(int seed) {
		std::get<0>(*this) = seed;
		std::get<1>(*this) = seed % 1024;
		std::get<2>(*this) = seed / 1024;
		std::get<3>(*this) = seed * 2;
	}
};
struct User
{
	int id;
	int group;
	int category;
	int likes;
	int visits;
	User(int seed)
		: id(seed), group(seed % 1024), category(seed % 128), likes(seed % 8192), visits((seed * 2) % 16384)
	{
	}
};

enum class from
{
	Legacy,
	IEnumerable,
};

enum class which
{
	From,
	Take,
	Skip,
	All,
	Select,
	Where,
	SelectMany,
	GroupBy,
	OrderBy

};

template <typename T>
class Flush_cache : public T
{
public:
	Flush_cache(int const magic)
		: T(magic)
	{}
};

template <typename T>
class Context
{
	const int Max;

	std::vector<Flush_cache<T>> context_;
	const std::vector<Flush_cache<T>> &context_c;

public:
	Context(int max = 200000)
		: Max(max), context_c(context_)
	{
		context_.reserve(Max);
		for (int i = 0; i < Max; ++i)
			context_.push_back({ i });
	}
	auto &get() { return context_; }
	auto &cget() const { return context_c; }
};

template <>
class Context<int>
{
	const int Max;

	std::vector<int> context_;
	const std::vector<int> &context_c;

public:
	Context(int max = 200000)
		: Max(max), context_c(context_)
	{
		context_.reserve(Max);
		for (int i = 0; i < Max; ++i)
			context_.push_back({ i % 12345 });
	}
	auto &get() { return context_; }
	auto &cget() const { return context_c; }
};

template <typename T, from From, which type>
struct Test;

template <>
struct Test<int, from::Legacy, which::From>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Leg->From", [&]() {
			int result = 0;
			for (const auto &it : data)
				result += it;
			return result;
		});
	}
};
template <>
struct Test<int, from::IEnumerable, which::From>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Enu->From", [&]() {
			return linq::make_enumerable(data)
				.Sum();
		});
	}
};
template <>
struct Test<int, from::Legacy, which::Take>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Leg->Take", [&]() {
			int result = 0;
			for (int i = 0; i < 100000; ++i)
				result += data[i];
			return result;
		});
	}
};
template <>
struct Test<int, from::IEnumerable, which::Take>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Enu->Take", [&]() {
			return linq::make_enumerable(data)
				.Take(100000)
				.Sum();
		});
	}
};
template <>
struct Test<int, from::Legacy, which::Skip>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Leg->Skip", [&]() {
			int result = 0;
			int i = 0;
			for (const auto &it : data)
			{
				if (i >= 100000)
					result += it;
				++i;
			}
			return result;
		});
	}
};
template <>
struct Test<int, from::IEnumerable, which::Skip>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Enu->Skip", [&]() {
			return linq::make_enumerable(data)
				.Skip(100000)
				.Sum();
		});
	}
};
template <>
struct Test<int, from::Legacy, which::Where>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Leg->Where", [&]() {
			int result = 0;
			for (const auto &it : data)
			{
				const auto val = it;
				if (val > 1234)
					result += val;
			}
			return result;
		});
	}
};
template <>
struct Test<int, from::IEnumerable, which::Where>
{
	auto operator()() const
	{
		Context<int> context;
		auto &data = context.get();
		return test("Enu->Where", [&]() {
			return linq::make_enumerable(data)
				.Where([](const auto &val) noexcept { return val > 1234; })
				.Sum();
		});
	}
};
template <typename T>
struct Test<T, from::Legacy, which::Select>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();
		return test("Leg->Select", [&]() {
			int result = 0;
			for (const auto &it : data)
				result += std::get<0>(it);
			return result;
		});
	}
};
template <typename T>
struct Test<T, from::IEnumerable, which::Select>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();
		return test("Enu->Select", [&]() {
			return linq::make_enumerable(data)
				.Select([](const auto &val) noexcept -> const auto & { return std::get<0>(val); })
				.Sum();
		});
	}
};
template <typename T>
struct Test<T, from::Legacy, which::Take>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();

		return test("Leg->Select.Take", [&]() {
			int result = 0;
			int i = 0;
			for (const auto &it : data)
			{
				const auto val = std::get<0>(it);
				if (i < 100000)
					result += val;
				else break;
			}
			return result;
		});
	}
};
template <typename T>
struct Test<T, from::IEnumerable, which::Take>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();
		return test("Enu->Select.Take", [&]() {
			return linq::make_enumerable(data)
				.Select([](const auto &val) noexcept -> const auto & { return std::get<0>(val); })
				.Take(100000)
				.Sum();
		});
	}
};
template <typename T>
struct Test<T, from::Legacy, which::Skip>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();

		return test("Leg->Select.Skip", [&]() {
			int result = 0;
			int i = 0;
			for (const auto &it : data)
			{
				const auto val = std::get<0>(it);
				if (i >= 100000)
					result += val;
				++i;
			}
			return result;
		});
	}
};
template <typename T>
struct Test<T, from::IEnumerable, which::Skip>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();
		return test("Enu->Select.Skip", [&]() {
			return linq::make_enumerable(data)
				.Select([](const auto &val) noexcept -> const auto & { return std::get<0>(val); })
				.Skip(100000)
				.Sum();
		});
	}
};
template <typename T>
struct Test<T, from::Legacy, which::Where>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();
		return test("Leg->Select.Where", [&]() {
			int result = 0;
			for (const auto &it : data)
			{
				const auto val = std::get<0>(it);
				if (val > 1234)
					result += val;
			}
			return result;
		});
	}
};
template <typename T>
struct Test<T, from::IEnumerable, which::Where>
{
	auto operator()() const
	{
		Context<T> context;
		auto &data = context.get();
		return test("Enu->Select.Where", [&]() {
			return linq::make_enumerable(data)
				.Select([](const auto &val) noexcept -> const auto & { return std::get<0>(val); })
				.Where([](const auto &val) noexcept { return val > 1234; })
				.Sum();
		});
	}
};

template<>
struct Test<User, from::Legacy, which::GroupBy>
{
	auto operator()() const
	{
		Context<User> context;
		auto &data = context.get();

		return test("Leg->GroupBy", [&]() {
			std::unordered_map<int, std::unordered_map<int, std::vector<User>>> groups;
			for (const auto &it : data)
				groups[it.group][it.category].push_back(it);
			return 0;
		});
	}
};
template <>
struct Test<User, from::IEnumerable, which::GroupBy>
{
	auto operator()() const
	{
		Context<User> context;
		auto &data = context.get();
		return test("Enu->GroupBy", [&]() {
			linq::make_enumerable(data)
				.GroupBy(
					[](const auto &key) noexcept { return key.group; },
					[](const auto &key) noexcept { return key.category; });
			return 0;
		});
	}
};
template<>
struct Test<User, from::Legacy, which::OrderBy>
{
	auto operator()() const
	{
		Context<User> context;
		auto &data = context.get();

		return test("Leg->OrderBy", [&]() noexcept {
			std::sort(data.begin(), data.end(), [](User const &l, User const &r)
			{
				return l.group < r.group || //asc
					(l.group == r.group && l.category > r.category) || //desc
					(l.category == r.category && l.likes < r.likes) || //asc
					(l.likes == r.likes && l.visits > r.visits); //desc
			});
			return 0;
		});
	}
};
template <>
struct Test<User, from::IEnumerable, which::OrderBy>
{
	auto operator()() const
	{
		Context<User> context;
		auto &data = context.get();
		return test("Enu->OrderBy", [&]() {
			linq::make_enumerable(data)
				.OrderBy(
					linq::asc([](const auto &key) noexcept { return key.group; }),
					linq::desc([](const auto &key) noexcept { return key.category; }),
					linq::asc([](const auto &key) noexcept { return key.likes; }),
					linq::desc([](const auto &key) noexcept { return key.visits; })
				);
				return 0;
		});
	}
};



void bench()
{
	Test<int, from::Legacy, which::From>()();
	Test<int, from::IEnumerable, which::From>()();
	Test<int, from::Legacy, which::Take>()();
	Test<int, from::IEnumerable, which::Take>()();
	Test<int, from::Legacy, which::Skip>()();
	Test<int, from::IEnumerable, which::Skip>()();
	Test<int, from::Legacy, which::Where>()();
	Test<int, from::IEnumerable, which::Where>()();
	Test<Object, from::Legacy, which::Select>()();
	Test<Object, from::IEnumerable, which::Select>()();
	Test<Object, from::Legacy, which::Take>()();
	Test<Object, from::IEnumerable, which::Take>()();
	Test<Object, from::Legacy, which::Skip>()();
	Test<Object, from::IEnumerable, which::Skip>()();
	Test<Object, from::Legacy, which::Where>()();
	Test<Object, from::IEnumerable, which::Where>()();
	Test<User, from::Legacy, which::GroupBy>()();
	Test<User, from::IEnumerable, which::GroupBy>()();
	Test<User, from::Legacy, which::OrderBy>()();
	Test<User, from::IEnumerable, which::OrderBy>()();
}

int main(int, char *[])
{
	std::srand(time(0));
	std::cout << "# Overhead Tests" << std::endl;
	bench();

	system("pause");
	return EXIT_SUCCESS;
}
