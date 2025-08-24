//
// Created by Akshat Jain on 24/08/25.
//

#include "simpledb/execution/filter_operator.h"
#include "simpledb/execution/operator.h"
#include "simpledb/execution/row.h"
#include "simpledb/catalog.h"
#include "simpledb/ast/ast.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief A mock operator for testing purposes.
 *
 * This class inherits from the Operator interface and simulates a data source.
 * It is initialized with a vector of rows and yields them one by one when its
 * next() method is called, just like a real TableScanOperator would.
 */
class MockScanOperator : public simpledb::execution::Operator {
   public:
    // Constructor takes the data it should produce.
    explicit MockScanOperator(std::vector<row::Row> data) : data_(std::move(data)), current_index_(0) {}

    // The next() method returns the next row from the internal vector.
    std::optional<row::Row> next() override {
        if (current_index_ < data_.size()) {
            // If there are still rows left, return the current one
            // and advance the index for the next call.
            return data_[current_index_++];
        }
        // If we've exhausted the data, return nullopt.
        return std::nullopt;
    }

   private:
    std::vector<row::Row> data_;
    size_t current_index_;
};

class FilterOperatorTest : public ::testing::Test {
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
    }

    void TearDown() override {
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
    }
};

TEST_F(FilterOperatorTest, FilterWithEqualsMatch) {
    // Add the schema to the catalog so the FilterOperator can find it.
    std::string table_name = "users";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Create source data
    std::vector<row::Row> source_data = {
        {"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}, {"3", "Charlie", "charlie@example.com"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause: WHERE id = '2'
    ast::WhereClause where_clause;
    where_clause.column_name = "id";
    where_clause.op = ast::ComparisonOp::EQUALS;
    where_clause.value = "2";

    // Create FilterOperator
    simpledb::execution::FilterOperator filter_op(table_name, std::move(mock_scan), where_clause);

    // Should return only Bob's row
    auto first_row = filter_op.next();
    ASSERT_TRUE(first_row.has_value());
    row::Row expected = {"2", "Bob", "bob@example.com"};
    ASSERT_EQ(*first_row, expected);

    // Should have no more rows
    ASSERT_FALSE(filter_op.next().has_value());
}

TEST_F(FilterOperatorTest, FilterWithStringMatch) {
    // Add the schema to the catalog
    std::string table_name = "users";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Create source data
    std::vector<row::Row> source_data = {
        {"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}, {"3", "Alice", "alice2@example.com"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause: WHERE name = 'Alice'
    ast::WhereClause where_clause;
    where_clause.column_name = "name";
    where_clause.op = ast::ComparisonOp::EQUALS;
    where_clause.value = "Alice";

    // Create FilterOperator
    simpledb::execution::FilterOperator filter_op(table_name, std::move(mock_scan), where_clause);

    // Should return first Alice row
    auto first_row = filter_op.next();
    ASSERT_TRUE(first_row.has_value());
    row::Row expected_first = {"1", "Alice", "alice@example.com"};
    ASSERT_EQ(*first_row, expected_first);

    // Should return second Alice row
    auto second_row = filter_op.next();
    ASSERT_TRUE(second_row.has_value());
    row::Row expected_second = {"3", "Alice", "alice2@example.com"};
    ASSERT_EQ(*second_row, expected_second);

    // Should have no more rows
    ASSERT_FALSE(filter_op.next().has_value());
}

TEST_F(FilterOperatorTest, FilterWithNoMatches) {
    // Add the schema to the catalog
    std::string table_name = "users";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Create source data
    std::vector<row::Row> source_data = {{"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause: WHERE name = 'Charlie'
    ast::WhereClause where_clause;
    where_clause.column_name = "name";
    where_clause.op = ast::ComparisonOp::EQUALS;
    where_clause.value = "Charlie";

    // Create FilterOperator
    simpledb::execution::FilterOperator filter_op(table_name, std::move(mock_scan), where_clause);

    // Should have no rows
    ASSERT_FALSE(filter_op.next().has_value());
}

TEST_F(FilterOperatorTest, FilterWithNotEqualsOperator) {
    // Add the schema to the catalog
    std::string table_name = "users";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Create source data
    std::vector<row::Row> source_data = {
        {"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}, {"3", "Charlie", "charlie@example.com"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause: WHERE id != '2'
    ast::WhereClause where_clause;
    where_clause.column_name = "id";
    where_clause.op = ast::ComparisonOp::NOT_EQUALS;
    where_clause.value = "2";

    // Create FilterOperator
    simpledb::execution::FilterOperator filter_op(table_name, std::move(mock_scan), where_clause);

    // Should return Alice's row
    auto first_row = filter_op.next();
    ASSERT_TRUE(first_row.has_value());
    row::Row expected_first = {"1", "Alice", "alice@example.com"};
    ASSERT_EQ(*first_row, expected_first);

    // Should return Charlie's row
    auto second_row = filter_op.next();
    ASSERT_TRUE(second_row.has_value());
    row::Row expected_second = {"3", "Charlie", "charlie@example.com"};
    ASSERT_EQ(*second_row, expected_second);

    // Should have no more rows
    ASSERT_FALSE(filter_op.next().has_value());
}

TEST_F(FilterOperatorTest, FilterWithAllComparisonOperators) {
    // Add the schema to the catalog
    std::string table_name = "products";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"price", command::Datatype::INT}};
    catalog::add_table(schema);

    // Test each comparison operator
    struct TestCase {
        ast::ComparisonOp op;
        std::string column;
        std::string value;
        std::vector<std::string> expected_ids;
    };

    std::vector<TestCase> test_cases = {{ast::ComparisonOp::EQUALS, "price", "20", {"2"}},
                                        {ast::ComparisonOp::NOT_EQUALS, "price", "20", {"1", "3"}},
                                        {ast::ComparisonOp::LESS_THAN, "price", "20", {"1"}},
                                        {ast::ComparisonOp::LESS_THAN_OR_EQUAL, "price", "20", {"1", "2"}},
                                        {ast::ComparisonOp::GREATER_THAN, "price", "20", {"3"}},
                                        {ast::ComparisonOp::GREATER_THAN_OR_EQUAL, "price", "20", {"2", "3"}}};

    for (const auto& test_case : test_cases) {
        // Create fresh source data for each test
        std::vector<row::Row> source_data = {
            {"1", "Product A", "10"},  // price < 20
            {"2", "Product B", "20"},  // price = 20
            {"3", "Product C", "30"}   // price > 20
        };
        auto mock_scan = std::make_unique<MockScanOperator>(source_data);

        // Create WHERE clause
        ast::WhereClause where_clause;
        where_clause.column_name = test_case.column;
        where_clause.op = test_case.op;
        where_clause.value = test_case.value;

        // Create FilterOperator
        simpledb::execution::FilterOperator filter_op(table_name, std::move(mock_scan), where_clause);

        // Collect all returned rows
        std::vector<std::string> actual_ids;
        while (true) {
            auto row = filter_op.next();
            if (!row.has_value()) {
                break;
            }
            actual_ids.push_back((*row)[0]);  // Get the ID column
        }

        // Verify results
        ASSERT_EQ(actual_ids, test_case.expected_ids)
            << "Failed for operator " << static_cast<int>(test_case.op) << " with value " << test_case.value;
    }
}

TEST_F(FilterOperatorTest, FilterWithEmptyInput) {
    // Add the schema to the catalog
    std::string table_name = "empty_table";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {{"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Create empty source data
    std::vector<row::Row> source_data = {};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause
    ast::WhereClause where_clause;
    where_clause.column_name = "id";
    where_clause.op = ast::ComparisonOp::EQUALS;
    where_clause.value = "1";

    // Create FilterOperator
    simpledb::execution::FilterOperator filter_op(table_name, std::move(mock_scan), where_clause);

    // Should have no rows
    ASSERT_FALSE(filter_op.next().has_value());
}

TEST_F(FilterOperatorTest, FilterThrowsOnNonExistentColumn) {
    // Add the schema to the catalog
    std::string table_name = "users";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {{"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Create source data
    std::vector<row::Row> source_data = {{"1", "Alice"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause with non-existent column
    ast::WhereClause where_clause;
    where_clause.column_name = "non_existent_column";
    where_clause.op = ast::ComparisonOp::EQUALS;
    where_clause.value = "value";

    // Should throw when constructing FilterOperator
    ASSERT_THROW(simpledb::execution::FilterOperator(table_name, std::move(mock_scan), where_clause),
                 std::runtime_error);
}

TEST_F(FilterOperatorTest, FilterThrowsOnNonExistentTable) {
    // Don't add any schema to the catalog

    // Create source data
    std::vector<row::Row> source_data = {{"1", "Alice"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // Create WHERE clause
    ast::WhereClause where_clause;
    where_clause.column_name = "id";
    where_clause.op = ast::ComparisonOp::EQUALS;
    where_clause.value = "1";

    // Should throw when constructing FilterOperator
    ASSERT_THROW(simpledb::execution::FilterOperator("non_existent_table", std::move(mock_scan), where_clause),
                 std::runtime_error);
}