#!/bin/bash

function CheckLib {
 problem = $(dpkg -s $1|grep installed)
 echo "Checking for "$1:$problem
 if [ "" == "$problem" ]; then
      echo "Setting up "$1
      apt-get --force-yes --yes install $1
 fi
}
				
echo "Checking for requested libraries..."

CheckLib kdesvn
CheckLib codeblocks
CheckLib codeblocks-contrib
CheckLib build-essential
CheckLib libx11-dev
CheckLib libgl1-mesa-dev
CheckLib libxrandr-dev
CheckLib libxxf86vm-dev

echo "Done checkin for requested libraries."


