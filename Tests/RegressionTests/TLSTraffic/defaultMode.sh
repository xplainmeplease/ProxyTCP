#!/bin/bash
# set -e
# set -o pipefail
./../../bin/proxytcp --mode default --port 1080 &
CLOCK_PID=$!
sleep 1

echo "" > log_file.txt
curl  --socks5-hostname localhost:1080 https://google.com/ | grep "google.com" >> log_file.txt
curl  --socks5-hostname localhost:1080 https://stackoverflow.com/ | grep "stackoverflow.com" >> log_file.txt
curl  --socks5-hostname localhost:1080 https://en.lyrsense.com/ | grep "en.lyrsense.com" >> log_file.txt
curl  --socks5-hostname localhost:1080 https://football.ua/ | grep "football.ua" >> log_file.txt
# trap 'kill -2 $CLOCK_PID' EXIT
kill $CLOCK_PID
ls

sudo ./../../bin/proxytcp --mode capture --output "$1/ProxyTCP/bin/meme.pcap" --port 1090 &
ps -aux | awk '/proxytcp/ {print $2}'
CLOCK_PID=$(ps -aux | awk '/proxytcp/ {print $2}') 
SUDO_PID=$(echo $CLOCK_PID | awk '{print $1;}')
NEW_PID=$(echo $CLOCK_PID | awk '{print $2;}')
echo $SUDO_PID 
echo $NEW_PID 
sleep 1

echo "" > log_file.txt
curl  --socks5-hostname localhost:1090 https://www.google.com/ | grep "google.com" >> log_file.txt
curl  --socks5-hostname localhost:1090 https://www.stackoverflow.com/ | grep "stackoverflow.com" >> log_file.txt
curl  --socks5-hostname localhost:1090 https://en.lyrsense.com/ | grep "en.lyrsense.com" >> log_file.txt
curl  --socks5-hostname localhost:1090 https://football.ua/ | grep "football.ua" >> log_file.txt
# trap 'kill -2 $SUDO_PID' EXIT
kill $SUDO_PID
kill $NEW_PID