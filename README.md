# compilium
C compiler + hikalium

## 方針
- プリプロセッサディレクティブは以下のもののみ実装する。
  - `#define <ident> <value>`（関数形式には対応しない）
  - `#include "local file path"`
- 出力はintel形式アセンブリとする
  - `clang -S -mllvm --x86-asm-syntax=intel` としたときの出力と等価
