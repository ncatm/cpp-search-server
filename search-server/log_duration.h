#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, out) LogDuration UNIQUE_VAR_NAME_PROFILE(x, out)


class LogDuration {
public:

	using Clock = std::chrono::steady_clock;

	LogDuration(const std::string log, std::ostream& stream) :
		log_(log), stream_(stream)
	{}

	~LogDuration() {
		using namespace std::chrono;
		using namespace std::literals;
		
		const auto end_time = Clock::now();
		const auto duration = end_time - start_time_;
		std::cerr << log_ << ": " << duration_cast<milliseconds>(duration).count() << "ms: " << std::endl;
	}

private:
	const std::string log_;
	std::ostream& stream_ = std::cerr;
	const Clock::time_point start_time_ = Clock::now();
};