#include "search_server.h"
#include "string_processing.h"
#include "document.h"

using namespace std::string_literals;

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);

    if (document_id < 0) {
        throw std::invalid_argument("id документа отрицателен");
    }

    if (documents_.count(document_id)) {
        throw std::invalid_argument("Документ с таким id уже есть");
    }

    const double inv_word_count = 1.0 / static_cast<double>(words.size());
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }

    document_id_by_add_order.push_back(document_id);
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
};

int SearchServer::GetDocumentId(int index) const {
    if (index < 0 || index > static_cast<int>(document_id_by_add_order.size())) {
        throw std::out_of_range("Индекс вне диапазона");
    }

    return document_id_by_add_order.at(static_cast<size_t>(index));
};

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    std::vector<std::string> matched_words;
    
    const Query query = ParseQuery(raw_query);

    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }

        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return { matched_words, documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("В документе есть недопустимое слово");
        }

        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }

    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    
    return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    
    if (text[0] == '-' && text.size() > 1) {
        is_minus = true;
        text = text.substr(1);
    }

    if (text == "-"s || CheckForTwoMinuses(text) || !IsValidWord(text)) {
        throw std::invalid_argument("В запросе есть ошибка");
    }

    return { text, is_minus, IsStopWord(text) };
}

bool SearchServer::CheckForTwoMinuses(const std::string& word) {
    return (word[0] == '-' && word[1] == '-') || word[0] == '-';
}

bool SearchServer::IsValidWord(const std::string& word) {
    return none_of(word.begin(), word.end(), 
        [](char c) {
            return c >= '\0' && c < ' ';
        }
    );
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    SearchServer::Query query;

    for (const std::string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }

    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(static_cast<double>(GetDocumentCount()) / static_cast<double>(word_to_document_freqs_.at(word).size()));
}