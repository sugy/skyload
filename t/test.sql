INSERT INTO table VALUES (1, 100, 200, "xxxxxxxxx");
INSERT INTO table VALUES (2, 101, 201, "xxxxxxxxx");
INSERT INTO table VALUES (3, 102, 202, "xxxxxxxxx");
INSERT INTO table VALUES (4, 103, 203, "xxxxxxxxx");
INSERT INTO table VALUES (5, 104, 204, "xxxxxxxxx");
INSERT INTO table VALUES (6, 105, 205, "xxxxxxxxx");

UPDATE table SET foo_column = "changed" WHERE id = 4;
UPDATE table SET foo_column = "changed" WHERE id = 6;

SELECT * FROM table WHERE cond_accessing_non_pk_that_never_succeeds;
SELECT * FROM table WHERE cond_accessing_non_pk_that_never_succeeds;
SELECT * FROM table WHERE cond_accessing_non_pk_that_never_succeeds;
SELECT * FROM table WHERE cond_accessing_non_pk_that_never_succeeds;
