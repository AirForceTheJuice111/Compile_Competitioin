# Some Demo Code

A collection of demonstrations showing different Lex/Yacc patterns and approaches.

## Files Overview

### Standalone Lexer Demo
- **try0.lex** - Standalone lexer demonstration
  - Pattern matching examples: `a`, `^ba` (anchored), comments like `--...`
  - Whitespace handling, newlines, and catch-all patterns
  - Standalone compilation (no yacc needed)
  - Use: `make try0` then `./try0`

### Paired Lex/Yacc Demos

#### 1. t.lex + t.yacc 
**Purpose:** Basic character-level parser using direct character tokens

- **t.lex**: Generic character-level lexer
  - Returns raw characters from input: `'x'`, `'y'`, `'['`, `']'`, `'='`, `';'`, etc.
  - No named tokens - everything is literal character matching
  - `\` and `\n` are skipped as whitespace

- **t.yacc**: Simple statement parser
  - Grammar: `stmlist: | stmlist stm`
  - Matches statements: `x[...]y;` (declaration) or `x[...]=y;` (assignment)
  - Bracket lists: `[`, `]`, `[]`, `[][][]`, etc.
  - Output: prints "Decl" or "Stm" when patterns match

- **Build**: `make t`
- **Usage**: `echo "x [] y ;" | ./t`

#### 2. try.lex + try.yacc 
**Purpose:** Named token demonstration using Lex to generate tokens

- **try.lex**: Token-generating lexer
  - `1` → returns `ONE` token
  - `2` → returns `TWO` token
  - `\n` → returns `EOL` token
  - Whitespace is skipped
  - Demonstrates Lex calling yacc-defined tokens

- **try.yacc**: Token matcher
  - Grammar: `list: | ONE ONE '\n' { printf("Got it!\n"); }`
  - Matches exactly: `1 1` followed by newline
  - Shows how yacc defines tokens (`%token ONE TWO EOL`) that lex uses

- **Build**: `make try`
- **Usage**: `echo "1 1" | ./try` (should print "Got it!")

## Building

```bash
# Build all demos
make all

# Build individual demos
make t      # Character-level parser
make try    # Token-based parser
make try0   # Standalone lexer

# Clean all artifacts
make clean
```

## Key Differences

| Feature | t.lex/t.yacc | try.lex/try.yacc | try0.lex |
|---------|--------------|------------------|----------|
| **Token Style** | Character literals (`'x'`, `'y'`) | Named tokens (`ONE`, `TWO`) | N/A (no parser) |
| **Yacc Needed** | Yes | Yes | No |
| **Lexer Complexity** | Simple pass-through | Pattern recognition | Pattern recognition |
| **Use Case** | Simple grammar parsing | Token-based parsing | Text pattern matching |

## How Lex and Yacc Communicate

The **try.lex + try.yacc** pair demonstrates the standard Lex/Yacc integration:

1. **yacc** generates `y.tab.h` with token definitions:
   ```c
   #define ONE 258
   #define TWO 259
   #define EOL 260
   ```

2. **lex** includes `y.tab.h` and returns these token IDs:
   ```lex
   1  { return ONE; }   /* Returns 258 to yacc */
   2  { return TWO; }   /* Returns 259 to yacc */
   ```

3. **yacc** uses these token values to match grammar rules

## Compilation Order

For paired lex/yacc projects, always compile yacc first:

```bash
yacc -d try.yacc        # Generates y.tab.c and y.tab.h (with token definitions)
lex try.lex             # Reads y.tab.h, generates lex.yy.c
cc -o try try.c y.tab.c # Link together
```

The Makefile handles this dependency automatically.

## Notes
- See [../calculatorAST/README.md](../calculatorAST/README.md) for more complex precedence demonstrations
