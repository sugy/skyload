# はじめに #
skyloadはDrizzle、またはMySQLプロトコルを扱えるデータベースサーバに対して複数のコネクションを並列してロードテストを容易に行えるツールです。MySQL標準のmysqlslapに比べ、ロードテストの設定を細かく、かつ直感的に行える事を目標にしています。


# 必要条件 #

skyloadをインストールするには[libdrizzle](https://launchpad.net/libdrizzle)が必要です。libdrizzleは以下のURLから入手できます。

  * [https://launchpad.net/libdrizzle/+download](https://launchpad.net/libdrizzle/+download)

# インストール #

  * skyloadの[最新パッケージ](http://code.google.com/p/skyload/downloads/list)を入手します。
  * 入手したパッケージを展開して、作成されたディレクトリに入ります。
```
$ tar xvzf skyload-version.tar.gz
$ cd skyload-version
```

  * configureスクリプトを実行します
```
$ ./configure
```

  * skyloadをビルドします
```
$ make
```

  * ビルドテストを実行します
```
$ make check
```

  * ビルドしたskyloadをインストールします
```
$ sudo make install
```

# Quick Start (Drizzle編) #

このQuick Startではローカルに立ち上がっているDrizzleのInnoDBテーブルに5万行のデータをロードし、その実行速度を計測する方法を紹介します。以下の例は4本のコネクションから並列して、自動生成された5万行のデータをロードします。

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, point int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand)"
```

skyloadはテスト用に自動生成したデータベースとテーブルをテスト後にデフォルトで破棄します。破棄せずに、テスト後にデータベース操作を自分で行いたい場合は、'--keep' オプションを指定します。

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, point int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand)" --keep
```

以下の例でロードテスト終了後にdrizzleのクライエントを立ち上げ、データベースを閲覧できます。

```
$ drizzle

drizzle > use skyload;
drizzle > SHOW CREATE TABLE t1;
drizzle > SELECT * FROM t1 LIMIT 50;
```

また、'--port=' オプションでポート番号が指定されなかった場合、skyloadはDrizzleのデフォルトポートである、4427番への通信を試みます。

## Auto Incrementの性能測定 ##
もう一つの例として、skyloadで特定のストレージエンジンのauto increment性能を測定してみましょう。以下の例ではInnoDBに対してテストを行っています。

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key auto_increment, num int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 (num) VALUES (%rand)"
```

# Quick Start (MySQL編) #
Drizzleに対するロードテストとの違いは、'--mysql' というオプションを追加する事です。このオプションが適応された場合、skyloadはMySQLプロトコルを使って通信を行います。また、 '--port=' オプションでポート番号が指定されなかった場合はmysqldのデフォルトポートである3306番に対して通信を行います。

以下の例では4本の並列コネクションを用いて、5万行のデータをMySQLにロードします。

```
$ skyload --server=localhost --rows=50000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, point int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand)" --mysql
```

## MySQLに対するロードテストの注意点 ##

InnoDBが適応されていないオプションでビルドされたMySQLに対してロードテストを行うと、InnoDBではなく、[MyISAM](http://dev.mysql.com/doc/refman/5.4/en/myisam-storage-engine.html)のテーブルにロードされます。これはMySQLの仕様によるものです。

# INSERT Query テンプレート #

skyloadでは現状、[slow query log](http://dev.mysql.com/doc/refman/5.4/en/slow-query-log.html)などの外部ファイルからデータをロードする機能が未実装であり、代わりにINSERT文のテンプレートを '--insert=' オプションで指定する事でデータが自動生成されます。

INSERT Query テンプレートは以下の要素で構成されます。

  * 指定したテーブルに準拠したINSERT文
  * VALUES( ) 内に配置するカラムデータを表す特殊なplaceholder

### 現在扱えるplaceholder ###

  * **%seq**: シーケンシャルな値
  * **%rand**: ランダムな値

### 例 ###

以下のテーブルに対して使えるテンプレートを紹介します。

```
CREATE TABLE t1 (
  id int primary_key,
  name varchar(255),
  rank int
) ENGINE=InnoDB;
```

オーソドックスなテンプレート:

```
INSERT INTO t1 VALUES (%seq, %rand, %seq);
INSERT INTO t1 (id, rank) VALUES (%seq, %seq);
```

最初のテンプレートを使った実行例:

```
skyload --server=localhost --rows=5000 --concurrency=4 \
--table="CREATE TABLE t1 (id int primary key, name varchar(255), rank int) ENGINE=InnoDB" \
--insert="INSERT INTO t1 VALUES (%seq, %rand, %seq)"
```

### Placeholderの今後 ###
現状では個々のカラムデータのサイズを指定する事ができませんが、次のリリースでサポートする予定です。