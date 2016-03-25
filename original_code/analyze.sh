function check {
	local n
	n=`grep "$1" $2 | wc -l`
	echo ${1}: $n
}

check '0 to 1' $1
check '1 to 0' $1
check '1 to 2' $1
check '2 to 0' $1
check '2 to 3' $1
check '3 to 0' $1
check 'System fully idle' $1
