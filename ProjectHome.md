**Welcome**

skyload is a [libdrizzle](https://launchpad.net/libdrizzle) based load emulation tool aimed at running detailed, yet simple concurrent load tests against a database instance that can speak Drizzle (and/or) the MySQL protocol. It is originally intended to help storage engine developers but skyload can also help database admins for planning against certain access patterns.

skyload is not a replacement for [mysqlslap](http://dev.mysql.com/doc/refman/5.4/en/mysqlslap.html) or drizzleslap because it only provides a subset of what they can do. Rather, it is designed to do a good job at this subset of tasks by giving you more control over how you benchmark your server.

  * [Instructions on how to install and run skyload](http://code.google.com/p/skyload/wiki/Tutorial_en)

**What you can do with skyload**

  * Test the row insertion speed (singly or concurrently)
  * Test the selection speed
  * Test the update (replace) speed
  * All of the above at once, or individually.

**Planned Goodness**
  * Calculate latency encountered by each worker thread.

**Requirements**

You need [libdrizzle](https://launchpad.net/libdrizzle) to build skyload.