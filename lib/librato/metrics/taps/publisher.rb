module Librato
  module Metrics
    module Taps
      module Publisher
        class << self
          SOURCE_NAME_REGEX = /^[-A-Za-z0-9_.]{1,255}$/
          DEFAULT_URL = 'https://metrics-stg.heroku.com'

          @url = nil
          @user = nil
          @passwd = nil

          def url=(new_url)
            if new_url =~ /^http/
              @url = new_url
            else
              @url = "https://#{new_url}"
            end
          end

          def url
            @url || DEFAULT_URL
          end

          def user=(new_user)
            @user = CGI.escape(new_user)
          end

          def passwd=(new_passwd)
            @passwd = new_passwd
          end

          def build_url(path)
            fullurl = url
            return fullurl+path unless @user && @passwd

            fullurl.gsub(/(http(s|):\/\/)/, "\\1#{@user}:#{@passwd}@") + path
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
            unless !params[:source] || params[:source] =~ SOURCE_NAME_REGEX
              raise "Invalid source"
            end

            if counters.length > 0
              params[:counters] = {}
              counters.each_pair do |k, v|
                params[:counters][k] = {:value => v}
              end
            end
            if gauges.length > 0
              params[:gauges] = {}
              gauges.each_pair do |k, v|
                if v.respond_to?(:keys)
                  params[:gauges][k] = v
                else
                  params[:gauges][k] = {:value => Float(v)}
                end
              end
            end

            begin
              r = RestClient.post build_url('/v1/metrics.json'), params
            rescue RestClient::Exception => err
              puts "Failed to publish stats: #{err.message}"
              puts "Response: #{err.response}"
              return false
            rescue => err
              puts "Failed to publish stats (unknown exception): #{err.inspect}"
              return false
            end
            true
          end
        end
      end
    end
  end
end
