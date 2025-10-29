1. General
1.1 You must write C-code (Not C++).

1.2 Your code must compile without warnings.

1.3 Your code, when submitted, must be in a finished form. You are allowed a total of 5 lines of “commented-out” code per file.

1.4 Avoid using repetitive sections of copied and pasted code in your programs.

1.5 Busy loops (delays based on for-loops) and delay statements are not allowed, except for test code.

1.6 All code will be graded against the coding standard.

2. Files
2.1 All “.h” files must contain declarations for constant values, function prototypes, etc., that you want to “advertise” for use. All “.h” files must not contain executable code, only function declarations, #define, etc. This includes any definitions of any variables. Note: do not modify the provided “.h” files or you will lose points from the coding checker. Keep #defines only needed within a “.c” file local to that file.

2.2 All “.c” files must have a corresponding “.h” with the same base name, except for “main.c”.

3. Naming
3.1 All function names and constant values contained in “.h” files must be prefixed by the base-name of the file + underscore (e.g., ‘‘buttons_read()’’, ‘‘#define BUTTONS_BTN0_MASK 0x1’’). Function names and constant values that are used only within .c files do not need to be prefixed by the base-name of the file.

3.2 All ‘‘#define’’ names must be all uppercase letters.

3.3 All names (including #define) must be meaningful. For example, #define TEN 10 is unacceptable.

4. Data Types
4.1 For integer variables, you must use the types contained in stdint.h. There are two exceptions:
You may use int as the return type of main().
You may use the standard integer types when interacting with C or OS library calls (ie: char when working with strcpy(), int when calling open(), etc.).
5. Comments
5.1 Comments must be meaningful. Make sure that your code completely describes your intent. If your code is unclear or does not completely describe what is going on, comment accordingly.

5.2 Each function definition (not declaration), must have a comment header (immediately before the definition) which describes the function purpose/behavior, the arguments, and the return value.

5.3 Each time you create a new scope (when using {} braces), there must be a comment associated with it to describe what the code is doing. Loops and conditionals, for example, define a new scope.

The comment can immediately precede the scope or be inside the braces.
For if statements, a comment preceding the if is sufficient; you aren’t required to add additional comments for else/else if statements.
Small scope blocks (4 lines or less) don’t require a comment, just comment larger blocks. Blank lines count.
6. Formatting
6.1 You must use the clang-format code formatter on your code. The formatter will ensure that all indention is consistent throughout your code.
7. Magic Numbers
7.1 All scalar constants must be #define, with these exceptions:
The constants -1, 0, 1, or 2 may be used anywhere in your code.
Constants may be used in a const array (e.g., const uint8_t data[] = {0x10, 0x20, ...};
Any constants may be used inside of a function that contains "test" as a part of its name.
8. Miscellaneous
8.1 Use the const key word only if you are defining an array of constant values (e.g., const uint8_t foo[LEN] = {1, 2};)
9. State Machines
9.1 The state machine must use switch statements as taught by the textbook.

9.2 State transition must be before state action.

9.3 The state names used in the enum must be the same as those used in your state diagrams.

9.4 Each state machine must be placed in its own file. One state machine must be contained entirely in a single file.

9.5 All variables in your state machine defined outside of a function must be defined using the static keyword.

9.6 You must provide an init function (base-name_init) that is called before any of your provided functions can be called. This function must initialize the currentState and any other state variables.
