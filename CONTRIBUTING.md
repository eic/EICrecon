
# Code style and naming
There are many coding guidelines which could be selected by the whole community, now we need a minimum to start with consistent code

## C++ 

### Common 

- file extensions: cc, h
- constands, definitions: SCREAMING_SNAKE_CASE
- class names: PascalCase
- member functions: camelCase
- members:  m_snake_case
- local variables: snake_case
- base namespace: eicrecon

(This code smaple is temporary and needs further development) 

```c++
#include <vector>

/** Doxygen style for Foo */
class Foo {
    friend class AnotherClass;

public:
    Foo(some_arg) {
        m_field = some_arg;
    }
private: 
  int m_field;
};

class AnotherClass {
public:
    static const int MAX_VALUE = 42;
    static const int MIN_VALUE = 0;

    virtual void method1() = 0;
    virtual void method2() = 0;
private:
    int m_x;
    int  x() { return m_x; }
    void setX(x) { m_x = x; }
};

```


### Jana2 related: 

- factory names: WhatIsProduced_factory[_tag]
- services, processors, etc.: Name_service, Name_processor, Name_etc
- plugin with `name` should have `name`.cc file with `InitPlugin` function (It makes it so much easier to find the entry point of a plugin in the src tree)
- templated classes: end with "T" (classes that inherit from them don't, unless they are also a template)

## Python

- Strongly follow [PEP-8](https://peps.python.org/pep-0008/) unless naming is dictated by C++ wrapping

