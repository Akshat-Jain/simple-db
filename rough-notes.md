# Rough Notes

This is a dump of my thoughts when exploring different alternatives for xyz stuff.



Implementation of CREATE TABLE command:

1. Add TableSchema struct. It's not the same as CreateTableCommand (maybe it is initially, but it won't be eventually):
   1. CREATE INDEX command should update the TableSchema
   2. TableSchema can contain auto-generated names of constraints if the user hasn't supplied them (example, UNIQUE constraint)
   3. etc, we will find out more gradually
2. Files to add:
   1. TBD
