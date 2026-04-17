# SQL splitter test file for pquery
# Multiline comments like /* ... */ are allowed and handled by the splitter.
# Comments must start with '#', '-- ' (two dashes and whitespace), or '//'.
# ';' is the default statement delimiter unless changed by a DELIMITER command.
# this is a comment
// this is a comment

DROP TABLE IF EXISTS t1;
CREATE TABLE t1 (ID int);
INSERT INTO t1 VALUES (1);
INSERT INTO t1 VALUES (2);
INSERT INTO t1 VALUES (3);
INSERT INTO t1 VALUES (4);
INSERT INTO t1 VALUES (5);
ALTER TABLE t1 ADD PRIMARY KEY pk(ID);
ALTER TABLE t1 DROP PRIMARY KEY;
INSERT INTO t1 SELECT * FROM t1;
DELETE FROM t1 LIMIT 3;
ALTER TABLE t1 ENGINE=InnoDB;
UPDATE t1 SET ID=2;
CALL country_hos('Europe');
SELECT 'test 1';
SELECT * FROM t1;
CREATE DATABASE IF NOT EXISTS test DEFAULT CHARACTER SET="utf8" DEFAULT COLLATE="utf8_bin";
USE test;
CREATE TABLE t1 (a int, b int, c int);
INSERT INTO t1 VALUES (1,2,3);
SELECT * FROM t1;

# Basic splitter smoke tests repeated to exercise shuffling
DROP TABLE IF EXISTS t1;
CREATE TABLE t1 (ID int);
INSERT INTO t1 VALUES (1);
INSERT INTO t1 VALUES (2);
INSERT INTO t1 VALUES (3);
INSERT INTO t1 VALUES (4);
INSERT INTO t1 VALUES (5);
ALTER TABLE t1 ADD PRIMARY KEY pk(ID);
ALTER TABLE t1 DROP PRIMARY KEY;
INSERT INTO t1 SELECT * FROM t1;
DELETE FROM t1 LIMIT 3;
ALTER TABLE t1 ENGINE=InnoDB;
UPDATE t1 SET ID=2;
CALL country_hos('Europe');
SELECT 'test 1';
SELECT * FROM t1;

CREATE DATABASE IF NOT EXISTS test DEFAULT CHARACTER SET="utf8" DEFAULT COLLATE="utf8_bin";
USE test;
DROP TABLE IF EXISTS t1;
CREATE TABLE t1 (a int, b int, c int);
INSERT INTO t1 VALUES (1,2,3);
SELECT * FROM t1;

# Multiline CREATE TABLE
DROP TABLE IF EXISTS splitter_test;
CREATE TABLE splitter_test (
  id INT PRIMARY KEY,
  note VARCHAR(100),
  payload TEXT
);

# Multiline INSERT
INSERT INTO splitter_test (
  id,
  note,
  payload
) VALUES (
  1,
  'simple row',
  'plain text payload'
);

# Semicolon inside single quotes must not split the statement
INSERT INTO splitter_test VALUES (
  2,
  'text with ; semicolon',
  'value;still inside string'
);

# Semicolon inside double quotes must not split the statement
INSERT INTO splitter_test VALUES (
  3,
  "double quoted ; text",
  "another ; payload"
);

# Semicolon inside backticks must not split the statement
SELECT `note`, `payload` FROM splitter_test;

-- This is a MySQL-style line comment with required whitespace
SELECT 'comment after mysql style line comment';

# Legacy hash comment
SELECT 'hash comment line';

// Legacy slash comment
SELECT 'slash comment line';

/* Block comment before a multiline statement */
INSERT INTO splitter_test (
  id,
  note,
  payload
) VALUES (
  4,
  'after block comment',
  'block comments should be ignored by splitter'
);

/* Block comment with ; semicolon inside ;;;;; */
SELECT 'block comment with semicolon before statement';

# Multiline SELECT with semicolons inside strings
SELECT
  id,
  note,
  payload,
  'a;b;c' AS sample1,
  "x;y;z" AS sample2
FROM splitter_test
WHERE note <> 'nothing;to;split';

UPDATE splitter_test
SET payload = 'updated ; payload'
WHERE id = 1;

DELETE FROM splitter_test
WHERE id = 999;

SELECT * FROM splitter_test;

# ----------------------------------------------------------------------
# DELIMITER tests: statements that rely on custom delimiters
# ----------------------------------------------------------------------

DELIMITER //

DROP PROCEDURE IF EXISTS splitter_test_proc//
CREATE PROCEDURE splitter_test_proc()
BEGIN
  INSERT INTO splitter_test VALUES (10, 'proc', 'line 1');
  INSERT INTO splitter_test VALUES (11, 'proc', 'line 2 ; with semicolon');
END//

DELIMITER ;

CALL splitter_test_proc();

SELECT * FROM splitter_test;
