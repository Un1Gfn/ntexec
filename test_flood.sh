#!/bin/bash

rm -fv /tmp/garbage

for i in {1..16}; do
  echo "https://www.google.com/search?q=$i" >>/tmp/garbage
done

./ntexec.out - </tmp/garbage

rm -fv /tmp/garbage
