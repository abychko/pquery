# Multiline comments like /* */ are not allowed here due to shuffled SQL execution
# No preprocessing will be made for this file to skip multiline comments
# valid comments must be prefixed with '#', ';' or '//'
# this is a comment
; this is comment
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
#
