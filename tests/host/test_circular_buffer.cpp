#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>

// This will pull the data_structure, but use the mocked <atomic.hpp>, <hal/constants.hpp> etc.
#include <data_structures/atomic_cyclic_buffer.hpp>

using namespace data_structures;

TEST(AtomicCyclicBuffer, SingleProducerSingleConsumer)
{
    // 64 byte buffer (must be power of 2 based on your code)
    AtomicCyclicBuffer<int, 64> buffer;
    const int kNumItems = 100000;
    std::atomic<bool> done{false};

    std::thread producer([&]() {
        for (int i = 0; i < kNumItems; ++i) {
            // Spin until space is available
            while (buffer.Write(std::span<const int>(&i, 1)) == 0) {
                std::this_thread::yield();
            }
        }
        done = true;
    });

    std::thread consumer([&]() {
        int received_count = 0;
        int val;
        while (received_count < kNumItems) {
            if (buffer.Read(std::span<int>(&val, 1)) > 0) {
                // Verify ordering
                ASSERT_EQ(val, received_count);
                received_count++;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
}
