require 'async'
require 'async/promise'
require_relative 'subscription'
require_relative 'monitored_item'

module Scada
  class Client
    # Async-aware overrides for {Client} methods. Prepended when
    # +scada/async+ is required.
    #
    # Each method checks +Fiber.scheduler+ — if absent, falls back
    # to the sync implementation. When a scheduler is present,
    # operations return +Async::Task+ objects awaitable with +.wait+.
    #
    module AsyncSupport
      # Async-aware connect. Polls +_run_iterate+ until the session
      # activates or a timeout (5s) elapses.
      #
      # @return [self]
      # @raise [Scada::Error] on bad status or timeout
      #
      def connect
        return super unless Fiber.scheduler

        _connect_async

        deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 5.0

        loop do
          _run_iterate

          s = state
          raise_on_bad_status!(s.status, 'Connect failed')
          break if s.session_activated?

          if Process.clock_gettime(Process::CLOCK_MONOTONIC) > deadline
            raise Scada::Error, 'Connect timed out'
          end

          # sleep, not Fiber.yield — with a single fiber, yield returns
          # immediately and busy-loops at 100% CPU. sleep lets the
          # scheduler pause for TICK even when no other fibers exist.
          sleep TICK
        end

        wait_for_namespaces(deadline)
        @was_connected = true
        self
      end

      # Blocking event loop that drives the OPC UA client.
      #
      # Calls +_run_iterate+ each tick, attempts reconnection if the
      # session dropped, and detects session state transitions to fire
      # +on_connect+ / +on_disconnect+ callbacks.
      #
      # The callbacks fire synchronously inside this loop (during
      # +_run_iterate+ processing), so they must not perform blocking
      # SCADA operations — use a deferred queue or flag.
      #
      # @note Blocks the current fiber indefinitely. Run it in its
      #   own +Async+ task.
      #
      def run
        return super unless Fiber.scheduler

        session_was_activated = false
        loop do
          _run_iterate
          maybe_reconnect

          activated = state.session_activated?
          if activated && !session_was_activated
            session_was_activated = true
            @on_connect&.call
          elsif !activated && session_was_activated
            session_was_activated = false
            @on_disconnect&.call
          end

          sleep TICK
        end
      end

      # Async read. Returns an {Async::Task} that resolves to a
      # {DataValue} (single) or array.
      #
      # @param node_ids [Array<String, NodeId>]
      # @param attribute [Symbol] attribute to read (default +:value+)
      # @return [Async::Task<DataValue>]
      #
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

      # Async write. Returns an {Async::Task} that resolves when the write completes.
      #
      # @param node_id [String, NodeId]
      # @param value [Object]
      # @param type [Symbol, nil] SCADA type hint (e.g. +:double+, +:uint32+)
      # @return [Async::Task<nil>]
      #
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

      # Async method call. Returns an {Async::Task} that resolves to the method result.
      #
      # @param node_id [String, NodeId]
      # @param args [Array<Object>] input arguments
      # @return [Async::Task<Object>]
      #
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

      # Creates a new subscription for monitoring data changes or events.
      #
      # @param publish_interval [Float] seconds between server publish cycles
      # @return [Subscription]
      # @raise [Scada::Error] if the server rejects the subscription
      #
      def subscribe(publish_interval: 0.1)
        return super unless Fiber.scheduler

        condition = Async::Promise.new
        _create_subscription_async(publish_interval, condition)
        status, sub_id = condition.wait
        check_async_status!(status)
        Subscription.new(self, sub_id)
      end

      private

      # Attempts to reconnect if the session dropped. Called each tick from {#run}.
      # Silently swallows errors — reconnection will be retried on the next tick.
      #
      def maybe_reconnect
        return unless @was_connected
        s = state
        return if s.session_activated?

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
