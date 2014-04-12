#!/bin/bash

urlnotfound=1
connecterr=2
total=0
exitVal=-1
totalTime=0
totalData=0

filename="urlList.txt"

method=$1
machine=$2

hits=0
count=0

echo "Method $method machine $machine"

case $method in
	1)
	for i in {0..2}
	do
		for url in `cat $filename`
		do
			echo -n "Requested $url"
			output=`./client $machine $url 2>&1`
			temp=`echo $output | cut -d ":" -f 1`
			totalData=`expr $totalData + $temp`
			temp=`echo $output | cut -d ":" -f 3`
			totalTime=`expr $totalTime + $temp`
			temp=`echo $output | cut -d ":" -f 2`
			echo " which is $temp"
			if [ $temp == 1 ]
			then
				hits=`expr $hits + 1`
			fi
			count=`expr $count + 1`
		done
	done
	;;

	2)
	for i in {1..1000}
	do
		echo -n "In iteration $i "
		rnum=`expr $RANDOM % 100`
		if [ $rnum -lt 80 ]
		then
			lno=`expr $rnum % 3`
			lno=`expr $lno + 1`
			url=`head -$lno $filename | tail -1`
			echo "for $url"
			output=`./client $machine $url 2>&1`
		else
			lno=`expr 100 - $rnum`
			url=`head -$lno $filename | tail -1`
			echo "for $url"
			output=`./client $machine $url 2>&1`
		fi
		temp=`echo $output | cut -d ":" -f 1`
		totalData=`expr $totalData + $temp`
		temp=`echo $output | cut -d ":" -f 3`
		totalTime=`expr $totalTime + $temp`
		temp=`echo $output | cut -d ":" -f 2`
		echo " which is $temp"
		if [ $temp == 1 ]
		then
			hits=`expr $hits + 1`
		fi
		count=`expr $count + 1`
	done
	;;
esac

effectiveRate=`echo "scale=15; ($totalData/$totalTime) * 1000" | bc`
hitRatio=`echo "scale=15; ($hits/$count)" | bc`

echo "Effective data transfer rate : $effectiveRate"
echo "Hit ratio : $hitRatio"
