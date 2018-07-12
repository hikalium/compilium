# compilium
[![CircleCI](https://circleci.com/gh/hikalium/compilium.svg?style=svg)](https://circleci.com/gh/hikalium/compilium)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

C compiler + hikalium

## Build
```
make
```

## Test
```
make test
```

## Usage
```
./compilium -o dst.S src.c
```
Assembly source (.S) will be generated. You need to assemble it to get an executable binary.

### Options
- `--prefix_type <type>` : Specify symbol prefixes (`<type>` can be `Darwin` or `Linux`).

## License
MIT

## Author
[hikalium](https://github.com/hikalium)
