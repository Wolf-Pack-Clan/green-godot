#ifndef EFX_PARSER_H
#define EFX_PARSER_H

#include "thirdparty/tree-sitter/include/tree_sitter/api.h"
#include "parser/src/tree_sitter/parser.h"
#include "parser/bindings/c/tree_sitter/tree-sitter-efx.h"
#include "core/string_name.h"
#include "core/map.h"
#include "core/variant.h"

extern "C" {
    const TSLanguage *tree_sitter_efx(void);
}

struct EFXBlock {
    String type;
    String subtype;
    Map<String, Variant> properties;
};

class EFXParser {
    const TSParser *parser;
    const TSLanguage *language;
};

#endif // EFX_PARSER_H
