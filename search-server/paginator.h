#pragma once

#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    Iterator begin() const {
        return begin_;
    }

    Iterator end() const {
        return end_;
    }

    size_t size() const {
        return distance(begin_, end_);
    }

private:
    Iterator begin_;
    Iterator end_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        auto it_first = begin;
        auto it_last = begin;
        size_t counter = 0;
        do {
            if (counter == page_size) {
                pages_.push_back(IteratorRange<Iterator>(it_first, it_last));
                it_first = it_last;
                counter = 0;
            }
            else {
                ++it_last;
                ++counter;
            }
        } while (it_last != end);
        if (it_first != end) {
            pages_.push_back(IteratorRange<Iterator>(it_first, end));
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    size_t size() {
        return pages_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& output, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it < range.end(); ++it) {
        output << *it;
    }

    return output;
}