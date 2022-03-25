#/bin/bash
ip_read=`ip -f inet addr show`
str="read ip"
echo $str
echo $ip_read
if [ "$ip_read" = "172.16.0.6" ]; then
	/root/client 172.16.0.4
       	if [ "$?" -eq "0" ]; then
		echo 'Success : ip + client'
	else
		echo 'Failed : client'
	fi
else
	echo 'Failed : ip'
fi
