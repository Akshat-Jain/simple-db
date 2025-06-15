// tests/parser_test.cpp

#include "simpledb/parser.h"

#include <gtest/gtest.h>

TEST(Parser, GetCommandType) {
    ASSERT_EQ(parser::get_command_type("CREATE TABLE my_table (id INT, name TEXT)"), parser::CommandType::CREATE_TABLE);
    ASSERT_EQ(parser::get_command_type("INSERT INTO my_table VALUES (1, 'Alice')"), parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("SELECT * FROM my_table"), parser::CommandType::SELECT);
    ASSERT_EQ(parser::get_command_type("DROP TABLE my_table"), parser::CommandType::DROP_TABLE);
    ASSERT_EQ(parser::get_command_type("SHOW TABLES"), parser::CommandType::SHOW_TABLES);
    ASSERT_EQ(parser::get_command_type("INSERT INTO my_table (id, name) VALUES (1, 'Alice')"),
              parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("UNKNOWN COMMAND"), parser::CommandType::UNKNOWN);
}

TEST(Parser, GetCommandTypeCaseInsensitive) {
    ASSERT_EQ(parser::get_command_type("create table my_table (id INT, name TEXT)"), parser::CommandType::CREATE_TABLE);
    ASSERT_EQ(parser::get_command_type("insert into my_table VALUES (1, 'Alice')"), parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("select * from my_table"), parser::CommandType::SELECT);
    ASSERT_EQ(parser::get_command_type("drop table my_table"), parser::CommandType::DROP_TABLE);
    ASSERT_EQ(parser::get_command_type("show tables"), parser::CommandType::SHOW_TABLES);
    ASSERT_EQ(parser::get_command_type("insert into my_table (id, name) VALUES (1, 'Alice')"),
              parser::CommandType::INSERT);
}

TEST(Parser, GetCommandTypeHandlesWhitespace) {
    ASSERT_EQ(parser::get_command_type("  CREATE TABLE t (a INT)"), parser::CommandType::CREATE_TABLE);
    ASSERT_EQ(parser::get_command_type("INSERT INTO t VALUES (1)  "), parser::CommandType::INSERT);
    ASSERT_EQ(parser::get_command_type("SELECT   *   FROM    t"), parser::CommandType::SELECT);
    ASSERT_EQ(parser::get_command_type("\tDROP\tTABLE\tt"), parser::CommandType::DROP_TABLE);
    ASSERT_EQ(parser::get_command_type("   SHOW   TABLES   "), parser::CommandType::SHOW_TABLES);
    ASSERT_EQ(parser::get_command_type("INSERT INTO t (a) VALUES (1)  "), parser::CommandType::INSERT);
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

    std::optional<command::CreateTableCommand> table_command = parse_result.command;
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

    std::optional<command::CreateTableCommand> table_command = parse_result.command;
    ASSERT_EQ("my_table", table_command->table_name);
    ASSERT_EQ(2, table_command->column_definitions.size());
    ASSERT_EQ("id", table_command->column_definitions[0].column_name);
    ASSERT_EQ(command::Datatype::INT, table_command->column_definitions[0].type);
    ASSERT_EQ("name", table_command->column_definitions[1].column_name);
    ASSERT_EQ(command::Datatype::TEXT, table_command->column_definitions[1].type);
}

TEST(CreateTable, MissingCreateKeyword) {
    auto result = parser::parse_create_table("my_table (id INT)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected CREATE keyword.", result.error_message);
}

TEST(CreateTable, MissingTableKeyword) {
    auto result = parser::parse_create_table("CREATE my_table (id INT)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected TABLE keyword.", result.error_message);
}

TEST(CreateTable, NoTableName) {
    auto result = parser::parse_create_table("CREATE TABLE (id INT)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Table name is empty.", result.error_message);
}

TEST(CreateTable, InvalidTableNameCharacters) {
    auto result = parser::parse_create_table("CREATE TABLE my-table (id INT)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Table name 'my-table' contains invalid characters.", result.error_message);
}

TEST(CreateTable, UnknownColumnType) {
    auto result = parser::parse_create_table("CREATE TABLE my_table (id SOME_RANDOM_TYPE)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Unknown column type 'SOME_RANDOM_TYPE'. Supported types are INT and TEXT.", result.error_message);
}

TEST(CreateTable, ExtraTokensInColumnDefinition) {
    auto result = parser::parse_create_table("CREATE TABLE my_table (id INT something)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Extra tokens after column type in column definition: [id INT something].", result.error_message);
}

TEST(DropTable, BasicDropTable) {
    auto parse_result = parser::parse_drop_table("DROP TABLE my_table");
    ASSERT_TRUE(parse_result);

    std::optional<command::DropTableCommand> cmd = parse_result.command;
    ASSERT_EQ("my_table", cmd->table_name);
}

TEST(DropTable, ExtraWhitespace) {
    auto parse_result = parser::parse_drop_table("   DROP   TABLE   my_table   ");
    ASSERT_TRUE(parse_result);

    std::optional<command::DropTableCommand> cmd = parse_result.command;
    ASSERT_EQ("my_table", cmd->table_name);
}

TEST(DropTable, MissingDropKeyword) {
    auto result = parser::parse_drop_table("TABLE my_table");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected DROP keyword.", result.error_message);
}

TEST(DropTable, MissingTableKeyword) {
    auto result = parser::parse_drop_table("DROP my_table");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected TABLE keyword.", result.error_message);
}

TEST(DropTable, NoTableName) {
    auto result = parser::parse_drop_table("DROP TABLE");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Table name is empty.", result.error_message);
}

TEST(DropTable, InvalidTableNameCharacters) {
    auto result = parser::parse_drop_table("DROP TABLE my-table");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Table name 'my-table' contains invalid characters.", result.error_message);
}

TEST(DropTable, ExtraTokens) {
    auto parse_result = parser::parse_drop_table("DROP TABLE my_table extra_token");
    ASSERT_FALSE(parse_result);
    ASSERT_EQ(
        "ERROR: Invalid DROP TABLE command. Parsed table name as 'my_table', but found extra tokens after it: "
        "'extra_token'.",
        parse_result.error_message);
}

TEST(ShowTables, BasicShowTables) {
    auto parse_result = parser::parse_show_tables("SHOW TABLES");
    ASSERT_TRUE(parse_result);
}

TEST(ShowTables, ExtraWhitespace) {
    auto parse_result = parser::parse_show_tables("   SHOW   TABLES   ");
    ASSERT_TRUE(parse_result);
}

TEST(ShowTables, MissingTablesKeyword) {
    auto parse_result = parser::parse_show_tables("SHOW");
    ASSERT_FALSE(parse_result);
    ASSERT_EQ("ERROR: Expected TABLES keyword.", parse_result.error_message);
}

TEST(ShowTables, ExtraTokens) {
    auto parse_result = parser::parse_show_tables("SHOW TABLES extra_token");
    ASSERT_FALSE(parse_result);
    ASSERT_EQ("ERROR: Invalid SHOW TABLES command. Found extra tokens: 'extra_token'.", parse_result.error_message);
}

TEST(Insert, BasicInsert) {
    std::string query = "INSERT INTO my_table VALUES (1, 'Alice')";
    auto parse_result = parser::parse_insert(query);
    ASSERT_TRUE(parse_result);

    std::optional<command::InsertCommand> insert_command = parse_result.command;
    ASSERT_EQ("my_table", insert_command->table_name);
    ASSERT_TRUE(insert_command->columns.empty());
    ASSERT_EQ(std::vector<std::string>({"1", "Alice"}), insert_command->values);
}

TEST(Insert, InsertWithColumns) {
    std::string query = "INSERT INTO my_table (id, name) VALUES (1, 'Alice')";
    auto parse_result = parser::parse_insert(query);
    ASSERT_TRUE(parse_result);

    std::optional<command::InsertCommand> insert_command = parse_result.command;
    ASSERT_EQ("my_table", insert_command->table_name);
    ASSERT_EQ(std::vector<std::string>({"id", "name"}), insert_command->columns);
    ASSERT_EQ(std::vector<std::string>({"1", "Alice"}), insert_command->values);
}

TEST(Insert, ExtraWhitespace) {
    std::string query = "   INSERT   INTO   my_table   (id, name)   VALUES   (1, 'a b c d')   ";
    auto parse_result = parser::parse_insert(query);
    ASSERT_TRUE(parse_result);

    std::optional<command::InsertCommand> insert_command = parse_result.command;
    ASSERT_EQ("my_table", insert_command->table_name);
    ASSERT_EQ(std::vector<std::string>({"id", "name"}), insert_command->columns);
    ASSERT_EQ(std::vector<std::string>({"1", "a b c d"}), insert_command->values);
}

TEST(Insert, MissingIntoKeyword) {
    auto result = parser::parse_insert("INSERT my_table VALUES (1, 'Alice')");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected INTO keyword.", result.error_message);
}

TEST(Insert, MissingTableName) {
    auto result = parser::parse_insert("INSERT INTO (1, 'Alice')");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Table name '(1,' contains invalid characters.", result.error_message);
}

TEST(Insert, NoValues) {
    auto result = parser::parse_insert("INSERT INTO my_table");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Incomplete INSERT statement. Expected columns list or 'VALUES'.", result.error_message);
}

TEST(Insert, NoValuesAfterValuesKeyword) {
    auto result = parser::parse_insert("INSERT INTO my_table VALUES");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected '(' to start list.", result.error_message);
}

TEST(Insert, MissingValuesKeyword) {
    auto result = parser::parse_insert("INSERT INTO my_table (id, name)");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected VALUES keyword.", result.error_message);
}

TEST(Insert, MissingValuesAfterColumns) {
    auto result = parser::parse_insert("INSERT INTO my_table (id, name) VALUES");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Expected '(' to start list.", result.error_message);
}

TEST(Insert, DuplicateColumnName) {
    auto result = parser::parse_insert("INSERT INTO my_table (id, id) VALUES (1, 'Alice')");
    ASSERT_FALSE(result);
    ASSERT_EQ("ERROR: Duplicate column names found in the column list.", result.error_message);
}
