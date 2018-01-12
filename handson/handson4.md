## Handson-3 The Big Picture

`ID:515030910036` `Name:吕嘉伟`

### Question 1

I have used Docker to deploye a hadoop cluster with 3 containers in it.(1 master, 2 slaves)

When type 'jps' in master,it shows:

> 610 SecondaryNameNode
>
> 725 Jps
>
> 415 NameNode

When type 'jps' in slave02,it shows:
> 372 Jps
>
> 286 DataNode

***
### Question 2

In master,it shows:
> JMX enabled by default
>
> Using config: /usr/local/zookeeper3.4.10/bin/../conf/zoo.cfg
>
> Mode: leader

In slaves,it shows:
> JMX enabled by default
>
> Using config: /usr/local/zookeeper3.4.10/bin/../conf/zoo.cfg
>
> Mode: follower


In 1 master,2 slaves cluter,there is 1 Leader and 2 followers.

***

### Question 3


When the leader the fails, each follower tries to become leader by broadcasting the last zxid they have seen. Others reply OK if the zxid they have seen is less than or equal and NO if they have a higher zxid. You assume you are the leader if you get OK from the majority and broadcast the same to the followers.

***
### Question 4
When we save data in HDFS,we are actually manipulating a logical file we do not need to know where the file stores but the HDFS will do it for us. The Namenode stores metadata for namespace while the Datanode stores data blocks for file.
We can configure the local storage directory in hdfs-site.xml, like xxx/data/current.
In "current" directory,there is a "finalized" directory and a "rbw" directory.Both of them are used to actually store data. "finalized" contain a lot of file like blk_xxx blk_xxx.meta,.meta is probably the checksum info. "rbw" means "replica being written",it's used to store data which is being written by user at the right moment.

***
### Question 5
```
val sparkConf = new SparkConf().setAppName("Spark_hbase").setMaster("local[2]")
val sc = new SparkContext(sparkConf)
val conf = HBaseConfiguration.create()

conf.set("hbase.zookeeper.property.clientPort", "2181")
conf.set("hbase.zookeeper.quorum", "master")

conf.set(TableInputFormat.INPUT_TABLE, "test")
val usersRDD = sc.newAPIHadoopRDD(conf, classOf[TableInputFormat], classOf[org.apache.hadoop.hbase.io.ImmutableBytesWritable],                        classOf[org.apache.hadoop.hbase.client.Result])
val count = usersRDD.count()
println("Temp RDD count:" + count)


```
