# nosh
nosh is a no shit C++ test framework

# Getting started
1. Download `nosh.hpp` or clone this repository
2. Make sure `nosh.hpp` is in one of your project's include paths
3. Follow the tutorial section to add test cases to your project using nosh

# Tutorial
## Simple test

```cpp
class SimpleTest: public Test {
public:
  SimpleTest() {
    //TODO Construction goes here
  }
  
  virtual ~SimpleTest() {
    //TODO Destruction goes here
  }
  
  virtual void run() {
    //TODO Test logic goes here
  }
};

int main(void) {
  execTest(SimpleTest());
}
```

## Assertions
### Basic assertions
| **Fatal assertion** | **Nonfatal assertion** | **Verifies** |
|:--------------------|:-----------------------|:-------------|
| `assertTrue(`_condition_`)`;  | `expectTrue(`_condition_`)`;   | _condition_ is true |
| `assertFalse(`_condition_`)`; | `expectFalse(`_condition_`)`;  | _condition_ is false |

### Binary assertions


| **Fatal assertion** | **Nonfatal assertion** | **Verifies** |
|:--------------------|:-----------------------|:-------------|
|`assertEQ(`_val1_`, `_val2_`);`|`expectEQ(`_val1_`, `_val2_`);`| _val1_ `==` _val2_ |
|`assertNE(`_val1_`, `_val2_`);`|`expectNE(`_val1_`, `_val2_`);`| _val1_ `!=` _val2_ |
|`assertLT(`_val1_`, `_val2_`);`|`expectLT(`_val1_`, `_val2_`);`| _val1_ `<` _val2_ |
|`assertLE(`_val1_`, `_val2_`);`|`expectLE(`_val1_`, `_val2_`);`| _val1_ `<=` _val2_ |
|`assertGT(`_val1_`, `_val2_`);`|`expectGT(`_val1_`, `_val2_`);`| _val1_ `>` _val2_ |
|`assertGE(`_val1_`, `_val2_`);`|`expectGE(`_val1_`, `_val2_`);`| _val1_ `>=` _val2_ |

## Test with assertions
TBD

## Test with parameters
TBD

## Generating parameters
TBD

## Creating generators
TBD

# Features
- [x] Test fixtures
- [x] Basic and binary assertions
- [x] Tests with parameters
- [x] Parameter generators
- [ ] Print information about test case before run. Print parameters in case parameters are involved
- [ ] Versbose/No verbose command line flag
- [ ] Filter tests from command line
- [ ] Filter tests programatically
- [ ] Export test results in JSON, XML
