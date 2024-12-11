#include <boost/url.hpp>
#include <boost/url/url.hpp>
#include <boost/url/parse.hpp>
#include "../../include/logger.h"
#include "../../include/crawler/parser.h"

Parser::Parser() :logger_(Logger::GetLogger()) {}

Parser::~Parser() {}

inline std::string PercentEncodeBraces(const std::string& url) {
    std::string encoded = url;
    size_t pos = 0;
    while ((pos = encoded.find('{', pos)) != std::string::npos) {
        encoded.replace(pos, 1, "%7B");
        pos += 3; // "%7B" 的长度
    }
    pos = 0;
    while ((pos = encoded.find('}', pos)) != std::string::npos) {
        encoded.replace(pos, 1, "%7D");
        pos += 3; // "%7D" 的长度
    }
    return encoded;
}

/**
 * @brief 递归遍历 Gumbo 解析树，提取所有 <a> 标签的 href 属性。
 *
 * @param node 当前遍历的 Gumbo 节点。
 * @param links 用于存储解析出的链接的引用。
 * @param base_url 基础 URL，用于解析相对链接。
 */
void Parser::ExtractLinks(GumboNode* node, std::vector<std::string>& links, const std::string& base_url) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    // 检查当前节点是否为 <a> 标签
    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href && href->value) {
            std::string link = href->value;
            link = PercentEncodeBraces(link);

            try {
                // 解析相对或绝对 URL
                boost::urls::result<boost::urls::url> uri_ref_result = boost::urls::parse_uri_reference(link);
                if (!uri_ref_result.has_value()) {
                    throw boost::system::system_error(uri_ref_result.error());
                }
                boost::urls::url uri_ref = uri_ref_result.value();

                // 解析基础 URL
                boost::urls::result<boost::urls::url> base_result = boost::urls::parse_uri(base_url);
                if (!base_result.has_value()) {
                    throw boost::system::system_error(base_result.error());
                }
                boost::urls::url base = base_result.value();

                // 解析 URI 引用为绝对 URL
                boost::urls::url absolute;
                boost::urls::resolve(base, uri_ref, absolute);

                // 将绝对 URL 转换为字符串并添加到链接列表中
                links.emplace_back(absolute.c_str());
            }
            catch (const boost::system::system_error& e) {
                // 记录解析失败的链接，但不影响其他链接的解析
                logger_->Warn("Failed to parse URL: " + link + " Error: " + e.what());
            }
        }
    }

    // 递归遍历子节点
    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        ExtractLinks(static_cast<GumboNode*>(children->data[i]), links, base_url);
    }
}


/**
 * @brief 解析 HTML 内容中的所有链接，并将相对链接解析为绝对链接。
 *
 * @param html_content 要解析的 HTML 内容。
 * @param base_url 基础 URL，用于解析相对链接。
 * @return ParsedLinks 包含解析出的所有链接的结构体。
 */
ParsedLinks Parser::ParseLinks(const std::string& html_content, const std::string& base_url) {
    ParsedLinks parsed_links;

    // 使用 Gumbo 解析 HTML 内容
    GumboOutput* output = gumbo_parse(html_content.c_str());

    // 从解析树的根节点开始提取链接
    ExtractLinks(output->root, parsed_links.links, base_url);

    // 释放 Gumbo 解析树占用的内存
    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return parsed_links;
}