//
// Created by Akshat Jain on 01/06/25.
//

#include <utility>

#include "simpledb/result.h"

namespace results {
    ExecutionResult::ExecutionResult(ResultStatus status,
                                     std::optional<std::string> message,
                                     std::optional<ResultSet> data)
        : _result_status{status}, _message{std::move(message)}, _data{std::move(data)} {}

    ExecutionResult ExecutionResult::Ok(std::string msg) {
        return {ResultStatus::SUCCESS, std::move(msg), std::nullopt};
    }

    ExecutionResult ExecutionResult::Error(std::string error_msg) {
        return {ResultStatus::ERROR, std::move(error_msg), std::nullopt};
    }

    ExecutionResult ExecutionResult::SuccessWithData(ResultSet data, std::optional<std::string> msg) {
        return {ResultStatus::SUCCESS, std::move(msg), data};
    }

    ResultStatus ExecutionResult::get_status() const { return _result_status; }

    std::optional<std::string> ExecutionResult::get_message() const { return _message; }

    bool ExecutionResult::has_data() const { return _data.has_value(); }

    const ResultSet& ExecutionResult::get_data() const {
        if (!_data.has_value()) {
            throw std::logic_error("ExecutionResult: Attempted to get data when no data is present.");
        }
        return _data.value();
    }

}  // namespace results
