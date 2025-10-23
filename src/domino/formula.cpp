#include "formula.hpp"

#include <domino/nsfsearc.h>

#include <vector>

#include "../utils/error.hpp"
#include "../utils/parser.hpp"

Formula::Formula(std::string formula_str) {
  WORD t = NULL;
  DHANDLE formula_handle = NULLHANDLE;

  STATUS err = NSFFormulaCompile(nullptr, 0, formula_str.data(), formula_str.size(),
                                 &formula_handle, &this->formula_size, &t, &t, &t, &t, &t);
  if (err != NOERROR) {
    throw NotesException(err, "NSFFormulaCompile error");
  }

  this->formula_obj = new OSObject(formula_handle);
}

[[nodiscard]] auto readTextList(OSObject* obj, WORD data_len) -> const std::string {
  // Verfy we have data
  if (data_len < 8) {
    return std::string{""};
  }

  // Get the amount of entries
  auto entries = obj->get<WORD>();
  auto* lengths = obj->get_raw<const WORD*>();

  // Concatenate all entries
  std::string output = "";
  for (WORD i = 0; i < entries; i++) {
    auto* content = reinterpret_cast<const char*>(lengths + entries);
    output += std::string(content, lengths[i]);
  }

  return output;
}

auto Formula::evaluate(NOTEHANDLE note_handle) -> std::string {
  // Start the computation
  auto* compiled_formula = this->formula_obj->get_raw<char*>();
  STATUS err = NSFComputeStart(NULL, compiled_formula, &this->compute_handle);
  if (err != NOERROR) {
    NSFComputeStop(compute_handle);
    throw NotesException(err, "NSFComputeStart error");
  }

  // Evaluate the formula
  DHANDLE result = NULLHANDLE;
  WORD result_len = NULL;
  err = NSFComputeEvaluate(compute_handle, note_handle, &result, &result_len, nullptr, nullptr,
                           nullptr);
  if (err != NOERROR) {
    NSFComputeStop(compute_handle);
    throw NotesException(err, "NSFComputeEvaluate error");
  }

  // Process the result
  if (result != NULLHANDLE) {
    auto result_obj = new OSObject(result);
    auto data_type = result_obj->get<WORD>();

    // Read (if text list)
    if (data_type == TYPE_TEXT_LIST) {
      auto item_buffer = result_obj->get<USHORT>(result_len);
      return Parser::parse_text_list(item_buffer);
    }
  }

  NSFComputeStop(compute_handle);
  return std::string{};
}