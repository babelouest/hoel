-- Public domain, no copyright. Use at your own risk

DROP TABLE IF EXISTS `test_table`;

CREATE TABLE `test_table` (
  `id_col` INTEGER PRIMARY KEY AUTOINCREMENT,
  `integer_col` INTEGER,
  `string_col` TEXT,
  `date_col` TIMESTAMP
);
