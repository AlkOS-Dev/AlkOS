#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <numeric>
#include <random>
#include <span>
#include <thread>
#include <vector>

#include <data_structures/atomic_cyclic_buffer.hpp>

using namespace data_structures;

class AtomicCyclicBufferConcurrencyTest : public ::testing::Test
{
    protected:
    static constexpr size_t kBufSize = 64;
    AtomicCyclicBuffer<int, kBufSize> buffer;
};

TEST_F(AtomicCyclicBufferConcurrencyTest, SPSC_GivenHighContentionStreaming_MaintainsDataIntegrity)
{
    // Description:
    // Given: A shared buffer and high volume of data (200k items)
    // When: Producer and Consumer run concurrently
    // Then: Every item is transferred exactly once and in order

    const int kNumItems = 200000;
    std::atomic<bool> producer_done{false};

    std::thread producer([&]() {
        for (int i = 0; i < kNumItems; ++i) {
            int val = i;
            // Spin until space available
            while (buffer.Write(std::span<const int>(&val, 1)) == 0) {
                std::this_thread::yield();
            }
        }
        producer_done = true;
    });

    std::thread consumer([&]() {
        int received_count = 0;
        int val            = -1;
        while (received_count < kNumItems) {
            if (buffer.Read(std::span<int>(&val, 1)) > 0) {
                ASSERT_EQ(val, received_count)
                    << "Data ordering violation at index " << received_count;
                received_count++;
            } else {
                if (producer_done && buffer.IsEmpty() && received_count < kNumItems) {
                    // Should not happen if logic is correct
                    std::this_thread::yield();
                }
            }
        }
    });

    producer.join();
    consumer.join();
}

TEST(AtomicCyclicBufferSPSC, SPSC_GivenMoveOnlyTypes_TransfersOwnershipCorrectly)
{
    // Description:
    // Given: Buffer of unique_ptrs
    // When: Streaming moves across threads
    // Then: Pointers are valid on receive and order is maintained

    const int kNumItems              = 50000;
    static constexpr size_t kBufSize = 128;
    AtomicCyclicBuffer<std::unique_ptr<int>, kBufSize> buffer;

    std::thread producer([&]() {
        for (int i = 0; i < kNumItems; ++i) {
            auto ptr = std::make_unique<int>(i);

            // Loop until write succeeds
            while (true) {
                // We must put the pointer in a container to pass as span
                std::vector<std::unique_ptr<int>> batch;
                batch.push_back(std::move(ptr));

                // Try write
                if (buffer.Write(std::span<std::unique_ptr<int>>(batch)) > 0) {
                    // Success, batch[0] is now moved-from, we are done with 'i'
                    break;
                } else {
                    // Failed (buffer full), restore ptr to try again
                    ptr = std::move(batch[0]);
                    std::this_thread::yield();
                }
            }
        }
    });

    std::thread consumer([&]() {
        int received_count = 0;
        while (received_count < kNumItems) {
            std::unique_ptr<int> out_arr[1];
            if (buffer.Read(std::span<std::unique_ptr<int>>(out_arr)) > 0) {
                ASSERT_NE(out_arr[0], nullptr);
                ASSERT_EQ(*out_arr[0], received_count);
                received_count++;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
}

TEST(AtomicCyclicBufferSPSC, SPSC_GivenRandomBatchSizes_OperatesCorrectly)
{
    // Description:
    // Given: Shared buffer
    // When: Producer writes random batches [1..N], Consumer reads random batches [1..M]
    // Then: Data consistency is maintained

    const int kTotalItems            = 100000;
    static constexpr size_t kBufSize = 256;
    AtomicCyclicBuffer<int, kBufSize> buffer;
    std::atomic<bool> producer_done{false};

    std::thread producer([&]() {
        std::mt19937 rng(12345);
        std::uniform_int_distribution<size_t> batch_dist(
            1, kBufSize
        );  // Try writing up to capacity

        int current_val = 0;
        while (current_val < kTotalItems) {
            size_t batch_size = batch_dist(rng);
            // Don't produce more than total
            if (current_val + batch_size > kTotalItems) {
                batch_size = kTotalItems - current_val;
            }

            // Prepare batch
            std::vector<int> batch(batch_size);
            std::iota(batch.begin(), batch.end(), current_val);

            // Write batch (potentially partial)
            size_t items_written = 0;
            while (items_written < batch_size) {
                // span of remaining items
                std::span<const int> remaining(
                    batch.data() + items_written, batch_size - items_written
                );

                size_t n = buffer.Write(remaining);
                items_written += n;

                if (n == 0)
                    std::this_thread::yield();
            }

            current_val += batch_size;
        }
        producer_done = true;
    });

    std::thread consumer([&]() {
        std::mt19937 rng(67890);
        std::uniform_int_distribution<size_t> batch_dist(1, kBufSize);

        int next_expected = 0;
        std::vector<int> read_buf(kBufSize);

        while (next_expected < kTotalItems) {
            size_t request_size = batch_dist(rng);

            size_t n = buffer.Read(std::span<int>(read_buf.data(), request_size));

            for (size_t i = 0; i < n; ++i) {
                ASSERT_EQ(read_buf[i], next_expected++);
            }

            if (n == 0)
                std::this_thread::yield();
        }
    });

    producer.join();
    consumer.join();
}
