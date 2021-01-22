#!/bin/bash

test -z "$CONF" -o -z "$CLIENT" && exit 1

CONF="$HOME/$CONF"

# echo "CONF=$CONF"
# echo "CLIENT=$CLIENT"

# echo -n '192.168.1.13' > "$CONF"
# echo -n '192.168.1.13' > "~/.ntexec"
# exit 0

URL=https://www.example.org
function correct_conf { echo -n '192.168.1.13' > "$CONF" ; }

rm -rfv "$CONF"

set -x

# NoParam - Show help
./"$CLIENT"
read -r; clear

# OneParam - Fail on poor url
correct_conf; ./"$CLIENT" ''
correct_conf; ./"$CLIENT" -
correct_conf; ./"$CLIENT" --
correct_conf; ./"$CLIENT" 1
read -r; clear

# One/TwoParam(s) - Fail on poor conf
rm   -rfv                 "$CONF"; ./"$CLIENT" "$URL"
echo -e '192.168.1.13 '  >"$CONF"; ./"$CLIENT" "$URL"; ./"$CLIENT" --192.168.1.13 "$URL"
echo -e '192.168.1.13xy' >"$CONF"; ./"$CLIENT" "$URL"; ./"$CLIENT" "$URL" --192.168.1.13
echo -e '192.168.1.13\t' >"$CONF"; ./"$CLIENT" "$URL"; ./"$CLIENT" --192.168.1.13 "$URL"
echo -e '192.168.1.013'  >"$CONF"; ./"$CLIENT" "$URL"; ./"$CLIENT" "$URL" --192.168.1.13
echo -e '192.168.1.13'   >"$CONF"; ./"$CLIENT" "$URL"; ./"$CLIENT" --192.168.1.13 "$URL"
read -r; clear

# OneParam - Succeed
correct_conf; ./"$CLIENT" "$URL"
read -r; clear

# TwoParams - Fail on duplicate
correct_conf; ./"$CLIENT" --192.168.1.13 --192.168.1.13
read -r; clear

# TwoParams - Fail on ip gotchas
correct_conf; ./"$CLIENT" --192.168.89.64$'\n' "$URL"
correct_conf; ./"$CLIENT" '--192.168.89.64 ' "$URL"
correct_conf; ./"$CLIENT" "$URL" --192.168.89.64x
read -r; clear

# TwoParams - Succeed
correct_conf; ./"$CLIENT" --192.168.89.64 "$URL"
correct_conf; ./"$CLIENT" "$URL" --192.168.89.64
read -r; clear

# PrevConf - Succeed
./"$CLIENT" "$URL"
read -r; clear
