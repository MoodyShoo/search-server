#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server) : search_server_(search_server) {}

int RequestQueue::GetNoResultRequests() const {
    return std::count_if(requests_.begin(), requests_.end(),
        [](const QueryResult& result) {
            return result.empty == true;
        }
    );
}

void RequestQueue::DeleteTimedOutRequests() {
    while (!requests_.empty() && requests_.front().query_time < current_time_ - min_in_day_) {
        requests_.pop_front();
    }
}