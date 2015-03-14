Coding style is a somewhat contentious topic amongst programmers. Nearly everybody uses a different style, and they all insist that theirs is the correct way. So, for the record, and so that people can read my code more effectively, here is a concise documentation of the coding style used for this project:

## Symbol names ##

Function names should be a descriptive lowercase English name with multiple words divided by underscores, `just_like_this`.

Variable names should be short and abbreviated, with no interior uppercase letters, `likethis`.

`tmp` is preferred over `temp`.

When iterating over a linked list, if the previous entry must be preserved (for example, when removing an entry), the preferred variable names are `curr` and `prev`. If the previous entry does not matter, the preferred variable name is `tmp`.

If a symbol (either a function or variable) is to be used only by local code, it should be preceded with an underscore, `_like_this`. This marks it as something that should not be misused. An exception to this is any mutex lock, which is named `_lock` even though it may be used by outside code.

If you need an `int` to store the return value of certain functions to check for errors, it should always be called `r`.

A variable should never be called lowercase `l` or uppercase `O` to avoid the obvious confusion with 1 and 0.

`for` loop iterator variables should be `int`s called `i`, `j`, `k`, etc., skipping `l` as mentioned previously. Alternatively, when looping through a linked list, a loop like this can (and should) be used:

```
for (tmp = list_start; /* Condition */; tmp = tmp->next) {
    /* Loop interior */
}
```

Regarding integer types: SDL's fixed-width integer types (e.g. `Uint32`, `Sint16`) should be used in all structs and other situations where memory management matters (and care should be taken to avoid misalignment). Vanilla ints should be used for local variables and booleans.

Always use `int`, not `bool`. Not all C compilers support `bool`.

## Indentation ##

Indent appropriately. Each level should be indented by 4 spaces. Most code editors will allow you to use the tab key to insert that many spaces. Be sure you're inserting spaces and not tabs.

## Curly brace usage ##

The opening curly brace (`{`) of any block should go at the end of the same line, except for function declarations, where it should go on its own line. The closing curly brace (`}`) should always go on its own line.

`if` statements with only one line inside them may omit the curly braces. An empty loop (such as a busy `for` loop) may also omit the curly braces. All other blocks should always use curly braces.

Most code editors at this point will highlight curly brace pairs, so indicating at the end of every block where its beginning is is not necessary. Conversely, commenting every `#else`, `#elif`, and `#endif` _is_ necessary.

## `typedef` ##

When defining a new struct, do so like this:

```
typedef struct newstruct_ newstruct;
struct newstruct_ {
    /* Stuff goes here */
};
```

When defining a new enum, do so like this:

```
typedef enum {
    /* Stuff goes here */
} newenum;
```

## `const`, `define`, `enum` ##

`#define` should only be used in global scope, unless it's a macro, in which case it's acceptable to `#define` a helper macro within a function and `#undef` at the end of the function.

Remember that the main difference between `const` and `#define` is that `#define` inserts the value literally into the code, and `const` just acts as a memory value that you can't change. Thus, `const` should be used for frequently-used numbers that might bloat the code in memory. Memory conservation, especially with regards to cache size, should always be on your mind, especially when writing performance-critical parts of the engine.

`enum` should only be used in situations where an actual list is used. All values in the `enum` should have a constant prefix that easily identifies what it's for (e.g. the menu system has the `brmenu_action` `enum` with `DO_UP`, `DO_DOWN`, etc.) If the `enum` type does not need a global name, don't give it one; inline enum declarators are encouraged where applicable.

## Commenting ##

Comments should be descriptive, but not overly verbose. Say what you need to say and move on. That said, humor in comments does not go unappreciated.

```
/* Single line comments should look like this. */
```

```
/*
 * Multiline comments should look like this.
 * Electric boogaloo.
 */
```

## Compatibility ##

In general, maximum compatibility is desired. This mainly means:

  * All local variables should be declared at the beginning of a function. You should _never_ declare a new variable in a `for` loop initializer.
  * Struct and array initializers are only for initialization. Some compilers accept them as generic values (e.g. you could pass them to a function), but most don't.
  * Only use C-style comments (`/* */`). Never use C++-style comments (`//` to the end of the line).
  * The # in a preprocessor directive must always be the first character of the line it appears on. Most preprocessors will generate a warning if it isn't, and some won't accept it at all.
  * Only letters, numbers, and underscores are allowed in symbol names.
  * When working with function pointers, always use semantic notation (`*(func_pointer(args))`), not shortcut notation (`func_pointer(args)`).