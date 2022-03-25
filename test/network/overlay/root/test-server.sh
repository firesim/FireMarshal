#/bin/bash
ip_read=`ifconfig eth0 | grep "inet " | awk -F'[: ]+' '{ print $4 }'`
if [ "$ip_read" = "172.16.0.4" ]; then
	/root/server 2
       	if [ "$?" -eq "0" ]; then
		echo 'Success : ip + server'
	else
		echo 'Failed : server'
	fi
else
	echo 'Failed : ip'
fi
