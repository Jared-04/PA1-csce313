echo -e "To remove colour from tests, set COLOUR to 1 in sh file\n"
COLOUR=0
if [[ COLOUR -eq 0 ]]; then
    ORANGE='\033[0;33m'
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
else
    ORANGE='\033[0m'
    GREEN='\033[0m'
    RED='\033[0m'
    NC='\033[0m'
fi

SCORE=0

echo -e "\nStart testing"

expected_output="Client-side is done and exited
Server terminated"

if ./client; then
	echo -e "  ${GREEN}Passed${NC}" ### FIRST ###
	SCORE=$(($SCORE+15))
else
	echo -e "  ${RED}Test 1 Failed${NC}"
fi

echo -e "\nSCORE: ${SCORE}/85\n"

# Test 2
#if ./client -p 1 -t 0.004 -e 1; then

expected_output="For person 1, at time 0.004, the value of ecg 1 is 0.68"
program_output=$(./client -p 1 -t 0.004 -e 1 2>&1)

if [[ $? -eq 0 && "$program_output" == *"$expected_output"* ]]; then
	echo -e " ${GREEN}Passed${NC}" ### SECOND ###
	SCORE=$(($SCORE+5))
else
	echo -e " ${RED}Test 2 p1 Failed${NC}"
fi
echo -e "\nSCORE: ${SCORE}/85\n"

# Test 2 part 2
./client -p 1
if diff -qwB <(head -n 1000 BIMDC/1.csv) received/x1.csv >/dev/null; then
        echo -e "  ${GREEN}Passed${NC}" ### THIRD ###
        SCORE=$(($SCORE+10))
else
        echo -e "  ${RED}Test 2 p2 Failed${NC}"
fi
echo -e "\nSCORE: ${SCORE}/85\n"

# Test 3
dd if=/dev/zero of=test.bin bs=1024k count=10
./client -f test.bin
if diff -qwB test.bin received/test.bin > /dev/null;then
	echo -e "  ${GREEN}Passed${NC}" ### FOURTH ###
        SCORE=$(($SCORE+20))
else
        echo -e "  ${RED}Test 3 (1/3) Failed${NC}"
fi

rm test.bin received/test.bin

truncate -s 10000000 test.bin
./client -f test.bin
if diff -qwB test.bin received/test.bin > /dev/null;then
        echo -e "  ${GREEN}Passed${NC}" ### FIFTH ###
        SCORE=$(($SCORE+10))
else
        echo -e "  ${RED}Test 3 (2/3) Failed${NC}"
fi

rm test.bin received/test.bin

head -c 100000000 /dev/zero > test.bin
truncate -s 100000000 test.bin
./client -f test.bin
if diff -qwB test.bin received/test.bin > /dev/null;then
        echo -e "  ${GREEN}Passed${NC}" ### SIXTH ###
        SCORE=$(($SCORE+5))
else
        echo -e "  ${RED}Test 3 (3/3) Failed${NC}"
fi

# rm test.bin received/test.bin

#test 4
./client -c -f 5.csv
if diff -qwB BIMDC/5.csv received/5.csv > /dev/null;then
	echo -e "  ${GREEN}Passed${NC}" ### SEVENTH ###
        SCORE=$(($SCORE+15))
else
	echo -e "  ${RED}Test 4 Failed${NC}"
fi
echo -e "\nSCORE: ${SCORE}/85\n"

#test 5
if ls fifo* data*_* *.tst *.o *.csv 1>/dev/null 2>&1; then
	echo -e "  ${RED}Test 5 Failed${NC}"
else
	echo -e "  ${GREEN}Passed${NC}" ### EIGHTH ###
        SCORE=$(($SCORE+5))
fi

echo -e "\nSCORE: ${SCORE}/85\n"
