#ifndef __PARSER_H__
#define __PARSER_H__

#include <gumbo.h>
#include <string>
#include "../../include/logger.h"
#include "../common_types.h"

class Parser {
public:
    Parser();
    ~Parser();

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    ParsedLinks ParseLinks(const std::string& html_content, const std::string& base_url);

private:
    std::shared_ptr<Logger> logger_;

    void ExtractLinks(GumboNode* node, std::vector<std::string>& links, const std::string& base_url);
};

#endif // __PARSER_H__