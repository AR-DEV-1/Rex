# Rex Code Generation

## Introduction
Sometimes code needs to be generated before passed to the compiler. Rex provides an easy way to generate code through json files.
Current rex can generate both enumerations and static arrays.

## Setup
There's always a json files that's the owner of the code, all other json files extend this code

### How to generate an enumeration
In an owning json file, the following fields should be provided
- Type: the type of the code that needs to be generated, either `Enum` or `Array`
- ClassName: the name of the enum class itself
- Filepath: the filepath, relative to the source folder, where the resulting code should be written to
- Content: the content of the enum

An example of generating an enumeration can go as followed. 

**Owning Json File Example**
```
"MyEnum": {
    "Type": "Enum",
    "ClassName": "MyEnum",
    "Filepath": "1_engine/rex_engine/include/public/rex_engine/my_enum.h",
    "Content": [
        "FirstItem",
        "SecondItem",
        "ThirdItem"
    ]
}
```

**Non-Owning Json File Example**
```
"MyEnum": {
    "Content": [
        "FourthItem",
        "FifthItem"
]
  }
```

### How to generate an array
In an owning json file, the following fields should be provided
- Type: the type of the code that needs to be generated, either `Enum` or `Array`
- ClassName: the name of the global array object
- ElementType: the type of the elements within the array
- Includes: the includes needed for the file
- Content: the content of the array


**Owning Json File Example**
```
"MyArray": {
    "Type": "Array",
    "Name": "g_my_global_array",
    "ElementType": "MyArrayElement",
    "Includes": [
        "rex_engine/my_include.h"
    ],
    "Filepath": "1_engine/rex_engine/include/public/rex_engine/my_array.h",
    "Content": [
        "FirstItem",
        "SecondItem",
        "ThirdItem",
]
  }
```

**Non-Owning Json File Example**
```
"MyArray": {
    "Content": [
        "FourthItem",
        "FifthItem"
    ]
  }

```