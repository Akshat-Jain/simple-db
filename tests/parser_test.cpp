#include "simpledb/parser.h"

#include <gtest/gtest.h>

TEST(AntlrParser, ParsesSelectAll) {
    std::string query = "SELECT * FROM users";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<ast::SelectCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "users");
    EXPECT_TRUE(cmd->projection.empty());
}

TEST(AntlrParser, ParsesSelectColumns) {
    std::string query = "SELECT id, name FROM users";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<ast::SelectCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "users");
    std::vector<std::string> expected_cols = {"id", "name"};
    EXPECT_EQ(cmd->projection, expected_cols);
}

TEST(AntlrParser, ParsesCreateTable) {
    std::string query = "CREATE TABLE products (id INT, price TEXT)";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<command::CreateTableCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "products");
    ASSERT_EQ(cmd->column_definitions.size(), 2);
    EXPECT_EQ(cmd->column_definitions[0].column_name, "id");
    EXPECT_EQ(cmd->column_definitions[0].type, command::Datatype::INT);
    EXPECT_EQ(cmd->column_definitions[1].column_name, "price");
    EXPECT_EQ(cmd->column_definitions[1].type, command::Datatype::TEXT);
}

TEST(AntlrParser, ParsesDropTable) {
    std::string query = "DROP TABLE customers";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<command::DropTableCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "customers");
}

TEST(AntlrParser, ParsesShowTables) {
    std::string query = "SHOW TABLES";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());
    ASSERT_NE(std::get_if<command::ShowTablesCommand>(&(*result)), nullptr);
}

TEST(AntlrParser, ParsesInsertWithColumns) {
    std::string query = "INSERT INTO customers (id, name) VALUES ('123', 'ACME Corp')";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<command::InsertCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "customers");
    std::vector<std::string> expected_cols = {"id", "name"};
    EXPECT_EQ(cmd->columns, expected_cols);
    std::vector<std::string> expected_vals = {"123", "ACME Corp"};
    EXPECT_EQ(cmd->values, expected_vals);
}

TEST(AntlrParser, ParsesInsertWithoutColumns) {
    std::string query = "INSERT INTO customers VALUES ('123', 'ACME Corp')";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<command::InsertCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "customers");
    EXPECT_TRUE(cmd->columns.empty());
    std::vector<std::string> expected_vals = {"123", "ACME Corp"};
    EXPECT_EQ(cmd->values, expected_vals);
}

TEST(AntlrParser, HandlesWhitespaceAndCase) {
    std::string query = "   cReAtE    TaBlE   my_table   (   id   iNt  , name    tExT )   ";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());

    auto* cmd = std::get_if<command::CreateTableCommand>(&(*result));
    ASSERT_NE(cmd, nullptr);

    EXPECT_EQ(cmd->table_name, "my_table");
    ASSERT_EQ(cmd->column_definitions.size(), 2);
    EXPECT_EQ(cmd->column_definitions[0].column_name, "id");
    EXPECT_EQ(cmd->column_definitions[0].type, command::Datatype::INT);
}

TEST(AntlrParser, HandlesOptionalSemicolon) {
    std::string query = "SELECT * FROM users;";
    auto result = parser::parse_sql(query);
    ASSERT_TRUE(result.has_value());
    ASSERT_NE(std::get_if<ast::SelectCommand>(&(*result)), nullptr);
}

TEST(AntlrParser, ReturnsNulloptOnInvalidSyntax) {
    // --- Completely Unknown Commands ---
    EXPECT_FALSE(parser::parse_sql("ALTER TABLE my_table ADD COLUMN new_col INT").has_value());
    EXPECT_FALSE(parser::parse_sql("UPDATE my_table SET col1 = 1").has_value());
    EXPECT_FALSE(parser::parse_sql("DELETE FROM my_table").has_value());
    EXPECT_FALSE(parser::parse_sql("EXPLAIN SELECT * FROM my_table").has_value());

    // --- Empty/Whitespace Input ---
    EXPECT_FALSE(parser::parse_sql("").has_value());
    EXPECT_FALSE(parser::parse_sql("    ").has_value());
    EXPECT_FALSE(parser::parse_sql("\t\n").has_value());

    // --- Partial/Incorrect Keywords ---
    EXPECT_FALSE(parser::parse_sql("CREATE mytable (id INT)").has_value());
    EXPECT_FALSE(parser::parse_sql("CREATE").has_value());
    EXPECT_FALSE(parser::parse_sql("INSERT mytable VALUES (1, 'Alice')").has_value());
    EXPECT_FALSE(parser::parse_sql("INSERT").has_value());
    EXPECT_FALSE(parser::parse_sql("DROP mytable").has_value());
    EXPECT_FALSE(parser::parse_sql("DROP").has_value());
    EXPECT_FALSE(parser::parse_sql("SHOW").has_value());

    // --- CREATE TABLE Errors ---
    EXPECT_FALSE(parser::parse_sql("CREATE TABLE (id INT)").has_value());                        // No table name
    EXPECT_FALSE(parser::parse_sql("CREATE TABLE my-table (id INT)").has_value());               // Invalid table name
    EXPECT_FALSE(parser::parse_sql("CREATE TABLE my_table (id SOME_RANDOM_TYPE)").has_value());  // Unknown type
    EXPECT_FALSE(parser::parse_sql("CREATE TABLE my_table (id INT something)").has_value());  // Extra tokens in col def

    // --- DROP TABLE Errors ---
    EXPECT_FALSE(parser::parse_sql("DROP TABLE").has_value());                       // No table name
    EXPECT_FALSE(parser::parse_sql("DROP TABLE my_table extra_token").has_value());  // Extra tokens

    // --- SHOW TABLES Errors ---
    EXPECT_FALSE(parser::parse_sql("SHOW TABLES extra_token").has_value());  // Extra tokens

    // --- INSERT Errors ---
    EXPECT_FALSE(parser::parse_sql("INSERT INTO my_table").has_value());             // Incomplete
    EXPECT_FALSE(parser::parse_sql("INSERT INTO my_table VALUES").has_value());      // No parenthesis
    EXPECT_FALSE(parser::parse_sql("INSERT INTO my_table (id, name)").has_value());  // Missing VALUES

    // --- SELECT Errors ---
    EXPECT_FALSE(parser::parse_sql("SELECT col1, col2 my_table").has_value());                   // Missing FROM
    EXPECT_FALSE(parser::parse_sql("SELECT FROM my_table").has_value());                         // Empty projection
    EXPECT_FALSE(parser::parse_sql("SELECT col1, col2 FROM").has_value());                       // No table name
    EXPECT_FALSE(parser::parse_sql("SELECT col1, col2 FROM my_table extra_token").has_value());  // Extra tokens
}
