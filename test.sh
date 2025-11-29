#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    # 入力が { で始まる場合は int main() でラップする
    if [[ "$input" == "{"* ]]; then
        input="int main() $input"
    fi

    output/mini_compiler "$input" > tmp.s
    cc -c test.c -o output/test.o
    cc -o output/tmp tmp.s output/test.o
    output/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else
      echo "$input => $expected expected, but got $actual"
      exit 1
    fi
}

assert 0 '{ return 0; }'
assert 76 '{ return 76; }'
assert 21 '{ return 5+20-4; }'
assert 41 '{ return  12 + 34 - 5 ; }'
assert 47 '{ return 5+6*7; }'
assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'
assert 10 '{ return -10+20; }'
assert 10 '{ return - -10; }'
assert 10 '{ return - - +10; }'
assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'
assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'
assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'
assert 3 '{ int a; a=3; return a; }'
assert 8 '{ int a; int z; a=3; z=5; return a+z; }'
assert 6 '{ int a; int b; a=b=3; return a+b; }'
assert 3 '{ int foo; foo=3; return foo; }'
assert 8 '{ int foo123; int bar; foo123=3; bar=5; return foo123+bar; }'
assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'
assert 3 '{ {1; {2;} return 3;} }'
assert 5 '{ ;;; return 5; }'
assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'
assert 4 '{ if (0) return 3; else return 4; }'
assert 3 '{ if (1) return 3; else return 4; }'
assert 3 '{ if (1==1) return 3; else return 4; }'
assert 4 '{ if (1!=1) return 3; else return 4; }'
assert 3 '{ if (1>0) return 3; else return 4; }'
assert 4 '{ if (1<0) return 3; else return 4; }'
assert 3 '{ if (1>=0) return 3; else return 4; }'
assert 4 '{ if (1<=0) return 3; else return 4; }'
assert 3 '{ while (0) return 2; return 3; }'
assert 3 '{ while (1-1) return 2; return 3; }'
assert 2 '{ while (1) return 2; return 3; }'
assert 2 '{ while (2-1) return 2; return 3; }'
assert 55 '{ int i; int j; i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 '{ int i; int j; for (i=0; i<10; i=i+1) j=3; return j; }'
assert 3 '{ for (;;) return 3; }'
assert 3 '{ return ret3(); }'
assert 5 '{ return ret5(); }'
assert 8 '{ return ret3() + ret5(); }'
assert 2 '{ return ret5() - ret3(); }'
assert 8 '{ return add2(3, 5); }'
assert 2 '{ return sub(5, 3); }'
assert 21 '{ return add6(1,2,3,4,5,6); }'
assert 66 '{ return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 '{ return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'
assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 7 'int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }'
assert 1 'int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'
assert 3 'int main() { int x; int *y; x=3; y=&x; return *y; }'
assert 3 'int main() { int x; int y; int *z; x=3; y=5; z=&y+8; return *z; }'
assert 3 'int main() { int x; int *y; y=&x; *y=3; return x; }'

echo OK