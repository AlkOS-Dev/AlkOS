#ifndef ALKOS_KERNEL_TEST_TEST_STRUCTS_HPP_
#define ALKOS_KERNEL_TEST_TEST_STRUCTS_HPP_

class MoveOnlyInt
{
    public:
    MoveOnlyInt(int value) : value_(value) {}

    MoveOnlyInt(const MoveOnlyInt &)            = delete;
    MoveOnlyInt &operator=(const MoveOnlyInt &) = delete;

    MoveOnlyInt(MoveOnlyInt &&other) : value_(other.value_) { other.value_ = 0; }

    MoveOnlyInt &operator=(MoveOnlyInt &&other)
    {
        if (this != &other) {
            value_       = other.value_;
            other.value_ = 0;
        }
        return *this;
    }

    int getValue() const { return value_; }

    private:
    int value_;
};

struct alignas(16) AlignedStruct {
    double x;
    double y;
};

class CustomString
{
    public:
    CustomString() : data_{}, size_(0) {}

    CustomString(const char *str)
    {
        size_ = 0;
        while (str[size_] != '\0' && size_ < 63) {
            data_[size_] = str[size_];
            size_++;
        }
        data_[size_] = '\0';
    }

    bool operator==(const CustomString &other) const
    {
        if (size_ != other.size_)
            return false;
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] != other.data_[i])
                return false;
        }
        return true;
    }

    bool operator==(const char *str) const
    {
        size_t i = 0;
        while (str[i] != '\0' && i < size_) {
            if (data_[i] != str[i])
                return false;
            ++i;
        }
        return str[i] == '\0' && i == size_;
    }

    private:
    char data_[256];
    size_t size_;
};

struct ComplexType {
    double values[4];
    CustomString name;

    bool operator==(const ComplexType &other) const
    {
        for (int i = 0; i < 4; ++i) {
            if (values[i] != other.values[i])
                return false;
        }
        return name == other.name;
    }
};

#endif  // ALKOS_KERNEL_TEST_TEST_STRUCTS_HPP_
