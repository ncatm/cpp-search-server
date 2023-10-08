#include "search_server.h"

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(
        SplitIntoWords(stop_words_text))
{
}
SearchServer::SearchServer(std::string_view stop_words_text) 
    : SearchServer(SplitIntoWordsView(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
    const std::vector<int>& ratings) {

    if (document_id < 0) {
        throw std::invalid_argument("документ с отрицательным id"s);
    }
    if (documents_.count(document_id)) {
        throw std::invalid_argument("документ c id ранее добавленного документа"s);
    }
    if (!IsValidWord(document)) {
        throw std::invalid_argument("наличие недопустимых символов"s);
    }

    auto words = SplitIntoWordsNoStop(document);
    const std::string document_string{ document };
    documents_.emplace(document_id, DocumentData{ SearchServer::ComputeAverageRating(ratings), status, document_string });

    words = SplitIntoWordsNoStop(documents_.at(document_id).text_);

    for (auto word : words) {
        word_to_document_freqs_[word][document_id] += 1.0 / words.size();
        document_to_word_freqs_[document_id][word] += 1.0 / words.size();
    }

    document_ids_.emplace(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}


std::map<std::string_view, double> SearchServer::GetWordFrequencies(const int& document_id) const {
    return document_to_word_freqs_.at(document_id);
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument(std::string("Query word is empty"));
    }
    auto word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument(std::string("Query word is invalid"));
    }

    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
    Query result;

    for (auto word : SplitIntoWordsView(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            }
            else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }

    sort(result.minus_words.begin(), result.minus_words.end());
    sort(result.plus_words.begin(), result.plus_words.end());

    auto last_minus = unique(result.minus_words.begin(), result.minus_words.end());
    auto last_plus = unique(result.plus_words.begin(), result.plus_words.end());

    size_t newSize = last_minus - result.minus_words.begin();
    result.minus_words.resize(newSize);

    newSize = last_plus - result.plus_words.begin();
    result.plus_words.resize(newSize);

    return result;
}

SearchServer::Query SearchServer::ParseQueryParallel(std::string_view text) const {
    Query result;

    for (auto word : SplitIntoWordsView(text)) {
        const QueryWord query_word(ParseQueryWord(word));
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            }
            else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }
    return result;
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
    int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string_view> matched_words;
    for (auto word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (auto word : query.minus_words) {
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

void SearchServer::RemoveDocument(int document_id) {
    documents_.erase(document_id);
    document_ids_.erase(document_id);

    auto iter = document_to_word_freqs_.find(document_id);

    for (auto& it : (*iter).second) {
        word_to_document_freqs_.at(it.first).erase(document_id);
    }

    this->document_to_word_freqs_.erase(iter);
}

std::set<int>::iterator SearchServer::begin() const {
    return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end() const {
    return document_ids_.end();
}

int SearchServer::GetDocumentCount() const {
    return int(documents_.size());
}
bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(std::string_view word) {
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string_view> words;
    for (auto word : SplitIntoWordsView(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument(std::string("Word is invalid"));
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

    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

void RemoveDuplicates(SearchServer& search_server) {
    std::vector<int> remove_list;
    std::set<std::string> word_list;

    for (auto iter = search_server.begin(); iter != search_server.end(); ++iter) {
        std::string word;
        for (auto i : search_server.GetWordFrequencies(*iter)) {
            word += i.first;
        }
        if (!word_list.insert(word).second) {
            remove_list.push_back(*iter);
        }
    }
    for (const int& iter : remove_list)
    {
        std::cout << "Found duplicate document id " << iter << '\n';
        search_server.RemoveDocument(iter);
    }
}