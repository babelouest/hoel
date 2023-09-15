-- Public domain, no copyright. Use at your own risk

DROP TABLE IF EXISTS test_table;

CREATE TABLE test_table (
  id_col SERIAL PRIMARY KEY,
  integer_col INTEGER,
  double_col NUMERIC(10, 2),
  string_col VARCHAR(256),
  date_col TIMESTAMPTZ
);
