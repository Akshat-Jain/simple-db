//
// Created by Akshat Jain on 01/06/25.
//

#ifndef SIMPLE_DB_RESULT_H
#define SIMPLE_DB_RESULT_H

#include "simpledb/execution/row.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace results {
    enum class ResultStatus { SUCCESS, ERROR };

    struct ResultSet {
        std::vector<std::string> headers;
        std::vector<row::Row> rows;
    };

    inline bool operator==(const ResultSet& lhs, const ResultSet& rhs) {
        return lhs.headers == rhs.headers && lhs.rows == rhs.rows;
    }

    inline std::ostream& operator<<(std::ostream& os, const ResultSet& rs) {
        os << "ResultSet(";
        os << "Headers: [";
        for (size_t i = 0; i < rs.headers.size(); i++) {
            os << rs.headers[i];
            if (i < rs.headers.size() - 1) {
                os << ", ";
            }
        }
        os << "], ";
        os << "Rows: [";
        for (size_t i = 0; i < rs.rows.size(); i++) {
            os << "[";
            for (size_t j = 0; j < rs.rows[i].size(); j++) {
                os << rs.rows[i][j];
                if (j < rs.rows[i].size() - 1) {
                    os << ", ";
                }
            }
            os << "]";
            if (i < rs.rows.size() - 1) {
                os << ", ";
            }
        }
        os << "])";
        return os;
    }

    class ExecutionResult {
       public:
        // --- Static Factory Methods (Declarations) ---
        static ExecutionResult Ok(std::string msg);
        static ExecutionResult Error(std::string error_msg);
        static ExecutionResult SuccessWithData(ResultSet data, std::optional<std::string> msg = std::nullopt);

        // --- Const Accessor Methods (Declarations, can be defined inline if simple) ---
        ResultStatus get_status() const;                 // Declaration
        std::optional<std::string> get_message() const;  // Declaration
        bool has_data() const;                           // Declaration
        const ResultSet& get_data() const;               // Declaration

       private:
        ExecutionResult(ResultStatus status, std::optional<std::string> message, std::optional<ResultSet> data);
        ResultStatus _result_status;
        std::optional<std::string> _message;
        std::optional<ResultSet> _data;

        // todo: More fields like _execution_time_ms, _num_result_rows, _num_rows_affected, etc. can be added as needed.

       public:
    };
}  // namespace results

#endif  // SIMPLE_DB_RESULT_H
