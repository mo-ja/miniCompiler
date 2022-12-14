#!/bin/bash

assert(){
    expected="$1"
    input="$2"
    
    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input -> $actual"
    else
	echo "$input -> $expected expected, but got $actual"
	exit 1
    fi
}

assert 0 0
assert 42 42
assert 16 "12+4"
assert 4 "12-3-5"
assert 4 " 12 - 3 - 5 "
assert 14 " 12 - (3 - 5) "
assert 47 "5 + 6*7"
assert 15 "5 * ( 9 - 6)"
assert 4 "(3 + 5 )/2"
assert 5 "+5"
assert 32 "+4 * 8"
assert 18 "-4*8 + 50"
assert 2 "(5 == 2+3) + 1"
assert 1 "(5 < 2+3) + 1"
assert 0 "(5 < 2+3)"
assert 1 "5 <= 2+3"
assert 1 "5 < 2*3"
assert 0 "5 > 2*3"
assert 0 "5 >= 2*3"
assert 1 "6 >= 2*3"
assert 1 "7 >= 2*3"
assert 1 "8*4 + (5 - 3) >= 2*3"
echo OK
