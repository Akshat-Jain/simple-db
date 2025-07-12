//
// Created by Akshat Jain on 20/04/25.
//

#include "simpledb/parser.h"

#include "ANTLRInputStream.h"
#include "SimpleDBLexer.h"
#include "SimpleDBParser.h"
#include "parser/ast_builder_visitor.h"

namespace parser {
    std::optional<CommandVariant> parse_sql(const std::string& query) {
        antlr4::ANTLRInputStream input(query);
        SimpleDBLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        SimpleDBParser parser(&tokens);

        // This will build the parse tree. If there are errors, ANTLR will
        // attempt to recover and build a partial/malformed tree.
        SimpleDBParser::QueryContext* tree = parser.query();

        // After parsing, we ask the parser if it encountered any syntax errors.
        if (parser.getNumberOfSyntaxErrors() > 0) {
            // The query had a syntax error, so we fail the parse.
            return std::nullopt;
        }

        // If we get here, the syntax was valid. Now we visit the tree.
        AstBuilderVisitor visitor;
        std::any result = visitor.visit(tree);
        if (!result.has_value()) {
            return std::nullopt;
        }

        if (result.type() == typeid(ast::SelectCommand)) {
            return std::any_cast<ast::SelectCommand>(result);
        }
        if (result.type() == typeid(command::CreateTableCommand)) {
            return std::any_cast<command::CreateTableCommand>(result);
        }
        if (result.type() == typeid(command::DropTableCommand)) {
            return std::any_cast<command::DropTableCommand>(result);
        }
        if (result.type() == typeid(command::InsertCommand)) {
            return std::any_cast<command::InsertCommand>(result);
        }
        if (result.type() == typeid(command::ShowTablesCommand)) {
            return std::any_cast<command::ShowTablesCommand>(result);
        }

        throw std::runtime_error("Unsupported command type in parser.");
    }
}  // namespace parser
