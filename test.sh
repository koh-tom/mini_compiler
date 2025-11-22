#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    output/mini_compiler "$input" > tmp.s
    cc -o output/tmp tmp.s
    output/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else
      echo "$input => $expected expected, but got $actual"
      exit 1
    fi
}

assert 0 0
assert 76 76

echo OK