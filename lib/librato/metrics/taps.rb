__LIB_DIR__ = File.expand_path(File.join(File.dirname(__FILE__), '../..'))

$LOAD_PATH.unshift __LIB_DIR__ unless
  $LOAD_PATH.include?(__LIB_DIR__) ||
  $LOAD_PATH.include?(File.expand_path(__LIB_DIR__))

require 'rubygems'
require 'jmx4r'
require 'rest_client'
require 'trollop'
require 'json'

module Librato
  module Metrics
    module Taps

      def self.version
        File.read(File.join(File.dirname(__FILE__), '../../../VERSION')).chomp
      end
    end
  end
end

require 'librato/metrics/taps/jmx.rb'
require 'librato/metrics/taps/publisher.rb'
