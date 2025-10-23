/**
 * @file Green Godot effect file parser
 * @author Kazam <mrcreamy112233@gmail.com>
 * @license GNU GPLv3
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

module.exports = grammar({
  name: 'efx',

  extras: $ => [
    /\s/,
    $.comment
  ],

  rules: {
    source_file: $ => repeat(choice(
      $.block,
      $.comment
    )),

    block: $ => seq(
      $.block_header,
      '{',
      repeat(choice($.property, $.transition)),
      '}'
    ),

    block_header: $ => seq(
      $.identifier,
      optional(seq(':', $.identifier))
    ),

    property: $ => seq(
      $.identifier,
      $.value
    ),
    
    transition: $ => seq(
      '@transition',
      '(',
      //alias('property_name', $.identifier),
      $.identifier,
      ')',
      '[',
      'start',
      $.value,
      'end',
      $.value,
      ']'
    ),
    
    transition_start: $ => seq(
      'start',
      $.value
    ),
    
    transition_end: $ => seq(
      'end',
      $.value
    ),

    value: $ => choice(
      $.number,
      $.string,
      $.boolean,
      $.color,
      $.vector3,
      $.vector2
    ),

    number: $ => {
      const decimal = /[0-9]+\.[0-9]*/
      const integer = /[0-9]+/
      return token(choice(
        seq(optional('-'), decimal),
        seq(optional('-'), integer)
      ))
    },

    string: $ => seq('"', repeat(choice(/[^"\\]/, /\\./, seq('//', /.*/))), '"'),

    boolean: $ => choice('true', 'false'),

    vector2: $ => seq(
      '(',
      $.number,
      //',',
      $.number,
      ')'
    ),
    
    vector3: $ => seq(
      '(',
      $.number,
      //',',
      $.number,
      //',',
      $.number,
      ')'
    ),
    
    color: $ => seq(
      '(',
      $.number,
      //',',
      $.number,
      //',',
      $.number,
      //',',
      $.number,
      ')'
    ),

    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,

    comment: $ => token(seq('#', /.*/))
  }
});
