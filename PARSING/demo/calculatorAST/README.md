# Calculator AST Demo

This directory contains multiple versions of a calculator parser demonstrating different operator precedence and associativity configurations using Yacc/Bison and Lex/Flex.

## Files Overview

### Core Files
- **calc.c** - Main calculator implementation
- **calc.h** - Header file with AST node definitions
- **calc.lex** - Lexer specification (tokenization rules)
- **printast.c** - AST printing utilities
- **util.c** - Utility functions
- **Makefile** - Build configuration for all versions

### Parser Versions

The different `calc?.yacc` files demonstrate various operator precedence and associativity configurations:

| File | Description |
|------|-------------|
| **calc.yacc** | Full operators: `\|`, `&`, `+`, `-`, `*`, `/`, `%` all declared with `%left` precedence. Most comprehensive version. |
| **calc0.yacc** | Identical to calc.yacc. Reference/baseline version. |
| **calc1.yacc** | Minimal precedence: only `%left '-'` then `%left '*'`. Simplest version. |
| **calc2.yacc** | Precedence order: `%left '-'` (lower) then `%left '*'` (higher). Demonstrates precedence ordering. |
| **calc3.yacc** | Reversed precedence: `%left '*'` (lower) then `%left '-'` (higher). Shows how order affects parsing. |
| **calc4.yacc** | Adds unary minus: `%left '-'`, `%left '*'`, `%nonassoc UMINUS`. Handles prefix operators. |
| **calc5.yacc** | Unary minus first: `%nonassoc UMINUS` declared before binary operators. Alternative unary handling. |
| **calc6.yacc** | Complex precedence: `%left '-'`, `%left '*'`, `%right UMINUS`, `%left MINUSMINUS`. Demonstrates both unary and postfix operators. |

### Key Differences Explained

#### Operator Precedence
- **Higher precedence** = binds tighter = evaluates first
- In Yacc, **later declarations** have higher precedence than earlier ones
- Example: `%left '-'` then `%left '*'` means `*` has higher precedence than `-`

#### Associativity
- **`%left`** - Left associative (e.g., `a - b - c` = `(a - b) - c`)
- **`%right`** - Right associative (e.g., `a ^ b ^ c` = `a ^ (b ^ c)`)
- **`%nonassoc`** - Non-associative (prevents chaining, e.g., `a < b < c` is an error)

#### Expression Examples

With **calc2** (`%left '-'` then `%left '*'`):
- `2 + 3 * 4` → `2 + (3 * 4) = 14` (multiplication has higher precedence)
- `10 - 5 - 2` → `(10 - 5) - 2 = 3` (left associative)

With **calc3** (`%left '*'` then `%left '-'`):
- `2 + 3 * 4` → `(2 + 3) * 4 = 20` (subtraction has higher precedence - unusual!)
- Produces different parse tree and different results

With **calc4/calc5** (UMINUS handling):
- `-5 * 3` correctly parses as `(-5) * 3 = -15`
- UMINUS precedence ensures unary minus binds tighter than binary operators

## Building

### Build a Single Version
```bash
make calc       # Build default calc (uses calc.yacc)
make calc0      # Build calc0 (uses calc0.yacc)
make calc1      # Build calc1 (uses calc1.yacc)
... 
make calc6      # Build calc6 (uses calc6.yacc)
```

### Build All Versions
```bash
make all
```
This creates executables: `calc`, `calc0`, `calc1`, `calc2`, `calc3`, `calc4`, `calc5`, `calc6`

### Clean Build Artifacts
```bash
make clean
```
Removes all executables, object files, and generated parser/lexer files.

## Using the Makefile

The Makefile uses several key techniques to support multiple versions:

1. **Symbol Prefix (`-p` flag in yacc, `-P` flag in lex)**
   - Prevents symbol name collisions between versions
   - Each version gets unique function names (calc0_yyparse, calc1_yyparse, etc.)

2. **Separate Output Files**
   - Each version generates its own parser: `calcN_y.tab.c`, `calcN_y.tab.h`
   - Each version generates its own lexer: `calcN_lex.yy.c`

3. **Version-Specific Dependencies**
   - Each target has explicit dependencies on its version-specific files
   - Clean compilation with no conflicts

## Example Usage

To test different precedence behaviors:

```bash
# Build and test version 2 (multiplication has higher precedence)
make calc2
echo "2 + 3 * 4" | ./calc2

# Build and test version 3 (subtraction has higher precedence)
make calc3
echo "2 + 3 * 4" | ./calc3

# Compare results to see how precedence affects parsing
```

## Dependencies

- **yacc** or **bison** - Parser generator
- **lex** or **flex** - Lexer generator
- **cc** or **gcc** - C compiler
- **make** - Build tool

## Notes

- The shared files (calc.c, calc.h, printast.c, util.c, util.h) are used by all versions
- Each version produces a separate executable, allowing comparison of parsing behaviors
- The Makefile automatically handles all intermediate file generation and cleanup
- Use `make clean` before rebuilding if you make changes to the .yacc files
