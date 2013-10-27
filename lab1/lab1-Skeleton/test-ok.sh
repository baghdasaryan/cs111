#! /bin/sh

# Test that the program works correctly in a sequential mode.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
true

#: : :

false || echo hello, Dr. Eggert!
false && echo hello, world!
true && false || echo hello, TA!

pwd > pwd1
echo pwd > pwd2

cat < /etc/passwd | tr a-z A-Z | sort -u
cat < /etc/passwd | tr a-z A-Z | sort -u > out

#; ; ; ; ;

EOF

chmod +x test.sh

sh test.sh >test.exp || exit
../timetrash test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"

