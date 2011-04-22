__LIB_DIR__ = File.expand_path(File.join(File.dirname(__FILE__), '../..'))

$LOAD_PATH.unshift __LIB_DIR__ unless
  $LOAD_PATH.include?(__LIB_DIR__) ||
  $LOAD_PATH.include?(File.expand_path(__LIB_DIR__))

require 'rubygems'
require 'jmx4r'

module Librato
  module Metrics
    module Taps
    end
  end
end

require 'librato/metrics/taps/jmx.rb'
