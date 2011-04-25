module Librato
  module Metrics
    module Taps
      module Publisher
        class << self
          SOURCE_NAME_REGEX = /^[-A-Za-z0-9_.]{1,255}$/
          DEFAULT_URL = 'https://metrics-stg.heroku.com'

          @source = nil
          @url = nil
          @user = nil
          @passwd = nil

          def source=(new_source)
            raise "Invalid source" unless new_source =~ SOURCE_NAME_REGEX

            source = new_source
          end

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
          def post(counters, gauges, measure_time = nil)
            params = {
              :measure_time => measure_time || Time.now.tv_sec
            }
            params[:source] = @source if @source

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
            rescue => err
              puts "Failed to publish stats: #{err.message}"
              return false
            end
            true
          end
        end
      end
    end
  end
end
