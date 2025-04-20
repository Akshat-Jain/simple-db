// tests/parser_test.cpp

#include "simpledb/parser.h"

#include <gtest/gtest.h>

TEST(CreateTable, BasicCreateTable) {
    std::string query = "CREATE TABLE my_table (id INT, name TEXT)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);

    ASSERT_TRUE(table_command.has_value());
    ASSERT_EQ("my_table", table_command->table_name);
    ASSERT_EQ(2, table_command->column_definitions.size());
    ASSERT_EQ("id", table_command->column_definitions[0].column_name);
    ASSERT_EQ(command::Datatype::INT, table_command->column_definitions[0].type);
    ASSERT_EQ("name", table_command->column_definitions[1].column_name);
    ASSERT_EQ(command::Datatype::TEXT, table_command->column_definitions[1].type);
}

TEST(CreateTable, ExtraWhitespace) {
    std::string query = "   CREATE    TABLE   my_table   (   id   INT  , name    TEXT )   ";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);

    ASSERT_TRUE(table_command.has_value());
    ASSERT_EQ("my_table", table_command->table_name);
    ASSERT_EQ(2, table_command->column_definitions.size());
    ASSERT_EQ("id", table_command->column_definitions[0].column_name);
    ASSERT_EQ(command::Datatype::INT, table_command->column_definitions[0].type);
    ASSERT_EQ("name", table_command->column_definitions[1].column_name);
    ASSERT_EQ(command::Datatype::TEXT, table_command->column_definitions[1].type);
}

TEST(CreateTable, MissingCreateKeyword) {
    std::string query = "TABLE my_table (id INT)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);
    ASSERT_FALSE(table_command.has_value());
}

TEST(CreateTable, MissingTableKeyword) {
    std::string query = "CREATE my_table (id INT)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);
    ASSERT_FALSE(table_command.has_value());
}

TEST(CreateTable, NoTableName) {
    std::string query = "CREATE TABLE (id INT)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);
    ASSERT_FALSE(table_command.has_value());
}

TEST(CreateTable, InvalidTableNameCharacters) {
    std::string query = "CREATE TABLE my-table (id INT)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);
    ASSERT_FALSE(table_command.has_value());
}

TEST(CreateTable, UnknownColumnType) {
    std::string query = "CREATE TABLE my_table (id SOME_RANDOM_TYPE)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);
    ASSERT_FALSE(table_command.has_value());
}

TEST(CreateTable, ExtraTokensInColumnDefinition) {
    std::string query = "CREATE TABLE my_table (id INT something)";
    std::optional<command::CreateTableCommand> table_command = parser::parse_create_table(query);
    ASSERT_FALSE(table_command.has_value());
}
