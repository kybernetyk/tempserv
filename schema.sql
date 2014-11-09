BEGIN TRANSACTION;
CREATE TABLE data (id integer primary key, timestamp integer NOT NULL, temp real NOT NULL);
COMMIT;
