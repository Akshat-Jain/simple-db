// tests/parser_test.cpp

#include "simpledb/parser.h"

#include <gtest/gtest.h>

TEST(Parser, GetCommandType) {
    ASSERT_EQ(parser::get_command_type("CREATE TABLE my_table (id INT, name TEXT)"), parser::CommandType::CREATE_TABLE);
    ASSERT_EQ(parser::get_command_type("INSERT INTO my_table VALUES (1, 'Alice')"), parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("SELECT * FROM my_table"), parser::CommandType::SELECT);
    ASSERT_EQ(parser::get_command_type("DROP TABLE my_table"), parser::CommandType::DROP_TABLE);
    ASSERT_EQ(parser::get_command_type("SHOW TABLES"), parser::CommandType::SHOW_TABLES);
    ASSERT_EQ(parser::get_command_type("UNKNOWN COMMAND"), parser::CommandType::UNKNOWN);
}

TEST(Parser, GetCommandTypeCaseInsensitive) {
    ASSERT_EQ(parser::get_command_type("create table my_table (id INT, name TEXT)"), parser::CommandType::CREATE_TABLE);
    ASSERT_EQ(parser::get_command_type("insert into my_table VALUES (1, 'Alice')"), parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("select * from my_table"), parser::CommandType::SELECT);
    ASSERT_EQ(parser::get_command_type("drop table my_table"), parser::CommandType::DROP_TABLE);
    ASSERT_EQ(parser::get_command_type("show tables"), parser::CommandType::SHOW_TABLES);
}

TEST(Parser, GetCommandTypeHandlesWhitespace) {
    ASSERT_EQ(parser::get_command_type("  CREATE TABLE t (a INT)"), parser::CommandType::CREATE_TABLE);
    ASSERT_EQ(parser::get_command_type("INSERT INTO t VALUES (1)  "), parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("SELECT   *   FROM    t"), parser::CommandType::SELECT);
    ASSERT_EQ(parser::get_command_type("\tDROP\tTABLE\tt"), parser::CommandType::DROP_TABLE);
    ASSERT_EQ(parser::get_command_type("   SHOW   TABLES   "), parser::CommandType::SHOW_TABLES);
}

TEST(Parser, GetCommandTypeHandlesPartialOrIncorrectKeywords) {
    // First keyword is known, but second (if expected) is missing or wrong
    ASSERT_EQ(parser::get_command_type("CREATE mytable (id INT)"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("CREATE"), parser::CommandType::UNKNOWN);  // CREATE alone

    ASSERT_EQ(parser::get_command_type("INSERT mytable VALUES (1)"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("INSERT"), parser::CommandType::UNKNOWN);  // INSERT alone

    ASSERT_EQ(parser::get_command_type("DROP mytable"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("DROP"), parser::CommandType::UNKNOWN);  // DROP alone

    ASSERT_EQ(parser::get_command_type("SHOW"), parser::CommandType::UNKNOWN);  // SHOW alone
}

TEST(Parser, GetCommandTypeHandlesCompletelyUnknownAndEmpty) {
    ASSERT_EQ(parser::get_command_type("ALTER TABLE my_table ADD COLUMN new_col INT"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("UPDATE my_table SET col1 = 1"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("DELETE FROM my_table"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("EXPLAIN SELECT * FROM my_table"), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type(""), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("    "), parser::CommandType::UNKNOWN);
    ASSERT_EQ(parser::get_command_type("\t\n"), parser::CommandType::UNKNOWN);
}

TEST(CreateTable, BasicCreateTable) {
    std::string query = "CREATE TABLE my_table (id INT, name TEXT)";
    auto parse_result = parser::parse_create_table(query);
    ASSERT_TRUE(parse_result);
    ASSERT_TRUE(parse_result.command.has_value());

    std::optional<command::CreateTableCommand> table_command = parse_result.command;
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
    auto parse_result = parser::parse_create_table(query);
    ASSERT_TRUE(parse_result);
    ASSERT_TRUE(parse_result.command.has_value());

    std::optional<command::CreateTableCommand> table_command = parse_result.command;
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
    ASSERT_FALSE(parser::parse_create_table(query));
}

TEST(CreateTable, MissingTableKeyword) {
    std::string query = "CREATE my_table (id INT)";
    ASSERT_FALSE(parser::parse_create_table(query));
}

TEST(CreateTable, NoTableName) {
    std::string query = "CREATE TABLE (id INT)";
    ASSERT_FALSE(parser::parse_create_table(query));
}

TEST(CreateTable, InvalidTableNameCharacters) {
    std::string query = "CREATE TABLE my-table (id INT)";
    ASSERT_FALSE(parser::parse_create_table(query));
}

TEST(CreateTable, UnknownColumnType) {
    std::string query = "CREATE TABLE my_table (id SOME_RANDOM_TYPE)";
    ASSERT_FALSE(parser::parse_create_table(query));
}

TEST(CreateTable, ExtraTokensInColumnDefinition) {
    std::string query = "CREATE TABLE my_table (id INT something)";
    ASSERT_FALSE(parser::parse_create_table(query));
}

TEST(DropTable, BasicDropTable) {
    auto parse_result = parser::parse_drop_table("DROP TABLE my_table");
    ASSERT_TRUE(parse_result);
    ASSERT_TRUE(parse_result.command.has_value());

    std::optional<command::DropTableCommand> cmd = parse_result.command;
    ASSERT_TRUE(cmd.has_value());
    ASSERT_EQ("my_table", cmd->table_name);
}

TEST(DropTable, ExtraWhitespace) {
    auto parse_result = parser::parse_drop_table("   DROP   TABLE   my_table   ");
    ASSERT_TRUE(parse_result);
    ASSERT_TRUE(parse_result.command.has_value());

    std::optional<command::DropTableCommand> cmd = parse_result.command;
    ASSERT_TRUE(cmd.has_value());
    ASSERT_EQ("my_table", cmd->table_name);
}

TEST(DropTable, MissingDropKeyword) {
    std::string query = "TABLE my_table";
    ASSERT_FALSE(parser::parse_drop_table(query));
}

TEST(DropTable, MissingTableKeyword) {
    std::string query = "DROP my_table";
    ASSERT_FALSE(parser::parse_drop_table(query));
}

TEST(DropTable, NoTableName) {
    std::string query = "DROP TABLE";
    ASSERT_FALSE(parser::parse_drop_table(query));
}

TEST(DropTable, InvalidTableNameCharacters) {
    std::string query = "DROP TABLE my-table";
    ASSERT_FALSE(parser::parse_drop_table(query));
}

TEST(DropTable, ExtraTokens) {
    std::string query = "DROP TABLE my_table extra_token";
    ASSERT_FALSE(parser::parse_drop_table(query));
}

TEST(ShowTables, BasicShowTables) {
    auto parse_result = parser::parse_show_tables("SHOW TABLES");
    ASSERT_TRUE(parse_result);
    ASSERT_TRUE(parse_result.command.has_value());

    std::optional<command::ShowTablesCommand> cmd = parse_result.command;
    ASSERT_TRUE(cmd.has_value());
}

TEST(ShowTables, ExtraWhitespace) {
    auto parse_result = parser::parse_show_tables("   SHOW   TABLES   ");
    ASSERT_TRUE(parse_result);
    ASSERT_TRUE(parse_result.command.has_value());

    std::optional<command::ShowTablesCommand> cmd = parse_result.command;
    ASSERT_TRUE(cmd.has_value());
}

TEST(ShowTables, MissingTablesKeyword) {
    std::string query = "SHOW";
    ASSERT_FALSE(parser::parse_show_tables(query));
}

TEST(ShowTables, ExtraTokens) {
    std::string query = "SHOW TABLES extra_token";
    ASSERT_FALSE(parser::parse_show_tables(query));
}
