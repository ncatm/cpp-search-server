#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    std::vector<std::vector<Document>> result(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(), [&search_server](auto& query)
        {return search_server.FindTopDocuments(query); });
    return result;

}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    auto process_queries = ProcessQueries(search_server, queries);
    std::vector<Document> result_querie;
    for (auto doc : process_queries) {
        result_querie.insert(result_querie.end(), doc.begin(), doc.end());
    }
    return result_querie;
}