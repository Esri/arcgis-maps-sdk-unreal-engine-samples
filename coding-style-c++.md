# Unreal C++ Coding Style
The intent of this document is to provide direction and guidance to Esri Unreal C++ programmers that will enable them to employ good programming style and proven programming practices leading to safe, reliable, testable, and maintainable code.

We use Runtime Core [C++ Coding Guidelines](https://devtopia.esri.com/runtime/runtimecore/blob/master/docs/writing_code/guidelines/coding_guidelines.md) as our baseline and add few modifications to make it compatible with Unreal Engine [Coding Standard](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/index.html).

In order to make differences clear, only important modifications to the RTC code style will be covered in this guide.
Reading the two coding guidelines is recommened to understand the reasons behind each new rule.
<br/>
<br/>
## C++ Coding Standards

### General Rules
- Access specifiers **should** preferably be declared in the order of **public:**, **protected:**, then **private:**, with only one of each.
- Within each access specifier all methods **must** be declared first, followed by the member variables, with at least 1 blank line between them. This helps the reader quickly see the layout and size of the class.
- Use the [Clang format file](https://devtopia.esri.com/runtime/millennium-falcon/blob/master/docs/unreal.clang-format) defined below in the project to automatically format (lint) the code.
- Use braces in control statements even in one line block cases.
- Leave a blank line at the end of the file. All .cpp and .h files should include a blank line, to coordinate with gcc.
- Address compiler warnings. Compiler warning messages mean something is wrong. Fix what the compiler is warning you about. If you absolutely can't address it, use #pragma to suppress the warning, but this should only be done as a last resort.
- Debug code should either be generally useful and polished, or not checked in. Debug code that is intermixed with other code makes the other code harder to read.
- Always use the TEXT() macro around string literals. Without it, code that constructs FStrings from literals will cause an undesirable string conversion process.
- Shadowed variables (two variables with the same name in the same scope) are not allowed. C++ allows variables to be shadowed from an outer scope, but this makes usage ambiguous to a reader.
    ```
    int count = 0;
    ...
    if (IsChild())
    {
        int count = 10;
        ...
    }
    ```
- Use intermediate variables to simplify complicated expressions. If you have a complicated expression, it can be easier to understand if you split it into sub-expressions, that are assigned to intermediate variables, with names describing the meaning of the sub-expression within the parent expression.
    ```
    if ((Blah->BlahP->WindowExists->Etc && Stuff) && !(bPlayerExists && bGameStarted && bPlayerStillHasPawn && IsTuesday())))
    {
        DoSomething();
    }
    ```
    Should be replaced with

    ```
    const bool bIsLegalWindow = Blah->BlahP->WindowExists->Etc && Stuff;
    const bool bIsPlayerDead = bPlayerExists && bGameStarted && bPlayerStillHasPawn && IsTuesday();
    if (bIsLegalWindow && !bIsPlayerDead)
    {
        DoSomething();
    }
    ```

### Indentation
- Indentation **must** be 4 spaces. Tabs **must** be used for indentation.

### Long lines of code
- Comments and statements that extend beyond code viewer (150 columns) in a single line can be broken up and indented for readability. When passing large numbers of parameters, it is acceptable to group related parameters on the same line.

### Commenting
- Only valid C++ style comments (//) should be used
- Code that is not used (commented out) must be deleted
- One should avoid stating in comments what is better stated in code
- Comments in header files should describe the externally visible behavior of the functions or classes being documented
- Any important assumptions, contracts, or other useful information must be documented

### Naming

#### Capitalization
- Use PascalCase (aka Upper Camel Case) to name functions, classes, and other objects. The first letter of each word in a name (such as type name or variable name) is capitalized, and there is no underscore between words.
	- `UPrimitiveComponent` is correct, but not `lastMouseCoordinates` or `delta_coordinates`.

#### Naming Convention
- Type names **must** be prefixed with an additional upper-case letter to distinguish them from variable names, as described [here](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/#namingconventions).
	- `TArray, UObject, AActor, SWidget, FVector` are type names, `PositionArray, TargetEnemy, MainMenu1, Location` are variable names.
	- UnrealHeaderTool requires the correct prefixes in most cases, so it's important to provide them.
- Type and variable names **must** be nouns.
	- `AvailableTiles` is correct, `Available` is not.
- Boolean variables **must** be prefixed by **b**.
	- `bPendingDestruction` or `bHasFadedIn`.
- Multi-word names **must NOT** be written in snake_case.
	- `TargetEnemies` is correct, `target_enemies` is not.
- Function names, variable names, constants, and enum values **must NOT** be composed entirely of lowercase letters.
	- `GetChildActors()` is correct, `getChildActors(), getchildactors() or get_child_actors()` is not.

#### Classes
- Protected and private member variables of classes and structs **must NOT** be prefixed with **m_**.
	- `PreviousLocations` is correct, `m_previousLocations, _previousLocations` is not.
- Protected and private member functions **must NOT** be suffixed with an underscore (_).
- Interface classes (prefixed with "I") should always be abstract, and must not have member variables. Interfaces are allowed to contain methods that are not pure-virtual, and can even contain methods that are non-virtual or static, as long as they are implemented inline.
- Use the virtual and override keywords when declaring an overriding method. When declaring a virtual function in a derived class, that overrides a virtual function in the parent class, you must use both the virtual and the override keywords.
	
#### Functions
- Method names **must** be verbs that describe the method's effect, or describe the return value of a method that has no effect.
	- `FindNeighbors()` or `GetPosition()`.
- Identifiers **must NOT** begin with the underscore character (_). Standard library functions often begin identifiers with _ (ex: _main, _exit).
- All functions that return a bool **should** ask a true/false question.
	- `IsVisible()` or `ShouldClearBuffer()`.
- A procedure (a function with no return value) **should** use a strong verb followed by an Object.
	- `DestroyChildren()`.
	- Exception: the name of a method doesn't need to include the name of the class it belongs to because it is inferred from context => Class `Person` would contain the method `Duplicate` instead of `DuplicatePerson`.
- Names to avoid include those beginning with "Handle" and "Process" because the verbs are ambiguous.
	- Use `DestroyChildren()` instead of `HandleChildren()`.
- Functions that return a value **should** describe the return value. The name **should** clarify what value the function will return.
	- `bool IsTeaFresh(FTea Tea);` makes it clearer what true means compared to `bool CheckTea(FTea Tea);`

#### Parameters
- Function parameters **must** follow the PascalCase (aka Upper Camel Case) naming convention.
	```
	bool IsTeaFresh(FTea Tea);
	void SomeMutatingOperation(FThing& OutResult, const TArray<Int32>& InArray)
	void AddSomeThings(const int32 Count);
	void FBlah::SetMemberArray(TArray<FString> InNewArray)
	void Sweeten(const float EquivalentGramsOfSucrose);
	virtual void Drink(float& OutFocusMultiplier, float& OutThirstQuenchingFraction) override;
	float FTea::Steep(const float VolumeOfWater, const float TemperatureOfWater, float& OutNewPotency)
	FCup* MakeCupOfTea(FTea* Tea, bool bAddSugar = false, bool bAddMilk = false, bool bAddHoney = false, bool bAddLemon = false)
	``` 
- Though **not required**, we encourage you to prefix function parameter names with "Out" if they are passed by reference, and the function is expected to write to that value. This makes it obvious that the value passed in this argument will be replaced by the function.
	- `void GetLocationAndRotation(FVector& OutLocation, FQuat& OutRotation)`.
- If an In or Out parameter is also a boolean, put the "b" before the In/Out prefix.
	- `bOutResult`.
- Though **not required**, we encourage you to prefix function parameter names with "In" to avoid ambiguities with class properties.
	- `void ConditionalUpgradeFontDataToBulkData(UObject* InOuter);`.

#### Namespaces
- You can use namespaces to organize your classes, functions and variables where appropriate, but there are [specific rules](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/#namespaces) that you have to follow in Unreal.
- Namespaces are not supported by UnrealHeaderTool, so they should not be used when defining UCLASSes, USTRUCTs and so on.

### Modern C++ Language Syntax
- Extract from [here](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/#modernc++languagesyntax):
> We are using many C++14 language features that seem to be well-supported across modern compilers, such as range-based-for, move semantics and lambdas with capture initializers. In some cases, we can wrap up usage of these features in preprocessor conditionals (such as rvalue references in containers). However, we might decide to avoid certain language features entirely, until we are confident we won't be surprised by the appearance of a new platform appearing that can't digest the syntax.

- Using modern C++14 language features is encouraged.

### Function parameters
- Avoid using anonymous literals in function calls. Prefer named constants which describe their meaning.
    ```
    Trigger(TEXT("Soldier"), 5);
    ```
    Should be replaced with
    ```
    const FName ObjectName = TEXT("Soldier");
    const float CooldownInSeconds = 5;
    Trigger(ObjectName, CooldownInSeconds);
    ```
- Prefer passing enum type variables instead of multiple boolean variables.
    ```
    FCup* MakeCupOfTea(FTea* Tea, bool bAddSugar = false, bool bAddMilk = false, bool bAddHoney = false, bool bAddLemon = false);
    FCup* Cup = MakeCupOfTea(Tea, false, true, true);
    ```
    Should be replaced with
    ```
    enum class ETeaFlags
    {
        None,
        Milk  = 0x01,
        Sugar = 0x02,
        Honey = 0x04,
        Lemon = 0x08
    };
    FCup* MakeCupOfTea(FTea* Tea, ETeaFlags Flags = ETeaFlags::None);
    FCup* Cup = MakeCupOfTea(Tea, ETeaFlags::Milk | ETeaFlags::Honey);
    ```
- Boolean parameters are often better replaced by an enum. It makes the code much more readable at the callsite.
    ```
    void Initialize(bool bEnableCaching);
    ...
    Initialize(false);
    ```
    Could be replaced with
    ```
    enum Caching
    {
        Enable,
        Disable
    };
    void Initialize(Caching C);
    ...
    Initialize(Caching::Disable);
    ```
- Avoid overly-long function parameter lists. If a function takes many parameters then consider passing a dedicated struct instead.
- Avoid overloading functions by bool and FString, as this can have unexpected behavior.
    ```
    void Func(const FString& String);
    void Func(bool bBool);

    Func(TEXT("String")); // Calls the bool overload!
    ```

### API Design Guidelines
There is an extensive description of the Unreal [API Design Guidelines](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/#apidesignguidelines).
<br/>
<br/>
## Clang format
In order to simplify the formating of the C++ files to follow the standard and to automatically solve style fixes, a [.clang-format file](https://devtopia.esri.com/runtime/millennium-falcon/blob/master/docs/unreal.clang-format) has been defined.
New versions of Visual Studio automatically detect .clang-format files in your project folder and let's you easily reformat your document with the `Format Document` command or by pressing `Ctrl+K` `Ctrl+D`.
In order to automatically format the source code while working on the project, there are 2 useful extensions:

### Format document on Save
This extension formats your document using the appropriate .clang-format file in your project folder every time you save your file.
To install the extension, go to the Extensions menu and install the extension:
> Extensions > Manage Extensions > Online > Format document on Save

You will be asked to restart Visual Studio.

### Format All Files
With this extension you can format all the documents inside a folder of your Solution Explorer.
To install the extension, go to the Extensions menu and install the extension:

> Extensions > Manage Extensions > Online > Format All Files

You will be asked to restart Visual Studio.
Go to the Solution Explorer, right-click on the folder you want to format and select `Format All Files`.
This will automatically format all the files in that folder.
<br/>
<br/>
## Implementation

### Portable C++ code
- Use bool instead of BOOL, but NEVER assume the size of bool.
- Use TCHAR for a character, but NEVER assume the size of TCHAR.
    > TCHAR **must not** be used on third party libraries as it is defined by Unreal and we **must not** include code that depends on Unreal outside of the plugin code base.
- Use types that specify its size for all platforms: uint8/int8, uint16/int16, uint32/int32, uint64/int64, float (4 bytes), double (8 bytes).

### Use of standard libraries
Historically, [UE has avoided direct use of the C and C++ standard libraries](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/#useofstandardlibraries).
However, in recent years, the standard library has grown more stable and more mature, and includes functionality that we don't want to wrap with an abstraction layer or reimplement ourselves.

- Avoid mixing UE idioms and standard library idioms in the same API.
- Standard containers and strings should be avoided except in interop code.
- Public interfaces or blueprint definitions should **NOT** use standard containers or strings, only UE idioms.

### Physical Dependencies
- The source code of a Module **must** be organized in 3 folders:
    - `Public` folder: contains header files with declarations exposed by the module.
    - `Internal` folder: contains header files with declarations internally used. Classes **should** always be defined internal if they are not exposed by the module.
    - `Private` folder: contains source files with definitions.
- File names should not be prefixed where possible; for example, Scene.cpp instead of UScene.cpp
- All headers should protect against multiple includes with the #pragma once directive. Note that all compilers we use support #pragma once.
- Try to include every header you need directly, to make fine-grained inclusion easier.
- If you can use forward declarations instead of including a header, do so.

### Building Optimizations
- Split up large functions into logical sub-functions. One area of compilers' optimizations is the elimination of common subexpressions. The bigger your functions are, the more work the compiler has to do to identity them. This leads to greatly inflated build times.
- Don't use too many inline functions, because they force rebuilds even in files which don't use them. Inline functions should only be used for trivial accessors and when profiling shows there is a benefit to doing so.
- Be even more conservative in the use of FORCEINLINE. All code and local variables will be expanded out into the calling function, and this will cause the same build time problems caused by large functions.

### Platform-Specific Code

Please, follow the [rules](https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/#platform-specificcode) about platform-specific code.

### Protecting Code
We can create a precompiled library with our protected source code and link it as a binary library file, instead of distributing the original source code.
A precompiled library:
- **Must not** include Unreal containers or any other part of the Unreal engine to avoid linking against the Unreal libraries.
- Types defined by Unreal **must not** be used, built-in types **must** be used instead:
    - `uint8/int8, uint16/int16, uint32/int32, uint64/int64, TCHAR, ...` **must not** be used.
- Can have its own coding style. We will follow [C++ Coding Guidelines](https://devtopia.esri.com/runtime/runtimecore/blob/master/docs/writing_code/guidelines/coding_guidelines.md).
