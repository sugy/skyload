# Table of Contents #



# Requirement #
You need to have [libdrizzle](https://launchpad.net/libdrizzle/) installed on your system. If you don't have libdrizzle, you can download it from the following URL:

  * [https://launchpad.net/libdrizzle/+download](https://launchpad.net/libdrizzle/+download)

# Installation #
  * Download the latest [source package](http://code.google.com/p/skyload/downloads/list)
  * Extract the content and change the current directory to it
```
$ tar xvzf skyload-version.tar.gz
$ cd skyload-version
```

  * Run the configure script
```
$ ./configure
```

  * Build skyload
```
$ make
```

  * Run the build test
```
$ make check
```

  * Install skyload
```
$ sudo make install
```

# Quick Start (with Drizzle) #

This quick start will demonstrate how to load 50,000 rows to a simple local [InnoDB](http://www.innodb.com/) table with two columns. The following example will perform and measure how long it takes to load the table with four concurrent connections.

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, point int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand)"
```

Note that by default, skyload will delete the database it had created after each test run. If you want skyload to keep the test database and take a look at it yourself, you can do this by supplying the '--keep' option:

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, point int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand)" --keep

$ drizzle

drizzle > use skyload;
drizzle > SHOW CREATE TABLE t1;
drizzle > SELECT * FROM t1 LIMIT 50;
```

Another point to note is that skyload will attempt to communicate with Drizzle's default port (4427) if a port number isn't specified with the '--port=' option.

## Test  Auto Increment Performance ##
As another simple example, you can test the auto increment performance of a particular engine like so (in this case, InnoDB):

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key auto_increment, num int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 (num) VALUES (%rand)"
```

# Quick Start (with MySQL) #
The only difference between the above example is that you need to supply an additional '--mysql' option. This option makes skyload communicate using the MySQL protocol. Also note that under this mode, skyload will attempt to communicate with port 3306 (mysqld's default port) unless it is specified with the '--port=' option.

The following example will load 50,000 rows to MySQL using four concurrent connections.

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, point int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand)" --mysql
```

Note that if your MySQL instance isn't built with InnoDB, the generated test data will be loaded against a [MyISAM](http://dev.mysql.com/doc/refman/5.4/en/myisam-storage-engine.html) table (default behavior of MySQL).

# INSERT Query Template #
Although there are plans for supporting external data source import (e.g. the [slow query log](http://dev.mysql.com/doc/refman/5.4/en/slow-query-log.html)), you currently have to provide a template that describes the INSERT query with '--insert=' to run a load benchmark.

An INSERT query template comprises of:

  * INSERT statement of your choice that matches the table
  * Special placeholders placed inside the VALUES() clause

Currently available placeholders are:

  * **%seq**: sequential integer that is guaranteed to not clash
  * **%rand**: random value generated internally

## Example ##
Here are some possible INSERT query templates that can be applied to this table:
```
CREATE TABLE t1 (
  id int primary_key,
  name varchar(255),
  rank int
) ENGINE=InnoDB;
```

Orthodox Template:
```
INSERT INTO t1 VALUES (%seq, %rand, %seq);
INSERT INTO t1 (id, rank) VALUES (%seq, %seq);
```

Running skyload against a local drizzled instance with the first template:
```
skyload --server=localhost --rows=5000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, name varchar(255), rank int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %seq, %rand)"
```

# Custom Load Emulation #

You can run load tests against your database server by providing a newline-separated SQL file. There are two options to load external files: "--load-file="  and "--read-file=". A **load file** is intended for test data creation and **read file** is for creating read load.

Both options are similar in a way that you have the freedom to write anything you like but they differ in the following way:

  * File provided by "--load-file=" option cannot be executed against a database specified with "--db=" option.
  * File provided by "--load-file=" option is executed by the main thread (only executed once).
  * File provided by "--read-file=" option is executed by the worker thread(s).

## Example ##

Let the following be insert.sql

```
CREATE TABLE t1 (id int primary key,  num int, message varchar(255)) ENGINE=InnoDB; 
INSERT INTO t1 VALUES (1, 100, "xxxxxxxxx");
INSERT INTO t1 VALUES (2, 101, "xxxxxxxxx");
INSERT INTO t1 VALUES (3, 102, "xxxxxxxxx");
INSERT INTO t1 VALUES (4, 103, "xxxxxxxxx");
INSERT INTO t1 VALUES (5, 104, "xxxxxxxxx");
INSERT INTO t1 VALUES (6, 105, "xxxxxxxxx");
```

Let the following be read.sql

```
SELECT * FROM t1;
```

Given the two files, you can run skyload like so:

```
$ skyload --server=localhost --load-file=/path/to/insert.sql --read-file=/path/to/read.sql
```

You could also auto generate the table and run the read file against it

```
$ skyload --server=localhost --rows=5000 \
--table="CREATE TABLE t1 (id int primary key, num int, message varchar(255)) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand, %seq)" \
--read-file=/path/to/read.sql
```

If you wish to run the read test multiple times, you can do so by providing the "--runs=" option. The following will run the read test 1000 times.

```
$ skyload --server=localhost --rows=5000 \
--table="CREATE TABLE t1 (id int primary key, num int, message varchar(255)) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand, %seq)" \
--read-file=/path/to/read.sql \
--runs=1000
```

Because the read load is created by the worker threads, you can run the queries in parallel by using the "--concurrency=" option. If there are 5 workers and 10 statements in the read file, the number of queries executed in one run is 50.

```
$ skyload --server=localhost --rows=5000 \
--table="CREATE TABLE t1 (id int primary key, num int, message varchar(255)) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand, %seq)" \
--read-file=/path/to/read.sql \
--runs=1000 --concurrency=4
```

# Using an existing Database #

By specifying the database name with "--db=" option, you can run custom load emulation against the tables in that database. The limitation of this mode is that you can only run queries supplied with the "--read-file=" option. This is to avoid the existing database from being undesirably updated. The database that you specify will NOT BE dropped after the test run.

Saying that, it is possible to write SQL statements that can modify your database in the read file. Only use this mode on a database in your test environment.

## Example ##

```
$ skyload --server=localhost --read-file=/path/to/file --db=test --runs=1000
```