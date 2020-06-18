#pragma once

namespace Typhoon {

namespace detail {

JobId createJobImpl(JobFunction function, const void* data = nullptr, size_t dataSize = 0);
JobId createChildJobImpl(JobId parent, JobFunction function, const void* data = nullptr, size_t dataSize = 0);
JobId addContinuationImpl(JobId job, JobFunction function, const void* data, size_t dataSize);

struct ParallelForJobData {
	ParallelForFunction function;
	uint32_t            splitThreshold;
	uint32_t            offset;
	uint32_t            count;
	char                functionArgs[24];
};

void parallel_for_job(const JobParams& prm);

} // namespace detail

template <typename... ArgType>
JobId createJob(JobFunction function, ArgType... args) {
	static_assert((std::is_pod_v<ArgType> && ... && true));

	auto argTuple = std::make_tuple(args...);
	return createJobImpl(function, &argTuple, sizeof argTuple);
}

template <typename... ArgType>
JobId createChildJob(JobId parent, JobFunction function, ArgType... args) {
	static_assert((std::is_pod_v<ArgType> && ... && true));

	auto argTuple = std::make_tuple(args...);
	return detail::createChildJobImpl(parent, function, &argTuple, sizeof argTuple);
}

template <typename... ArgType>
void startChildJob(JobId parent, JobFunction function, ArgType... args) {
	JobId job = createChildJob(parent, function, args...);
	startJob(job);
}

template <typename... ArgType>
JobId addContinuation(JobId job, JobFunction function, ArgType... args) {
	static_assert((std::is_pod_v<ArgType> && ... && true));

	auto argTuple = std::make_tuple(args...);
	return detail::addContinuationImpl(job, function, &argTuple, sizeof argTuple);
}

template <typename... ArgType>
JobId parallelFor(JobId parent, size_t elementCount, size_t splitThreshold, ParallelForFunction function, const ArgType&... args) {
	static_assert((std::is_pod_v<ArgType> && ... && true));

	auto                       argTuple = std::make_tuple(args...);
	detail::ParallelForJobData jobData { function, (uint32_t)splitThreshold, 0, (uint32_t)elementCount };
	// Store extra arguments in the job data
	static_assert(sizeof argTuple <= sizeof jobData.functionArgs);
	std::memcpy(jobData.functionArgs, &argTuple, sizeof argTuple);
	return detail::createChildJobImpl(parent, detail::parallel_for_job, &jobData, sizeof jobData);
}

template <typename ArgType>
ArgType unpackJobArg(const void* args) {
	static_assert((std::is_pod_v<ArgType>));

	ArgType arg;
	std::memcpy(&arg, args, sizeof arg);
	return arg;
}

template <typename... ArgType>
std::tuple<ArgType...> unpackJobArgs(const void* args) {
	static_assert((std::is_pod_v<ArgType> && ... && true));

	std::tuple<ArgType...> tuple;
	std::memcpy(&tuple, args, sizeof tuple);
	return tuple;
}

} // namespace Typhoon