-- License: MIT
DROP TABLE IF EXISTS other_test;

CREATE TABLE other_test (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(128),
    age INTEGER,
    birthdate DATE,
    temperature FLOAT
);
