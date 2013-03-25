require 'test/unit'
require File.join(File.dirname(__FILE__), '../lib/librato/metrics/taps')
include Librato::Metrics

class TestPublisher < Test::Unit::TestCase

  def test_metric_name_with_space_is_sanitized
    metric_name_with_space = "java.lang:type:MemoryPool:CMS Perm Gen"
    sanitized_metric_name = Taps::Publisher.sanitize_metric_name(metric_name_with_space)
    assert_equal("java.lang:type:MemoryPool:CMSPermGen", sanitized_metric_name)
  end

  def test_metric_name_with_quote_is_sanitized
    metric_name_with_space = "Catalina:type:GlobalRequestProcessor_name:\"ajp-bio-8009\"::bytesReceived"
    sanitized_metric_name = Taps::Publisher.sanitize_metric_name(metric_name_with_space)
    assert_equal("Catalina:type:GlobalRequestProcessor_name:ajp-bio-8009::bytesReceived", sanitized_metric_name)
  end

  def test_regular_metric_name_remains_untouched
    metric_name_with_space = "java.lang:type:Memory::HeapMemoryUsage.committed"
    sanitized_metric_name = Taps::Publisher.sanitize_metric_name(metric_name_with_space)
    assert_equal("java.lang:type:Memory::HeapMemoryUsage.committed", sanitized_metric_name)
  end


end