//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/catalog.h"
#include "simpledb/parser.h"
#include "simpledb/planner.h"
#include "simpledb/executor.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

class SelectIntegrationTest : public ::testing::Test {
   protected:
    std::filesystem::path test_data_dir;

    void SetUp() override {
        // Create a unique temporary file path for each test.
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_data_dir = std::filesystem::temp_directory_path().string() + "/simpledb_" + test_info->test_suite_name() +
                        "_" + test_info->name();
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
        std::filesystem::create_directories(test_data_dir);

        catalog::initialize(test_data_dir);

        // Create a sample table with some data for testing.
        command::CreateTableCommand create_cmd;
        create_cmd.table_name = "test_table";
        create_cmd.column_definitions.push_back({"id", command::Datatype::INT});
        create_cmd.column_definitions.push_back({"name", command::Datatype::TEXT});
        create_cmd.column_definitions.push_back({"email", command::Datatype::TEXT});
        executor::execute_create_table_command(create_cmd, test_data_dir);
        // Insert some sample data into the table.
        row::Row row1 = {"1", "Alice", "alice@example.com"};
        row::Row row2 = {"2", "Bob", "bob@example.com"};
        executor::execute_insert_command({"test_table", {"id", "name", "email"}, row1}, test_data_dir);
        executor::execute_insert_command({"test_table", {"id", "name", "email"}, row2}, test_data_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
    }
};

TEST_F(SelectIntegrationTest, SelectAllColumns) {
    std::string query = "SELECT * FROM test_table";
    auto parse_result = parser::parse_sql(query);
    ASSERT_TRUE(parse_result.has_value());

    auto* select_command = std::get_if<ast::SelectCommand>(&(*parse_result));
    ASSERT_TRUE(select_command != nullptr);

    // Create the execution plan.
    auto operator_plan = planner::plan_select(*select_command, test_data_dir);
    ASSERT_TRUE(operator_plan);

    row::Row row1 = {"1", "Alice", "alice@example.com"};
    row::Row row2 = {"2", "Bob", "bob@example.com"};
    std::vector<row::Row> expected_rows = {row1, row2};

    int i = 0;
    while (true) {
        auto row = operator_plan->next();
        if (!row) {
            break;  // No more rows
        }

        ASSERT_EQ(*row, expected_rows[i]);
        i++;
    }
    ASSERT_EQ(i, expected_rows.size());
}

TEST_F(SelectIntegrationTest, SelectSomeColumns) {
    std::string query = "SELECT id, name FROM test_table";
    auto parse_result = parser::parse_sql(query);
    ASSERT_TRUE(parse_result.has_value());

    auto* select_command = std::get_if<ast::SelectCommand>(&(*parse_result));
    ASSERT_TRUE(select_command != nullptr);

    // Create the execution plan.
    auto operator_plan = planner::plan_select(*select_command, test_data_dir);
    ASSERT_TRUE(operator_plan);

    row::Row row1 = {"1", "Alice"};
    row::Row row2 = {"2", "Bob"};
    std::vector<row::Row> expected_rows = {row1, row2};

    int i = 0;
    while (true) {
        auto row = operator_plan->next();
        if (!row) {
            break;  // No more rows
        }

        ASSERT_EQ(*row, expected_rows[i]);
        i++;
    }
    ASSERT_EQ(i, expected_rows.size());
}

TEST_F(SelectIntegrationTest, SelectWithWhereEquals) {
    std::string query = "SELECT * FROM test_table WHERE id = 1";
    auto parse_result = parser::parse_sql(query);
    ASSERT_TRUE(parse_result.has_value());

    auto* select_command = std::get_if<ast::SelectCommand>(&(*parse_result));
    ASSERT_TRUE(select_command != nullptr);

    // Create the execution plan.
    auto operator_plan = planner::plan_select(*select_command, test_data_dir);
    ASSERT_TRUE(operator_plan);

    row::Row expected_row = {"1", "Alice", "alice@example.com"};

    int i = 0;
    while (true) {
        auto row = operator_plan->next();
        if (!row) {
            break;  // No more rows
        }

        ASSERT_EQ(*row, expected_row);
        i++;
    }
    ASSERT_EQ(i, 1);  // Should return exactly 1 row
}

TEST_F(SelectIntegrationTest, SelectWithWhereStringMatch) {
    std::string query = "SELECT id, name FROM test_table WHERE name = 'Bob'";
    auto parse_result = parser::parse_sql(query);
    ASSERT_TRUE(parse_result.has_value());

    auto* select_command = std::get_if<ast::SelectCommand>(&(*parse_result));
    ASSERT_TRUE(select_command != nullptr);

    // Create the execution plan.
    auto operator_plan = planner::plan_select(*select_command, test_data_dir);
    ASSERT_TRUE(operator_plan);

    row::Row expected_row = {"2", "Bob"};

    int i = 0;
    while (true) {
        auto row = operator_plan->next();
        if (!row) {
            break;  // No more rows
        }

        ASSERT_EQ(*row, expected_row);
        i++;
    }
    ASSERT_EQ(i, 1);  // Should return exactly 1 row
}

TEST_F(SelectIntegrationTest, SelectWithWhereNoMatch) {
    std::string query = "SELECT * FROM test_table WHERE name = 'Charlie'";
    auto parse_result = parser::parse_sql(query);
    ASSERT_TRUE(parse_result.has_value());

    auto* select_command = std::get_if<ast::SelectCommand>(&(*parse_result));
    ASSERT_TRUE(select_command != nullptr);

    // Create the execution plan.
    auto operator_plan = planner::plan_select(*select_command, test_data_dir);
    ASSERT_TRUE(operator_plan);

    int i = 0;
    while (true) {
        auto row = operator_plan->next();
        if (!row) {
            break;  // No more rows
        }
        i++;
    }
    ASSERT_EQ(i, 0);  // Should return no rows
}

TEST_F(SelectIntegrationTest, SelectWithWhereComparisonOperators) {
    std::string query = "SELECT name FROM test_table WHERE id != '1'";
    auto parse_result = parser::parse_sql(query);
    ASSERT_TRUE(parse_result.has_value());

    auto* select_command = std::get_if<ast::SelectCommand>(&(*parse_result));
    ASSERT_TRUE(select_command != nullptr);

    // Create the execution plan.
    auto operator_plan = planner::plan_select(*select_command, test_data_dir);
    ASSERT_TRUE(operator_plan);

    row::Row expected_row = {"Bob"};

    int i = 0;
    while (true) {
        auto row = operator_plan->next();
        if (!row) {
            break;  // No more rows
        }

        ASSERT_EQ(*row, expected_row);
        i++;
    }
    ASSERT_EQ(i, 1);  // Should return exactly 1 row (Bob, not Alice)
}
