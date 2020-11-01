declare -A os_to_pac_man;

os_to_pac_man[/etc/arch-release]=pacman		# archlinux
os_to_pac_man[/etc/debian_version]=apt-get	# debian-like systems
os_to_pac_man[/etc/redhat-release]=dnf		# fedora
os_to_pac_man[/etc/gentoo-release]=emerge	# gentoo
# FIXME: add more if needed

####################
# identifying linux distro and packet manager

declare pac_man;

for distr in ${!os_to_pac_man[@]}
do
    if [[ -f ${distr} ]];then
        pac_man=${os_to_pac_man[${distr}]}
    fi
done

####################
# installing needed packages

if [[ ${pac_man} == "pacman" ]]
then
	echo -e "\033[1;32m OS identified as arch linux, using pacman to install...\033[0m"
	sudo pacman -Syu cmake make gcc mysql binutils openssl boost zlib curl
elif [[ ${pac_man} == "apt-get" ]]
then
	echo -e "\033[1;32m OS identified as debian-like, using apt-get to install...\033[0m"
	sudo apt-get install cmake make gcc mysql libssl-dev libboost-system-dev zlib1g-dev libcurl4-openssl-dev
elif [[ ${pac_man} == "dnf" ]]
then 
	echo -e "\033[1;32m OS identified as fedora or another red hat product, using dnf to install...\033[0m"
	sudo dnf install cmake make gcc mysql
else 
	echo -e "\033[1;31m current version of linux distro is not supported. modify /scripts/install-deps.sh arrcordingly!\033[0m"
fi

####################
# installing needed libraries

cd ../
mkdir -p lib/
cd lib/
git clone https://github.com/reo7sp/tgbot-cpp
cd tgbot-cpp
cmake .
make -j4
sudo make install
