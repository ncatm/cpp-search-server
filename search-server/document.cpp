#include "document.h"
using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const Document& doc)
{
    out << std::string("{ document_id = "s) 
        << doc.id << std::string(", relevance = "s) 
        << doc.relevance << std::string(", rating = "s) 
        << doc.rating << std::string(" }"s);
    return out;
}