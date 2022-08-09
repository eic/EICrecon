
# Code style and naming
There are many coding guidelines which could be selected by the whole community, now we need a minimum to start with consistent code

## C++ 

### Common 

- file extensions: cc, h
- base namespace: eicrecon
- class names: CamelCase
- member functions: lowerCamelCase
- members:  m_snake_case
- local variables: snake_case


### Jana2 related: 

- factory names: WhatIsProduced_factory[_tag]
- services, processors, etc.: Name_service, Name_processor, Name_etc
- plugin with <name> should have <name>.cc with InitPlugin function
- templated classes: end with "T" (classes that inherit from them don't, unless they are also a template)

## Python

- Strongly follow [PEP-8](https://peps.python.org/pep-0008/) unless naming is dictated by C++ wrapping

