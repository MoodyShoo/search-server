#pragma once
#include <algorithm>
#include <deque>
#include <string>

#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        unsigned int query_time;
        bool empty;
    };

    void DeleteTimedOutRequests();

    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    unsigned int current_time_ = 1;
    const static int min_in_day_ = 1440;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    auto matched_documents = search_server_.FindTopDocuments(raw_query, document_predicate);

    if (matched_documents.size() > 0) {
        requests_.push_back({ current_time_, false });
    } else {
    requests_.push_back({ current_time_, true });
    }

    ++current_time_;

    DeleteTimedOutRequests();

    return matched_documents;
}