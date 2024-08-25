#pragma once

#include <set>
#include <string>
#include <vector>
#include <unordered_map>

#include "link.h"


std::string getHtmlContent(const Link& link);

std::vector<Link> extract_links(const std::string& html);

std::vector<Link> get_new_unique_links(const std::vector<Link>& links, std::set<Link> &existing_links);

std::unordered_map<std::string, int> countWordFrequency(std::string text, int minLength = 3, int maxLength = 35);
