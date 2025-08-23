//
// Created by Akshat Jain on 12/07/25.
//

#ifndef SIMPLE_DB_AST_BUILDER_VISITOR_H
#define SIMPLE_DB_AST_BUILDER_VISITOR_H

#include "SimpleDBBaseVisitor.h"

class AstBuilderVisitor : public SimpleDBBaseVisitor {
   private:
    std::string processIdentifier(const std::string &identifier);

   public:
    std::any visitQuery(SimpleDBParser::QueryContext *ctx) override;

    std::any visitSelectStatement(SimpleDBParser::SelectStatementContext *ctx) override;

    std::any visitProjection(SimpleDBParser::ProjectionContext *ctx) override;

    std::any visitColumnList(SimpleDBParser::ColumnListContext *ctx) override;

    std::any visitCreateStatement(SimpleDBParser::CreateStatementContext *ctx) override;

    std::any visitColumnDefinitions(SimpleDBParser::ColumnDefinitionsContext *ctx) override;

    std::any visitColumnDef(SimpleDBParser::ColumnDefContext *ctx) override;

    std::any visitDataType(SimpleDBParser::DataTypeContext *ctx) override;

    std::any visitDropStatement(SimpleDBParser::DropStatementContext *ctx) override;

    std::any visitInsertStatement(SimpleDBParser::InsertStatementContext *ctx) override;

    std::any visitValueList(SimpleDBParser::ValueListContext *ctx) override;

    std::any visitValue(SimpleDBParser::ValueContext *ctx) override;

    std::any visitShowStatement(SimpleDBParser::ShowStatementContext *ctx) override;
};

#endif  // SIMPLE_DB_AST_BUILDER_VISITOR_H
