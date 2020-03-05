$sudo apt-get install cgroup-bin cgroup-lite cgroup-tools cgroupfs-mount libcgroup1
$vim /etc/cgconfig.conf

#write this
----------------------------------------
mount
{
        memory = /var/cgroups;
}

group cache-test-arghya
{
        perm
        {
                task
                {
                        uid = rathish-exp;
                        gid = rathish-exp;
                        fperm = 770;
                }
                admin
                {
                        uid = rathish-exp;
                        gid = rathish-exp;
                        fperm = 770;
                }
        }

        memory
        {

        }
}

----------------------------------------

sudo update-grub
cgconfigparser -l /etc/cgconfig.conf
cgrulesengd -vvv --logfile=/var/log/cgrulesengd.log
