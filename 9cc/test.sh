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

echo OK
