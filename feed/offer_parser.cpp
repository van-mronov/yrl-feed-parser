#include "offer_parser.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>

using boost::lexical_cast;
using boost::bad_lexical_cast;

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;

namespace realty {
namespace feed {

namespace {

template<typename T>
bool try_add(const std::string& name, offer_node& node, document& doc_stream)
{
  try {
    doc_stream << name << lexical_cast<T>(node.data());
  }
  catch (const bad_lexical_cast&) {
    return false;
  }
  return true;
}

void parse_offer(offer_node& node, document& doc_stream)
{
  std::string name = node.name().raw();
  std::replace(name.begin(), name.end(), '-', '_');

  if (node.has_children()) {
    if (node.name() != "offer") doc_stream << name << open_document;
    for (offer_children_type::const_reference child : node.children()) {
      parse_offer(*child, doc_stream);
    }
    if (node.name() != "offer") doc_stream << close_document;
    return;
  }
  if (try_add<long int>(name, node, doc_stream)) return;
  if (try_add<double>(name, node, doc_stream)) return;
  if (node.data() == "true") doc_stream << name << true;
  else if (node.data() == "false") doc_stream << name << false;
  else doc_stream << name << node.data();
}

} // anonymous namespace

void parse_offer(offer_node&& offer, mongocxx::database& db)
{
  document doc_stream = document{};
  parse_offer(offer, doc_stream);
  bsoncxx::document::value offer_doc = doc_stream << finalize;
  std::cout << bsoncxx::to_json(offer_doc) << std::endl;
  throw std::exception();
  auto res = db["offers"].insert_one(std::move(offer_doc));
}

} // namespace feed
} // namespace realty
