#! /bin/bash

# Crash tests
yes helloWorld | head -n 20000 > test/tst
./crash_demo 32
cp test/tst test/tst_check_copy.txt
rm test/tst

./crash_demo -1
yes uhahaha | head -n 128 > test/tst

./crash_demo 0
yes uhahaha | head -n 128 > test/tst

./crash_demo 1
yes uhahaha | head -n 128 > test/tst

# cp tests
./crash_demo 2
cp test/tst test/tst_check_copy1

./crash_demo 3
cp test/tst test/tst_check_copy2

./crash_demo 1000
cp test/tst test/tst_check_copy3


# mv tests
./crash_demo 2
mv test/tst test/tst_check_mv

./crash_demo 3
mv test/tst_check_mv test/tst_check_mv2

./crash_demo 1000
mv test/tst_check_mv2 test/tst_check_mv3


./crash_demo 2
echo hello > test/file1 && cat test/file1
touch test/file1 && echo $?
diff base/hello.txt test/hello.txt >/dev/null 2>&1 && echo $?


# Uncrash the file system
./crash_demo -1
ls -la test

