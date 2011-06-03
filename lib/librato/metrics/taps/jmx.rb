module Librato
  module Metrics
    module Taps
      module JMX
        class << self
          def connect!(host = 'localhost', port = 3000)
            begin
              ::JMX::MBean.establish_connection :host => host, :port => port
            rescue => err
              return false
            end
            @connected = true
          end

          def match_beans(beans)
            raise "Not connected" unless @connected || connect!

            begin
              b = ::JMX::MBean.find_all_by_name(beans.to_s)
            rescue => err
              return nil
            end

            b.collect { |bean| bean.object_name.to_s }
          end

          #
          # Retrieves a list of JMX attributes converted to
          # gauges and counters.
          #
          # Argument options:
          #
          #    beans = ['bean1', 'bean2']
          #      Will retrieve each attribute of each beanname as
          #      a gauge with a count value of 1.
          #
          #    beans = {'bean1' => ['attr1', 'attr2'], 'bean2' => true}
          #      Will retrieve only the specified attributes for each
          #      bean name. To retrieve all attributes, simply set
          #      to true instead of a list.
          #
          #    beans = {'bean1' => {'attr' => 'gauge',
          #                         'attr2' => 'counter'}}
          #      Will retrieve the specified beans and their
          #      attributes like the example above, but with the
          #      ability to specify how each attribute is returned. By
          #      default, all attributes are returned as
          #      gauges. However, if you set the attribute name to
          #      'counter', it will return the attribute as a counter.
          #
          #
          def retrieve(bean_names)
            raise "Not connected" unless @connected || connect!

            gauges = {}
            counters = {}
            (bean_names.keys rescue bean_names).each do |bean|
              begin
                b = ::JMX::MBean.find_by_name(bean.to_s)
              rescue
                raise "No such bean: #{bean}"
              end

              if bean_names.respond_to?(:keys) &&
                  ( bean_names[bean].class == Array ||
                    bean_names[bean].class == Hash)
                attrs = bean_names[bean]
              else
                attrs = b.attributes.keys
              end

              attrs.each do |attr|
                attrname = attr.first
                begin
                  value = b.send(snake_case(attrname.to_s))
                rescue
                  raise "Bean #{bean} has no such attribute: #{attrname}"
                end

                # Skip attributes without a value
                next unless value

                # Check how we are interpretting this value
                if attrs.respond_to?(:keys) && "#{attr.last}".downcase == 'counter'
                  counters[metric_name(bean, attrname)] = value
                else
                  # Skip NaN
                  next if Float(value).nan?

                  gauges[metric_name(bean, attrname)] = value
                end
              end
            end

            [counters, gauges]
          end

          # From ActiveSupport
          def snake_case(camel_cased_word)
            camel_cased_word.to_s.gsub(/::/, '/').
              gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
              gsub(/([a-z\d])([A-Z])/,'\1_\2').
              tr("-", "_").
              downcase
          end

          def metric_name(bean_name, attr_name)
            "#{bean_name.gsub('=', ':').gsub(',', '_')}::#{attr_name}"
          end
        end
      end
    end
  end
end
