#include "document.h"
#include <iostream>

Document::Document() = default;

Document::Document(int input_id, double input_relevance, int input_rating) : id(input_id), relevance(input_relevance), rating(input_rating) {}

std::ostream& operator<<(std::ostream& output, const Document& doc) {
    output << "{ document_id = " << doc.id << ", relevance = " << doc.relevance << ", rating = " << doc.rating << " }";
    return output;
}