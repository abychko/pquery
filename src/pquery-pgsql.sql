# Multiline comments like /* */ are not allowed here due to shuffled SQL execution
# No BEGIN COMMIT blocks allowed here due to shuffled SQL execution
# No preprocessing will be made for this file to skip multiline comments
# valid comments must be prefixed with '#', ';' or '//'
#  this is a comment
;  this is a comment too
// this is a comment too

SELECT VERSION()
CREATE TABLE ABSTIME_TBL (f1 abstime);
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime 'now');
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime 'now');
SELECT count(*) AS two FROM ABSTIME_TBL WHERE f1 = 'now' ;
DELETE FROM ABSTIME_TBL;
INSERT INTO ABSTIME_TBL (f1) VALUES ('Jan 14, 1973 03:14:21');
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime 'Mon May  1 00:30:30 1995');
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime 'epoch');
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime 'infinity');
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime '-infinity');
INSERT INTO ABSTIME_TBL (f1) VALUES (abstime 'May 10, 1947 23:59:12');
INSERT INTO ABSTIME_TBL (f1) VALUES ('Feb 35, 1946 10:00:00');
INSERT INTO ABSTIME_TBL (f1) VALUES ('Feb 28, 1984 25:08:10');
INSERT INTO ABSTIME_TBL (f1) VALUES ('bad date format');
INSERT INTO ABSTIME_TBL (f1) VALUES ('Jun 10, 1843');
SELECT '' AS eight, * FROM ABSTIME_TBL;
SELECT '' AS six, * FROM ABSTIME_TBL   WHERE ABSTIME_TBL.f1 < abstime 'Jun 30, 2001';
SELECT '' AS six, * FROM ABSTIME_TBL   WHERE ABSTIME_TBL.f1 > abstime '-infinity';
SELECT '' AS six, * FROM ABSTIME_TBL   WHERE abstime 'May 10, 1947 23:59:12' <> ABSTIME_TBL.f1;
SELECT '' AS three, * FROM ABSTIME_TBL   WHERE abstime 'epoch' >= ABSTIME_TBL.f1;
SELECT '' AS four, * FROM ABSTIME_TBL   WHERE ABSTIME_TBL.f1 <= abstime 'Jan 14, 1973 03:14:21';
SELECT '' AS four, * FROM ABSTIME_TBL  WHERE ABSTIME_TBL.f1 <?> tinterval '["Apr 1 1950 00:00:00" "Dec 30 1999 23:00:00"]';
SELECT '' AS four, f1 AS abstime,  date_part('year', f1) AS year, date_part('month', f1) AS month,  date_part('day',f1) AS day, date_part('hour', f1) AS hour,  date_part('minute', f1) AS minute, date_part('second', f1) AS second  FROM ABSTIME_TBL  WHERE isfinite(f1)  ORDER BY abstime;
select max(100) from tenk1;
create table minmaxtest(f1 int);
create table minmaxtest1() inherits (minmaxtest);
create table minmaxtest2() inherits (minmaxtest);
create table minmaxtest3() inherits (minmaxtest);
create index minmaxtesti on minmaxtest(f1);
create index minmaxtest1i on minmaxtest1(f1);
create index minmaxtest2i on minmaxtest2(f1 desc);
create index minmaxtest3i on minmaxtest3(f1) where f1 is not null;
insert into minmaxtest values(11), (12);
insert into minmaxtest1 values(13), (14);
insert into minmaxtest2 values(15), (16);
insert into minmaxtest3 values(17), (18);

