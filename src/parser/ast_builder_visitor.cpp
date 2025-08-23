//
// Created by Akshat Jain on 12/07/25.
//

#include "ast_builder_visitor.h"
#include "simpledb/command.h"
#include "simpledb/ast/ast.h"
#include <vector>

std::string AstBuilderVisitor::processIdentifier(const std::string &identifier) {
    if (identifier.front() == '"' && identifier.back() == '"' && identifier.length() >= 2) {
        // Remove outer quotes and convert escaped quotes
        std::string result = identifier.substr(1, identifier.length() - 2);
        // Convert "" to "
        size_t pos = 0;
        while ((pos = result.find("\"\"", pos)) != std::string::npos) {
            result.replace(pos, 2, "\"");
            pos += 1;
        }
        return result;
    }
    return identifier;
}

std::any AstBuilderVisitor::visitQuery(SimpleDBParser::QueryContext *ctx) {
    if (ctx->createStatement()) {
        return visit(ctx->createStatement());
    }
    if (ctx->dropStatement()) {
        return visit(ctx->dropStatement());
    }
    if (ctx->insertStatement()) {
        return visit(ctx->insertStatement());
    }
    if (ctx->showStatement()) {
        return visit(ctx->showStatement());
    }
    if (ctx->selectStatement()) {
        return visit(ctx->selectStatement());
    }
    throw std::runtime_error("Unsupported query type in AST builder visitor.");
}

std::any AstBuilderVisitor::visitSelectStatement(SimpleDBParser::SelectStatementContext *ctx) {
    ast::SelectCommand command;
    command.table_name = processIdentifier(ctx->tableName->getText());
    command.projection = std::any_cast<std::vector<std::string>>(visit(ctx->projection()));
    return command;
}

std::any AstBuilderVisitor::visitProjection(SimpleDBParser::ProjectionContext *ctx) {
    if (ctx->ASTERISK()) {
        return std::vector<std::string>();
    }
    return visit(ctx->columnList());
}

std::any AstBuilderVisitor::visitColumnList(SimpleDBParser::ColumnListContext *ctx) {
    std::vector<std::string> columns;
    for (const auto &item : ctx->IDENTIFIER()) {
        columns.push_back(processIdentifier(item->getText()));
    }
    return columns;
}

std::any AstBuilderVisitor::visitCreateStatement(SimpleDBParser::CreateStatementContext *ctx) {
    command::CreateTableCommand command;
    command.table_name = processIdentifier(ctx->tableName->getText());
    command.column_definitions = std::any_cast<std::vector<command::ColumnDefinition>>(visit(ctx->columnDefinitions()));
    return command;
}

std::any AstBuilderVisitor::visitColumnDefinitions(SimpleDBParser::ColumnDefinitionsContext *ctx) {
    std::vector<command::ColumnDefinition> column_defs;
    for (const auto &item : ctx->columnDef()) {
        column_defs.push_back(std::any_cast<command::ColumnDefinition>(visit(item)));
    }
    return column_defs;
}

std::any AstBuilderVisitor::visitColumnDef(SimpleDBParser::ColumnDefContext *ctx) {
    command::ColumnDefinition column_def;
    column_def.column_name = processIdentifier(ctx->columnName->getText());
    column_def.type = std::any_cast<command::Datatype>(visit(ctx->dataType()));
    return column_def;
}

std::any AstBuilderVisitor::visitDataType(SimpleDBParser::DataTypeContext *ctx) {
    if (ctx->INT_TYPE()) {
        return command::Datatype::INT;
    } else if (ctx->TEXT_TYPE()) {
        return command::Datatype::TEXT;
    }
    throw std::runtime_error("Unsupported data type in AST builder visitor.");
}

std::any AstBuilderVisitor::visitDropStatement(SimpleDBParser::DropStatementContext *ctx) {
    command::DropTableCommand command;
    command.table_name = processIdentifier(ctx->tableName->getText());
    return command;
}

std::any AstBuilderVisitor::visitInsertStatement(SimpleDBParser::InsertStatementContext *ctx) {
    command::InsertCommand command;
    command.table_name = processIdentifier(ctx->tableName->getText());
    if (ctx->columnList()) {
        command.columns = std::any_cast<std::vector<std::string>>(visit(ctx->columnList()));
    }
    command.values = std::any_cast<std::vector<std::string>>(visit(ctx->valueList()));
    return command;
}

std::any AstBuilderVisitor::visitValueList(SimpleDBParser::ValueListContext *ctx) {
    std::vector<std::string> values;
    for (const auto &item : ctx->value()) {
        values.push_back(std::any_cast<std::string>(visit(item)));
    }
    return values;
}

std::any AstBuilderVisitor::visitValue(SimpleDBParser::ValueContext *ctx) {
    if (ctx->STRING_LITERAL()) {
        std::string text = ctx->STRING_LITERAL()->getText();
        if (text.length() >= 2) {
            // Remove the surrounding single quotes
            return text.substr(1, text.length() - 2);
        }
        return std::string("");
    } else if (ctx->INTEGER_LITERAL()) {
        return ctx->INTEGER_LITERAL()->getText();
    }
    throw std::runtime_error("Unsupported value type in AST builder visitor.");
}

std::any AstBuilderVisitor::visitShowStatement(SimpleDBParser::ShowStatementContext *ctx) {
    return command::ShowTablesCommand{};
}
