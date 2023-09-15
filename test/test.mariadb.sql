-- Public domain, no copyright. Use at your own risk

DROP TABLE IF EXISTS test_table;

CREATE TABLE test_table (
  id_col INT(11) PRIMARY KEY AUTO_INCREMENT,
  integer_col INT(11),
  double_col FLOAT(9,2),
  string_col VARCHAR(256),
  date_col TIMESTAMP
);
