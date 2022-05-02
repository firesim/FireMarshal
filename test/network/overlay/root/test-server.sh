#/bin/bash
ip_read=`ifconfig eth0 | grep "inet " | awk -F'[: ]+' '{ print $4 }'`
if [ "$ip_read" = "172.16.0.2" ]; then
	/root/server 2
       	if [ "$?" -eq "0" ]; then
		echo 'Success : ip + server'
	else
		echo 'Failed : server'
	fi
else
	echo 'Failed : ip'
	echo 'Expected : 172.16.0.2'
	echo 'Received : $ip_read'
fi
