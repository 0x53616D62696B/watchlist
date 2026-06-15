# C++ Tokens

A token is the smallest meaningful unit of C++ source code after the compiler has broken the text into pieces.
Tokens are the vocabulary of the language: keywords, names, literals, operators, punctuation, and a few preprocessing-specific pieces.

For example:

```cpp
int count = 42;
```

The compiler reads this as separate tokens:

```text
int    count    =    42    ;
```

Whitespace and comments help separate tokens, but they are usually not tokens themselves after lexical analysis.

## Quick Reference

| Token Kind | Examples | Meaning |
| --- | --- | --- |
| Keywords | `int`, `class`, `return`, `if`, `constexpr` | Reserved words with fixed language meaning |
| Identifiers | `count`, `User`, `calculateTotal` | Names created by the programmer |
| Literals | `42`, `3.14`, `'A'`, `"hello"`, `true`, `nullptr` | Fixed values written directly in source code |
| Operators | `+`, `-`, `*`, `=`, `==`, `&&`, `::`, `->` | Symbols that perform operations or form expressions |
| Punctuators | `;`, `,`, `{}`, `()`, `[]` | Symbols that structure declarations, statements, and expressions |
| Preprocessing tokens | `#`, `##`, header names, macro pieces | Tokens used while handling preprocessor directives |

## Why Tokens Matter

Understanding tokens helps when reading compiler errors, formatting code, writing macros, and recognizing why two pieces of code are parsed differently.

For example:

```cpp
int value = 10;
```

is valid because the tokens form a declaration:

```text
int    value    =    10    ;
```

But this is not the same:

```cpp
intvalue = 10;
```

Here `intvalue` is one identifier token, not the keyword `int` followed by the identifier `value`.
The compiler does not split it into two tokens automatically.

## Keywords

Keywords are reserved words that already have meaning in C++.
They cannot be used as ordinary identifiers.

Examples:

```cpp
int number = 5;

if (number > 0) {
    return;
}
```

Common keywords include:

| Category | Examples |
| --- | --- |
| Types | `int`, `char`, `bool`, `double`, `void` |
| Control flow | `if`, `else`, `switch`, `case`, `for`, `while`, `break`, `continue`, `return` |
| Object-oriented programming | `class`, `struct`, `public`, `private`, `protected`, `virtual`, `override` |
| Storage and lifetime | `static`, `extern`, `thread_local`, `mutable` |
| Compile-time features | `const`, `constexpr`, `consteval`, `typename`, `template` |
| Error handling | `try`, `catch`, `throw`, `noexcept` |
| Namespaces and casts | `namespace`, `using`, `static_cast`, `dynamic_cast`, `reinterpret_cast`, `const_cast` |

Some words look special but are not regular keywords in every context.
For example, `override` and `final` are contextual identifiers: they have special meaning in specific positions, but can still appear as identifiers elsewhere.

## Identifiers

Identifiers are names for variables, functions, classes, namespaces, enum values, templates, and other programmer-defined entities.

```cpp
int age = 30;

double calculateArea(double radius) {
    return 3.14159 * radius * radius;
}
```

Identifier rules:

- Must begin with a letter or underscore.
- After the first character, may contain letters, digits, and underscores.
- Cannot be a keyword.
- Are case-sensitive: `value`, `Value`, and `VALUE` are different identifiers.

Valid examples:

```cpp
int count = 0;
int user_id = 42;
int MaxValue = 100;
```

Invalid examples:

```cpp
int 2fast = 10;       // starts with a digit
int class = 5;        // keyword
int user-name = 1;    // '-' is an operator, not part of an identifier
```

Avoid identifiers that begin with underscores in production code.
Many underscore patterns are reserved for the implementation, especially names beginning with double underscores or an underscore followed by an uppercase letter.

## Literals

Literals are values written directly in source code.

```cpp
int answer = 42;
double pi = 3.14159;
char grade = 'A';
const char* text = "hello";
bool enabled = true;
int* pointer = nullptr;
```

Common literal categories:

| Literal Kind | Examples | Type Notes |
| --- | --- | --- |
| Integer | `0`, `42`, `0xff`, `0b1010`, `1'000` | Usually `int`, `long`, or unsigned variants depending on value and suffix |
| Floating-point | `3.14`, `1.0f`, `2.0e10` | Usually `double`; suffix `f` gives `float` |
| Character | `'a'`, `'\n'`, `'\x41'` | Usually `char` or a character type with prefix |
| String | `"hello"`, `"line\n"` | Array of `const char` with a null terminator |
| Boolean | `true`, `false` | Type `bool` |
| Pointer | `nullptr` | Type `std::nullptr_t` |

Literal suffixes change the literal type:

```cpp
auto a = 42;    // int
auto b = 42u;   // unsigned int
auto c = 42L;   // long
auto d = 3.0;   // double
auto e = 3.0f;  // float
```

Digit separators make large numbers easier to read:

```cpp
int population = 1'000'000;
```

The apostrophes are part of the numeric literal token and do not affect the value.

## Operators

Operators are tokens that combine, transform, access, or compare values.

```cpp
int total = price + tax;
bool valid = total > 0 && total < 100;
```

Common operators:

| Category | Examples |
| --- | --- |
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Assignment | `=`, `+=`, `-=`, `*=`, `/=` |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| Logical | `&&`, `||`, `!` |
| Bitwise | `&`, `|`, `^`, `~`, `<<`, `>>` |
| Increment/decrement | `++`, `--` |
| Member access | `.`, `->`, `.*`, `->*` |
| Scope | `::` |
| Conditional | `?:` |
| Memory | `new`, `delete` |

Some operators are made of multiple characters and are still one token.
For example, `==` is one equality operator token, not two assignment tokens.

```cpp
if (a == b) {
    // comparison
}
```

## Punctuators

Punctuators structure the program.
They separate statements, group expressions, define scopes, and mark lists.

```cpp
void print(int value) {
    std::cout << value << '\n';
}
```

Common punctuators:

| Punctuator | Use |
| --- | --- |
| `;` | Ends a statement or declaration |
| `,` | Separates arguments, declarators, or initializer elements |
| `{}` | Defines a block, class body, namespace body, or initializer list |
| `()` | Groups expressions, calls functions, and holds parameter lists |
| `[]` | Array indexing, attributes, captures, and some declarations |
| `:` | Labels, access specifiers, inheritance, bit-fields, and ternary operator part |
| `...` | Variadic templates, parameter packs, and ellipsis |

Example:

```cpp
std::vector<int> values = {1, 2, 3};
```

Important tokens:

```text
std    ::    vector    <    int    >    values    =    {    1    ,    2    ,    3    }    ;
```

## Whitespace and Comments

Whitespace includes spaces, tabs, and newlines.
It usually separates tokens but does not otherwise affect meaning.

These are equivalent:

```cpp
int x = 1;
```

```cpp
int
x
=
1
;
```

But whitespace can be required to keep tokens separate:

```cpp
int value = 1;   // keyword int + identifier value
intvalue = 1;    // one identifier token
```

Comments are removed before normal compilation:

```cpp
// single-line comment

/*
   multi-line comment
*/
```

Comments can separate tokens:

```cpp
int/**/value = 1;
```

This behaves like:

```cpp
int value = 1;
```

## Preprocessing Tokens

Before normal compilation, the preprocessor handles directives such as `#include`, `#define`, `#if`, and `#ifdef`.

```cpp
#include <iostream>

#define MAX_COUNT 10
```

Preprocessor-related tokens include:

| Token | Use |
| --- | --- |
| `#` | Starts a preprocessing directive or stringizes a macro argument |
| `##` | Concatenates tokens in a macro |
| Header names | `<iostream>`, `"my_header.hpp"` |
| Macro identifiers | `MAX_COUNT`, `DEBUG`, `LOG` |

Macro token pasting example:

```cpp
#define MAKE_NAME(prefix, suffix) prefix ## suffix

int MAKE_NAME(user, Count) = 3; // becomes int userCount = 3;
```

Macros work at the token level, which is why they can be powerful but also risky.
Prefer constants, functions, templates, and `constexpr` values when they express the same idea cleanly.

## Maximal Munch

C++ tokenization generally uses the longest sequence of characters that can form a valid token.
This is sometimes called the maximal munch rule.

For example:

```cpp
a+++++b
```

is tokenized roughly as:

```text
a    ++    ++    +    b
```

not as:

```text
a    ++    +    ++    b
```

The resulting expression is usually invalid or at least confusing.
Write clear spacing instead:

```cpp
a++ + ++b;
```

Another example:

```cpp
std::vector<std::vector<int>> matrix;
```

Modern C++ correctly treats the final `>>` in nested template declarations as two closing angle brackets in this context.
Older C++ standards required a space:

```cpp
std::vector<std::vector<int> > matrix;
```

## Alternative Tokens

C++ provides word-like alternatives for some operators.
They are valid tokens, but most modern codebases use the symbolic forms.

| Alternative | Symbol |
| --- | --- |
| `and` | `&&` |
| `or` | `||` |
| `not` | `!` |
| `bitand` | `&` |
| `bitor` | `|` |
| `xor` | `^` |
| `compl` | `~` |
| `and_eq` | `&=` |
| `or_eq` | `|=` |
| `xor_eq` | `^=` |
| `not_eq` | `!=` |

Example:

```cpp
if (ready and not failed) {
    start();
}
```

This means the same as:

```cpp
if (ready && !failed) {
    start();
}
```

## Common Mistakes

### Confusing One Token With Two Tokens

```cpp
int value = 10;
intvalue = 20;
```

`intvalue` is not `int value`.
It is one identifier.

### Using a Keyword as a Name

```cpp
int return = 5; // invalid
```

Use a descriptive non-keyword name:

```cpp
int result = 5;
```

### Forgetting That Operators Can Be Multi-Character Tokens

```cpp
if (a = = b) {   // invalid
}
```

The equality operator must be one token:

```cpp
if (a == b) {
}
```

### Assuming Comments Behave Like Runtime Text

```cpp
int value = 1; // value = 2;
```

The commented text is removed from the program.
It does not execute and does not create tokens for the normal compiler phase.

## Practical Reading Strategy

When code looks confusing, try mentally splitting it into tokens:

```cpp
const std::string& name = user.GetName();
```

Tokens:

```text
const    std    ::    string    &    name    =    user    .    GetName    (    )    ;
```

Then group those tokens by meaning:

- `const std::string&` is the type.
- `name` is the identifier being declared.
- `=` starts initialization.
- `user.GetName()` is the expression used to initialize it.
- `;` ends the declaration.

This habit makes declarations, templates, compiler errors, and macro-heavy code easier to reason about.

## Sources

- [C++ draft: Lexical conventions](https://eel.is/c++draft/lex)
- [cppreference: Phases of translation](https://en.cppreference.com/w/cpp/language/translation_phases)
- [cppreference: C++ keywords](https://en.cppreference.com/w/cpp/keyword)
- [cppreference: Identifiers](https://en.cppreference.com/w/cpp/language/identifiers)
- [cppreference: Alternative operator representations](https://en.cppreference.com/w/cpp/language/operator_alternative)
