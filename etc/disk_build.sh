disk_name="disk.iso"
disk_size=1 #gb
disk_build_ins="disk_build"

dd if=/dev/zero of=$disk_name bs=1073741824 count=$disk_size

if [ -e $disk_build_ins ]
then
	while IFS= read -r line
	do
		IFS=' ' read -ra ADDR <<< $line
		bin=${ADDR[0]}
		start=${ADDR[1]}
		size=${ADDR[2]}

		if [ -e "../bin/$bin" ]
		then
			dd if="../bin/$bin" of=$disk_name seek=$start bs=512 count=$size conv=notrunc
		else
			echo "$bin not found"
			exit
		fi

	done < $disk_build_ins

	mv $disk_name ..
else
	echo "disk build instructions not found"
	exit
fi
