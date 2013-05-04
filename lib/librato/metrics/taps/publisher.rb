module Librato
  module Metrics
    module Taps
      class Publisher
        SOURCE_NAME_REGEX = /^[-A-Za-z0-9_.]{1,255}$/
        DEFAULT_URL = 'https://metrics-stg.heroku.com'

        @url = nil
        @user = nil
        @passwd = nil

        DEFAULT_BATCH_SIZE = 300

        def initialize(opts)
          unless !opts[:source] || opts[:source] =~ SOURCE_NAME_REGEX
            raise "Invalid source"
          end

          unless !opts[:prefix] || opts[:prefix] =~ SOURCE_NAME_REGEX
            raise "Invalid prefix"
          end

          @client = Librato::Metrics::Client.new
          @client.authenticate(opts[:email], opts[:token])
          @client.api_endpoint = opts[:metrics_url]
          @client.agent_identifier("librato-metrics-tap-jmxbeans/%s" %
                                   [Taps.version])

          qparams = {
            :autosubmit_count => DEFAULT_BATCH_SIZE
          }

          [:prefix, :source].each do |o|
            qparams[o] = opts[o] if opts[o]
          end

          @queue_params = qparams
        end

        # Post metrics to metrics service
        #
        #  counters:
        #    {:my-counter-1 => 4, :my-counter-2 => 6, ...}
        #
        #  gauges:
        #    {:my-gauge-1 => 4, ...}
        #    or
        #    {:my-gauge-2 => {:value => 5, :count => 4,
        #                     :min => 3, :max => 6, ...}}
        #    or mix-match.
        def post(counters, gauges, params = {})
          mt = params[:measure_time] || now_secs
          q = @client.new_queue(@queue_params)

          if counters.length > 0
            counters.each_pair do |k, v|
              q.add k => {:type => :counter, :value => v, :measure_time => mt}
            end
          end

          if gauges.length > 0
            params[:gauges] = {}
            gauges.each_pair do |k, v|
              if v.respond_to?(:keys)
                q.add k => v.merge(:measure_time => mt)
              else
                q.add k => {:value => Float(v), :measure_time => mt}
              end
            end
          end

          begin
            q.submit
          rescue => err
            # XXX: ever get here? what about auto-submit?
            puts "Failed to publish stats (unknown exception): #{err.inspect}"
            return false
          end

          true
        end

        def now_secs
          Time.now.tv_sec
        end
      end
    end
  end
end
