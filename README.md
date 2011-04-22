# librato-metrics-taps

Collection of helper scripts and library routines to tap into external
metric sources and pump those metrics into Librato's Metric Service.

## librato-metrics-tap-jmxbeans

The *JMX Beans* tap script will connect to a JMX service and pull
monitoring attributes from a configured set of MBeans. The values of
these MBean attributes are pumped into Librato's Metric Service.

Bean names are configured through the `--beans` command line
option. This parameter takes the path to a YAML file that looks like
this:

```
--- 
org.apache.cassandra.internal:type=AntiEntropyStage: 
org.apache.cassandra.db:type=CompactionManager: 
org.apache.cassandra.db:type=StorageProxy: 
  - RangeOperations
  - ReadOperations
  - WriteOperations
org.apache.cassandra.db:type=StorageService: 
  - Load
```

For example, this will grab all attributes from the bean
`org.apache.cassandra.internal:type=AntiEntropyStage` and push those
attributes with a metric name as
`org.apache.cassandra.internal:type=AntiEntropyStage::<attribute
name>`. To limit the set of bean attributes published, provide a list
of the attributes as the hash value of the bean name. For example, the
configuration above will only grab the `Load` attribute of the
`org.apache.cassandra.db:type=StorageService` bean.


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

