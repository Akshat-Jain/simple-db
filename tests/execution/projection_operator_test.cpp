//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/planner.h"
#include "simpledb/execution/operator.h"
#include "simpledb/execution/row.h"
#include "simpledb/execution/projection_operator.h"
#include "simpledb/catalog.h"

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

class ProjectionOperatorTest : public ::testing::Test {
   protected:
    std::filesystem::path test_data_dir;

    void SetUp() override {
        // Create a unique temporary file path for each test.
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_data_dir = std::filesystem::temp_directory_path().string() + "simpledb_" + test_info->test_suite_name() +
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

TEST_F(ProjectionOperatorTest, ProjectAllColumns) {
    // Add the schema to the catalog so the ProjectionOperator can find it.
    std::string table_name = "table_name";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Action: Create the pipeline.
    // 1. Create the mock operator with the mock source data.
    row::Row first_row = {"1", "Alice", "alice@example.com"};
    row::Row second_row = {"2", "Bob", "bob@example.com"};
    std::vector<row::Row> source_data = {{"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // 2. Create the ProjectionOperator, giving it the mock operator as its child.
    simpledb::execution::ProjectionOperator proj_op(table_name, std::move(mock_scan), {});

    // Assert first row
    ASSERT_EQ(*proj_op.next(), first_row);

    // Assert second row
    ASSERT_EQ(*proj_op.next(), second_row);

    // Assert that there are no more rows.
    ASSERT_FALSE(proj_op.next().has_value());
}

TEST_F(ProjectionOperatorTest, ProjectSpecificColumns) {
    // Add the schema to the catalog so the ProjectionOperator can find it.
    std::string table_name = "table_name";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Action: Create the pipeline.
    // 1. Create the mock operator with the mock source data.
    std::vector<row::Row> source_data = {{"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // 2. Create the ProjectionOperator, giving it the mock operator as its child.
    std::vector<std::string> projection_cols = {"email", "id"};
    simpledb::execution::ProjectionOperator proj_op(table_name, std::move(mock_scan), projection_cols);

    // Assert first row
    auto first_row = proj_op.next();
    ASSERT_TRUE(first_row.has_value());
    row::Row expected_first = {"alice@example.com", "1"};
    ASSERT_EQ(*first_row, expected_first);

    // Assert second row
    auto second_row = proj_op.next();
    ASSERT_TRUE(second_row.has_value());
    row::Row expected_second = {"bob@example.com", "2"};
    ASSERT_EQ(*second_row, expected_second);

    // Assert that there are no more rows.
    ASSERT_FALSE(proj_op.next().has_value());
}

TEST_F(ProjectionOperatorTest, ProjectWithNoRows) {
    // Add the schema to the catalog so the ProjectionOperator can find it.
    std::string table_name = "empty_table";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Action: Create the pipeline.
    // 1. Create the mock operator with no data.
    std::vector<row::Row> source_data = {};
    auto mock_scan = std::make_unique<MockScanOperator>(source_data);

    // 2. Create the ProjectionOperator, giving it the mock operator as its child.
    simpledb::execution::ProjectionOperator proj_op(table_name, std::move(mock_scan), {});

    // Assert that there are no rows.
    ASSERT_FALSE(proj_op.next().has_value());
}

TEST_F(ProjectionOperatorTest, ProjectWithNonExistentColumn) {
    // Add the schema to the catalog so the ProjectionOperator can find it.
    std::string table_name = "table_name";
    catalog::TableSchema schema;
    schema.table_name = table_name;
    schema.column_definitions = {
        {"id", command::Datatype::INT}, {"name", command::Datatype::TEXT}, {"email", command::Datatype::TEXT}};
    catalog::add_table(schema);

    // Action: Create the pipeline.
    // 1. Create the mock operator with some data.
    std::vector<row::Row> source_data = {{"1", "Alice", "alice@example.com"}, {"2", "Bob", "bob@example.com"}};

    auto mock_scan = std::make_unique<MockScanOperator>(source_data);
    // 2. Create the ProjectionOperator, giving it the mock operator as its child.
    std::vector<std::string> projection_cols = {"email", "non_existent_column"};
    try {
        // This is the line that is expected to throw.
        simpledb::execution::ProjectionOperator proj_op(table_name, std::move(mock_scan), projection_cols);
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ("Projection column not found in table schema: non_existent_column", e.what());
    }
}
