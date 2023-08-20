#include "remove_duplicates.h"
#include<iostream>

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

