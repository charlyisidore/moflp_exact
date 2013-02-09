#!/bin/bash
# Authors:
# - CISSE Mohamed
# - SABER Takfarinas
# - LERSTEAU Charly
# M2 ORO

# From sscplp files (http://www-eio.upc.es/~elena/sscplp/), 
# generate file of the following format :
############################################################
#BEGIN OF FILE
# m : nbr of customers
# n : nbr of facilities
#
# matrix of assignation [ m x n ] over the first objective
# matrix of assignation [ m x n ] over the second objective
#
# cost of opening a facility over the first objective
#
# cost of opening a facility over the second objective
#
# demands of customers
#
# capacities of facilities
#END OF FILE
#############################################################
# e.g : create_cflp_instances lb ub m n
# lb, ub, m, n are integers
# [lb, ub] is an interval
# (m, n are now ignored and read directly from files)
# only the following calls are possible :
# create_cflp_instances 1 6 20 10
# create_cflp_instances 7 17 30 15
# create_cflp_instances 18 25 40 20
# create_cflp_instances 26 33 50 20
# create_cflp_instances 34 41 60 30
# create_cflp_instances 42 49 75 30
# create_cflp_instances 50 57 90 30

# download all sscplp instances (p?.txt) from Professor Elena Fernandez
function download_sscplp_instances 
{
	echo "Downloading sscplp instances from http://www-eio.upc.es/~elena/sscplp/ ..."
	for i in `seq 1 57`; do 
		wget -cq "http://www-eio.upc.es/~elena/sscplp/p$i.txt"
	done    
	echo "Done"
}

# remove all sscplp instances (p?.txt) downloaded
function delete_sscplp_instances
{
	echo "Removing sscplp instances ..."
	rm -f p*.txt
	echo "Done"
}

# remove all txt files
function delete_txt_files
{
	echo "Cleaning the directory ..."
	rm -f F*.txt p*.txt
	echo "Done"
}

# create cflp instances in a range
# parameters
# first - index of the first instance
# last - index of the last instance
function create_cflp_instances
{
	first=$1 
	last=$2

	local i # Solve conflicts in variable names
	local j

	echo "Creating instances $first -> $last ..."
	for i in `seq $first $((last-1))`
	do
		for j in `seq $(($i+1)) $last`
		do
			create_one_cflp_instance $i $j
		done
	done
	echo "Done"
}

# create one cflp instance with two mono-objective instances
# parameters
# index1 - index of the first instance
# index2 - index of the second instance
function create_one_cflp_instance
{
	local i # Solve conflicts in variable names
	export LC_NUMERIC=C # Read numbers as a C program

	destination="F$1-$2.txt"

	value1=($(cat "p$1.txt"))
	value2=($(cat "p$2.txt"))

	# Check if number of customers and number of facilities are the same in the two instances
	if [ ${value1[0]} -ne ${value2[0]} ] || [ ${value1[1]} -ne ${value2[1]} ]
	then
		echo "Error: mismatch number of customers and facilities between instances $1 and $2"
	fi

	# Use a more natural notation for numbers
	str=$(printf "%${#value1[@]}s")
	pattern=${str// /"%g "}
	value1=(`printf "$pattern" ${value1[@]}`)
	value2=(`printf "$pattern" ${value2[@]}`)

	# Parsing

	m=${value1[0]} # Number of customers
	n=${value1[1]} # Number of facilities

	cost1=(${value1[@]:2:($m*$n)}) # Costs of assignments in objective 1
	cost2=(${value2[@]:2:($m*$n)}) # Costs of assignments in objective 2

	demand=(${value1[@]:(2+$m*$n):$m}) # Demand of customers

	fcost1=(${value1[@]:(2+$m*$n+$m):$n}) # Costs of opening a facility in objective 1
	fcost2=(${value2[@]:(2+$m*$n+$m):$n}) # Costs of opening a facility in objective 2

	capacity=(${value1[@]:(2+$m*$n+$m+$n):$n}) # Capacity of facilities

	# Writing

	echo $m > $destination
	echo $n >> $destination
	echo "" >> $destination

	for i in `seq 0 $(($m-1))`
	do
		echo ${cost1[@]:($i*$n):$n} >> $destination
	done
	echo "" >> $destination

	for i in `seq 0 $(($m-1))`
	do
		echo ${cost2[@]:($i*$n):$n} >> $destination
	done
	echo "" >> $destination

	echo ${fcost1[@]} >> $destination
	echo "" >> $destination

	echo ${fcost2[@]} >> $destination
	echo "" >> $destination

	echo ${demand[@]} >> $destination
	echo "" >> $destination

	echo ${capacity[@]} >> $destination
}

######## BEGIN ########

delete_txt_files
download_sscplp_instances

###
# TAKE CARE, there is a bug for F7-8.txt :
# because the file p8.txt does not respect the format
# there is in p8.txt m n ? in the first line
#
# and F42-43.txt :
# because there is a problem on file p43.txt, it does not respect the format too
# we need to remove the last character
###
# some corections :
mv p7.txt tmp.txt
mv p8.txt p7.txt
mv tmp.txt p8.txt

mv p42.txt tmp.txt
mv p43.txt p42.txt
mv tmp.txt p43.txt

rm -f tmp.txt

create_cflp_instances 1 6 20 10
#create_cflp_instances 7 17 30 15 ?? bug without the corrections
create_cflp_instances 8 17 30 15
create_cflp_instances 18 25 40 20
create_cflp_instances 26 33 50 20
create_cflp_instances 34 41 60 30
#create_cflp_instances 42 49 75 30 ?? bug without the corrections
create_cflp_instances 43 49 75 30
create_cflp_instances 50 57 90 30
delete_sscplp_instances

######## END ########

