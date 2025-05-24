# Rough Notes

This is a dump of my thoughts when exploring different alternatives for xyz stuff.



Implementation of CREATE TABLE command:

1. Add TableSchema struct. It's not the same as CreateTableCommand (maybe it is initially, but it won't be eventually):
   1. CREATE INDEX command should update the TableSchema
   2. TableSchema can contain auto-generated names of constraints if the user hasn't supplied them (example, UNIQUE constraint)
   3. etc, we will find out more gradually
2. Files to add:
   1. catalog.h: TableSchema, CatalogData
   2. catalog.cpp: load_catalog, save_catalog


Catalog is database about the database.
1. Postgres: Info stored in pg_catalog schema. It has tables like pg_class, pg_tables, pg_attribute, etc. User table data is typically stored in separate files on disk,
2. MySQL: Table definitions are stored in .frm files. Table data is stored in .ibd files.
3. sqlite: sqlite_master table

Phase 1: Catalog Structures & JSON Serialization
   Define catalog::TableSchema and catalog::CatalogData in simpledb/catalog.h.
   Implement to_json/from_json for command::Datatype (enum <-> string).
   Implement to_json/from_json for command::ColumnDefinition.
   Implement to_json/from_json for catalog::TableSchema.
   Implement catalog::load_catalog() in simpledb/catalog.cpp (reads _catalog.json).
   Implement catalog::save_catalog() in simpledb/catalog.cpp (writes _catalog.json).
   Write minimal tests/driver code to verify load_catalog and save_catalog with a sample _catalog.json.
Phase 2: Integration into handle_create_table
   Modify handle_create_table in main.cpp to call catalog::load_catalog().
   In handle_create_table, check for table name conflicts using loaded catalog data.
   In handle_create_table, convert command::CreateTableCommand to catalog::TableSchema.
   In handle_create_table, add the new catalog::TableSchema to the CatalogData.
   In handle_create_table, call catalog::save_catalog() to persist changes.
   In handle_create_table, create the empty data file (e.g., table_name.csv).
   Test CREATE TABLE functionality thoroughly from the REPL.

catalog.json:
```json
[
  {
    "table_name": "users",
    "columns": [
      {"column_name": "id", "type": "INT"},
      {"column_name": "username", "type": "TEXT"}
    ]
  },
  {
    "table_name": "products",
    "columns": [
      {"column_name": "sku", "type": "TEXT"},
      {"column_name": "price", "type": "INT"},
      {"column_name": "description", "type": "TEXT"}
    ]
  }
]
```


https://aistudio.google.com/prompts/17DRNPhY1QuXOOECcAXZGttiH5vi1Qd8g


Todo:
1. Rename include guards from COMMAND_H to SIMPLEDB_COMMAND_H, to avoid conflicts with other libraries if they also have COMMAND_H.
2. 