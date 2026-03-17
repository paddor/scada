require 'async'
require 'async/promise'
require_relative 'subscription'
require_relative 'monitored_item'

module Scada
  class Client
    module AsyncSupport
      def connect
        if Fiber.scheduler
          _connect_async
          deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 5.0
          loop do
            _run_iterate
            s = state
            raise_on_bad_status!(s.status, 'Connect failed')
            break if s.session == SESSION_ACTIVATED
            if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
              raise Scada::Error, 'Connect timed out'
            end
            sleep TICK
          end
          wait_for_namespaces(deadline)
          @was_connected = true
          self
        else
          super
        end
      end

      def run
        return super unless Fiber.scheduler

        loop do
          _run_iterate
          maybe_reconnect
          sleep TICK
        end
      end

      def read(*node_ids, attribute: :value)
        if Fiber.scheduler
          nids = node_ids.map { |n| parse_node_id(n) }
          if nids.length == 1
            read_single_async(nids[0], attribute)
          else
            Async do
              nids.map { |nid| read(nid, attribute: attribute).wait }
            end
          end
        else
          super
        end
      end

      def write(node_id, value, type: nil)
        if Fiber.scheduler
          nid = parse_node_id(node_id)
          Async do
            condition = Async::Promise.new
            _write_async(nid, value, type, condition)
            status, _result = condition.wait
            check_async_status!(status)
          end
        else
          super
        end
      end

      def call(node_id, *args)
        if Fiber.scheduler
          nid = parse_node_id(node_id)
          Async do
            condition = Async::Promise.new
            _call_async(nid, args, condition)
            status, result = condition.wait
            check_async_status!(status)
            result
          end
        else
          super
        end
      end

      def subscribe(publish_interval: 0.1)
        return super unless Fiber.scheduler

        condition = Async::Promise.new
        _create_subscription_async(publish_interval, condition)
        status, sub_id = condition.wait
        check_async_status!(status)
        Subscription.new(self, sub_id)
      end

      private

      def maybe_reconnect
        return unless @was_connected
        s = state
        return if s.session == SESSION_ACTIVATED

        _connect_async
      rescue Scada::Error
        # Connect attempt failed, will retry next tick
      end

      def read_single_async(nid, attribute)
        Async do
          condition = Async::Promise.new
          _read_async(nid, attribute, condition)
          status, result = condition.wait
          check_async_status!(status)
          attribute == :value ? result : result&.value
        end
      end
    end

    prepend AsyncSupport
  end
end
