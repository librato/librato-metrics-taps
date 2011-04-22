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
          def retrieve(bean_names)
            raise "Not connected" unless @connected || connect!

            ret = Hash.new{|hash, key| hash[key] = {}}
            (bean_names.keys rescue bean_names).each do |bean|
              begin
                b = ::JMX::MBean.find_by_name(bean.to_s)
              rescue
                raise "No such bean: #{bean}"
              end

              if bean_names.respond_to?(:keys) && bean_names[bean].class == Array
                attrs = bean_names[bean]
              else
                attrs = b.attributes.keys
              end
              attrs.each do |attr|
                begin
                  ret[bean][attr] = b.send(snake_case(attr.to_s))
                rescue
                  raise "Bean #{bean} has no such attribute: #{attr}"
                end
              end
            end

            ret
          end

          # From ActiveSupport
          def snake_case(camel_cased_word)
            camel_cased_word.to_s.gsub(/::/, '/').
              gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
              gsub(/([a-z\d])([A-Z])/,'\1_\2').
              tr("-", "_").
              downcase
          end
        end
      end
    end
  end
end
