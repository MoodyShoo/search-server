#pragma once
#include <algorithm>
#include <deque>
#include <string>

#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        auto matched_documents = search_server_.FindTopDocuments(raw_query, document_predicate);

        if (matched_documents.size() > 0) {
            requests_.push_back({ current_time_, false });
        }
        else {
            requests_.push_back({ current_time_, true });
        }

        ++current_time_;

        DeleteTimedOutRequests();

        return matched_documents;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        return AddFindRequest(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            }
        );
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        int query_time;
        bool empty;
    };

    void DeleteTimedOutRequests();

    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    int current_time_ = 1;
    const static int min_in_day_ = 1440;
};