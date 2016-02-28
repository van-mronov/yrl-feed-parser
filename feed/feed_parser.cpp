#include "feed_parser.h"

#include <cstring>
#include <fstream>
#include <glibmm/convert.h>
#include <iostream>
#include <unordered_set>

namespace realty {

static const size_t PARSE_FEED_BUFFER_SIZE = 4096;

void test_offer_callback(offer_node_ptr&& offer)
{
  std::cout << "test_offer_callback" << std::endl;
}

void parse_feed(const std::string& fname)
{
  std::ifstream ifs(fname);
  char buffer[PARSE_FEED_BUFFER_SIZE];
  realty::feed_parser parser(std::move(test_offer_callback));
  parser.set_substitute_entities(true);
  do {
    std::memset(buffer, 0, PARSE_FEED_BUFFER_SIZE);
    ifs.read(buffer, PARSE_FEED_BUFFER_SIZE - 1);
    if (ifs.gcount()) {
      // We use Glib::ustring::ustring(InputIterator begin, InputIterator end)
      // instead of Glib::ustring::ustring( const char*, size_type ) because it
      // expects the length of the string in characters, not in bytes.
      Glib::ustring input(buffer, buffer + ifs.gcount());
      parser.parse_chunk(input);
    }
  }
  while (ifs);
}

feed_parser::feed_parser(fn_offer_callback&& offer_callback)
  : xmlpp::SaxParser(), m_offer_callback(offer_callback)
{
  std::locale::global(std::locale(""));
}

feed_parser::~feed_parser()
{
  // Empty
}

void feed_parser::on_start_document()
{
  std::cout << "on_start_document" << std::endl;
}

void feed_parser::on_end_document()
{
  std::cout << "on_end_document" << std::endl;
}

void feed_parser::on_start_element(
  const Glib::ustring& name, const AttributeList& attributes)
{
  if (name == "offer") {
    std::cout << name << " (";
    for (const auto& attr_pair : attributes) {
      try {
        std::cout << attr_pair.name;
      }
      catch (const Glib::ConvertError& e) {
        std::cerr << "feed_parser::on_start_element(): Exception caught while converting name for std::cout: " << e.what() << std::endl;
      }

      try {
        std::cout << " = " <<  attr_pair.value << ")";
      }
      catch (const Glib::ConvertError& e) {
        std::cerr << "feed_parser::on_start_element(): Exception caught while converting value for std::cout: " << e.what() << std::endl;
      }
    }

    m_offer_root = std::make_shared<offer_node>();
    m_offer_root->name(name);
    m_current_offer_node = m_offer_root;
  }
  else if (m_current_offer_node != nullptr) {
    if (m_current_offer_node->children().empty()) std::cout << std::endl;
    offer_node_ptr new_node = std::make_shared<offer_node>();
    new_node->name(name);
    m_current_offer_node->add_child(new_node);
    m_current_offer_node = new_node;
    m_offset += "  ";
    std::cout << m_offset << m_current_offer_node->name();
  }
}

void feed_parser::on_end_element(const Glib::ustring& name)
{
  if (name == "offer") {
    std::cout << "end offer" << std::endl << std::endl;
    m_offer_callback(std::move(m_offer_root));
    m_current_offer_node.reset();
    m_offset.clear();
    ::abort();
  }
  else if (m_current_offer_node != nullptr) {
    if ((m_current_offer_node->children().empty())
      && (!m_current_offer_node->data().empty())) {
      try {
        std::cout << ": " << m_current_offer_node->data();
      }
      catch (const Glib::ConvertError& e) {
        std::cout << "unable to convert data: " << e.what();
      }
      std::cout << std::endl;
    }
    m_current_offer_node = m_current_offer_node->parent();
    m_offset.erase(m_offset.end() - 2, m_offset.end());
  }
}

void feed_parser::on_characters(const Glib::ustring& text)
{
  if (m_current_offer_node != nullptr) {
    m_current_offer_node->data(text);
  }
}

void feed_parser::on_warning(const Glib::ustring& text)
{
  try {
    std::cout << "on_warning(): " << text << std::endl;
  }
  catch (const Glib::ConvertError& e) {
    std::cerr << "feed_parser::on_warning(): Exception caught while converting text for std::cout: " << e.what() << std::endl;
  }
}

void feed_parser::on_error(const Glib::ustring& text)
{
  try {
    std::cout << "on_error(): " << text << std::endl;
  }
  catch (const Glib::ConvertError& e) {
    std::cerr << "feed_parser::on_error(): Exception caught while converting text for std::cout: " << e.what() << std::endl;
  }
}

void feed_parser::on_fatal_error(const Glib::ustring& text)
{
  try {
    std::cout << "on_fatal_error(): " << text << std::endl;
  }
  catch (const Glib::ConvertError& e) {
    std::cerr << "feed_parser::on_characters(): Exception caught while converting value for std::cout: " << e.what() << std::endl;
  }
}

} // namespace realty
