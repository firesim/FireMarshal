#/bin/bash
ip_read=`ifconfig eth0 | grep "inet " | awk -F'[: ]+' '{ print $4 }'`
if [ "$ip_read" = "172.16.0.3" ]; then
	/root/client 172.16.0.2
       	if [ "$?" -eq "0" ]; then
		echo 'Success : ip + client'
	else
		echo 'Failed : client'
	fi
else
	echo 'Failed : ip'
	echo 'Expected : 172.16.0.3'
	echo 'Received : $ip_read'
fi
