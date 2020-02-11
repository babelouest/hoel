DROP TABLE IF EXISTS test_table;

CREATE TABLE test_table (
  id_col SERIAL PRIMARY KEY,
  integer_col INTEGER,
  string_col VARCHAR(256),
  date_col TIMESTAMPTZ
);
