
#include "document.h"

std::ostream& operator<<(std::ostream& out, const Document& doc)
{
    out << std::string("{ document_id = ") << doc.id << std::string(", relevance = ") << doc.relevance << std::string(", rating = ") << doc.rating << std::string(" }");
    return out;
}