# compilium v2
[![CircleCI](https://circleci.com/gh/hikalium/compilium/tree/v2.svg?style=svg)](https://circleci.com/gh/hikalium/compilium/tree/v2)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[WIP] C compiler + hikalium

## Build
```
make
```

## Usage
```
./compilium [--os_type=Linux|Darwin]
```

compilium takes stdin as an input, so you can compile your code like this (in bash):
```
./compilium <<< "int main(){ return 0; }"
```

## Test
```
make testall
```

## Local CI
```
circleci config validate
circleci local execute
```

## Debug Tips
```
make debug FAILCASE_FILE=examples/calc.c

```

## License
MIT

## Author
[hikalium](https://github.com/hikalium)
