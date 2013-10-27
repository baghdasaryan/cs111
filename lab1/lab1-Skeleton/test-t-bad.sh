#! /bin/sh

# Test that syntax errors are caught.

tmp=$0-$$.tmp
mkdir "$tmp" || exit
(
cd "$tmp" || exit
status=

# Sanity check, to make sure that the program works with at least one good example.
echo echo Hello, world! >test0.sh || exit
../timetrash test0.sh >test0.out 2>test0.err || exit
echo 'Hello, world!' >test0.exp || exit
diff -u test0.exp test0.out || exit
test ! -s test0.err || {
  cat test0.err
  exit 1
}

n=1
for bad in \
  '`' \
  '>' \
  '<' \
  'a >b <' \
  ';' \
  '; a' \
  'a ||' \
  'a
     || b' \
  'a
     | b' \
  'a
     ; b' \
  'a;;b' \
  'a&&&b' \
  'a|||b' \
  '|a' \
  '< a' \
  '&& a' \
  '||a' \
  '(a|b' \
  'a;b)' \
  '( (a)' \
  'a>>>b'
do
  echo "$bad" >test$n.sh || exit
  ../timetrash test$n.sh >test$n.out 2>test$n.err && {
    echo >&2 "test$n: unexpectedly succeeded for: $bad"
    status=1
  }
  test -s test$n.err || {
    echo >&2 "test$n: no error message for: $bad"
    status=1
  }
  n=$((n+1))
done

exit $status
) || exit

rm -fr "$tmp"

