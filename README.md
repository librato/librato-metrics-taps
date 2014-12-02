# librato-metrics-taps

## Deprecation Notice

**This project is being officially deprecated by the maintainers.**

We recommend alternatives such as [JMXTrans](https://github.com/jmxtrans/embedded-jmxtrans) and collectd's [JMX plugin](https://collectd.org/wiki/index.php/Plugin:GenericJMX) projects for submitting your JMX metrics to Librato. These have more comprehensive coverage, require fewer external dependencies, and are more actively developed than this project.

We will keep this project available on GitHub for now, but there will be no ongoing support or development of the project from this point forward.

## Overview

Collection of helper scripts and library routines to tap into external
metric sources and pump those metrics into Librato's Metric Service.

With these you can monitor JVM services, like Cassandra, and build monitoring dashboards:

![Cassandra Dashboard](https://s3.amazonaws.com/librato_images/Github/Selection_292.png)

## librato-metrics-tap-jmxbeans

The *JMX Beans* tap script will connect to a JMX service and pull
monitoring attributes from a configured set of MBeans. The values of
these MBean attributes are pumped into Librato's Metric Service.

The usage for librato-metrics-tap-jmxbeans is:

```
Usage: librato-metrics-tap-jmxbeans <options>

Options:
              --jmx-host, -j <s>:   JMX Hostname (default: localhost)
              --jmx-port, -m <i>:   JMX Port (default: 8080)
          --jmx-username, -x <s>:   JMX Username
          --jmx-password, -w <s>:   JMX Password
                 --email, -e <s>:   Metrics Email address
                 --token, -t <s>:   Metrics API token
                --source, -s <s>:   Optional source name
                --prefix, -z <s>:   Optional prefix name
          --measure-time, -a <i>:   Optional time for measurements
           --metrics-url, -r <s>:   Metrics URL (default:
                                    https://metrics-api.librato.com)
              --interval, -i <i>:   Run as a daemon and poll every N seconds
            --ignore-missing, -g:   Ignore missing beans/attributes
        --data-file-full, -d <s>:   YAML file defining beans & attribute names
  --data-file-attributes, -f <s>:   YAML file of bean attributes
             --bean-name, -b <s>:   Bean name to match against or lookup
                   --publish, -p:   Publish Bean Attributes to Librato Metrics
               --match-beans, -c:   Output beans that match --bean-name regexp
                   --version, -v:   Print version and exit
                      --help, -h:   Show this message
```

You can either specify a full bean and attribute definition file or
you can specify the bean name(s) separately on the commandline and
specify the bean attributes from the config file.

### Full Bean and Attribute Definition

Use the `--data-file-full` option to specify the path to a YAML file
defining the full list of beans and attributes to retreive. An example
of such a configuration file is:

```
--- 
org.apache.cassandra.internal:type=AntiEntropyStage:
org.apache.cassandra.internal:type=FlushSorter:
  ActiveCount:
  CompletedTasks: counter
  PendingTasks:
```

For example, this will grab all the attributes of the bean name
`org.apache.cassandra.internal:type=AntiEntropyStage` and will push
each of those attributes as metrics.

If the file contains a list of attributes for a bean, it will restrict
the attributs to the ones listed. For example, for the bean
`org.apache.cassandra.internal:type=FlushSorter`, it will limit the
bean attributes to *ActiveCount*, *CompletedTasks* and *PendingTasks*.

By default, attributes are published as **gauges** to the Librato
Metrics service. If instead you would like to publish the attribute as
a counter, set the attribute name to the value 'counter'.

An example full bean definition file is included for Cassandra under
the `examples/cassandra` directory in the top-level of this gem. For
example, to publish the thread JMX beans from Cassandra 0.8.1:

```
librato-metrics-tap-jmxbeans --email "$EMAIL" --token "$TOKEN" \
    --publish \
    --source "$SOURCE_NAME" \
    --jmx-host "$JMX_HOST" --jmx-port "$JMX_PORT" \
    --data-file-full examples/cassandra/tpstats-0_8_1.yaml \
    --measure-time "$MEASURE_TIME"
```

### Bean attributes separate from beans

Oftentimes it is useful to publish the same attributes from multiple
beans that all export the same attribute names. In this case you can
use the `--data-file-attributes` option to set a YAML file defining
the bean attributes to publish. You can then set the bean names using
the `--bean-name` option.

For example, a bean attribute definition file may look like:

```
---
LiveDiskSpaceUsed:
LiveSSTableCount:
MemtableColumnsCount: counter
```

If this file were used in combination with the option `--bean-name
org.apache.cassandra.db:type=ColumnFamilies,keyspace=MyKeyspace,columnfamily=mycf`,
then the attributes `LiveDiskSpaceUsed`, `LiveSSTableCount` and
`MemtableColumnsCount` for the bean representing the Cassandra
columnfamily *mycf* in the keyspace *MyKeyspace* would be published.

The `--bean-name` can also be a wildcard that will be expanded to all
matching beans. For example, the option `--bean-name
'org.apache.cassandra.db:type=ColumnFamilies,keyspace=MyKeyspace,columnfamily=*'`
would publish the same attributes for all column families in the
keyspace *MyKeyspace*.

Attributes are always published as *gauges* to the Librato Metrics
service. To publish them as a counter, set their value to 'counter'.

An example bean attribute definition file is included for Cassandra
under the `examples/cassandra` directory in the top-level of this
gem. For example, to publish the CF stats for all column families in
the keyspace *MyKeyspace*:

```
librato-metrics-tap-jmxbeans --email "$EMAIL" --token "$TOKEN" \
    --publish --source "$SOURCE_NAME" \
    --jmx-host "$JMX_HOST" --jmx-port "$JMX_PORT" \
    --bean-name 'org.apache.cassandra.db:type=ColumnFamilies,keyspace=MyKeyspace,columnfamily=*' \
    --data-file-attributes examples/cassandra/cfstats-0_8_1.yaml \
    --measure-time "$MEASURE_TIME"
```

## Contributing to librato-metrics-taps
 
* Check out the latest master to make sure the feature hasn't been implemented or the bug hasn't been fixed yet
* Check out the issue tracker to make sure someone already hasn't requested it and/or contributed it
* Fork the project
* Start a feature/bugfix branch
* Commit and push until you are happy with your contribution
* Make sure to add tests for it. This is important so I don't break it in a future version unintentionally.
* Please try not to mess with the Rakefile, version, or history. If you want to have your own version, or is otherwise necessary, that is fine, but please isolate to its own commit so I can cherry-pick around it.

## Copyright

Copyright (c) 2011 Librato, Inc. See LICENSE.txt for
further details.

