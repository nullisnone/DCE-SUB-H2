短信号获取软件运行环境搭建快速指南

本指南涉及指令运行环境如下：
操作系统版本: CentOS Linux release 7.9.2009 (Core)，可以使用cat /etc/redhat-release命令获取系统版本
Linux 内核版本: 3.10.0-1160.el7.x86_64，可以使用uname -r命令获取内核版本
网卡型号: Solarflare SFC9220，可用lspci命令查看网卡型号
Linux网卡节点名称：ens1f1
IP地址：10.226.13.22/24

1）添加静态路由
将组播目标地址239.239.239.8添加进静态路由表。
route add -host 239.239.239.8 dev ens1f1

运行route命令出现下面红框中的内容表示添加成功：


用route命令是临时添加路由表，如果永久添加路由表需要修改配置文件。
修改/etc/sysconfig/network-scripts/route-ens1f1，添加以下一行：
239.239.239.8/32 dev ens1f1

请注意在实际环境中的文件名可能跟示例不同，该文件名需要根据实际环境网络节点名称进行修改。

2）禁用源地址路由校验
在/etc/sysctl.conf文件中添加以下三行：
net.ipv4.conf.all.rp_filter = 0
net.ipv4.conf.default.rp_filter = 0
net.ipv4.conf.ens1f1.rp_filter = 0

重启机器，或者用以下命令使之立即生效：
sysctl -w net.ipv4.conf.all.rp_filter=0
sysctl -w net.ipv4.conf.default.rp_filter=0
sysctl -w net.ipv4.conf.ens1f1.rp_filter=0

运行sysctl –p输出下图红框中的内容表示修改成功。


3）运行addgroup命令加组播组
/root/addgroup -g multi://239.239.239.8@10.226.13.22#172.175.32.254 &
其中，
239.239.239.8为组播目标地址
10.226.13.22为本机ip地址
172.175.32.254为组播源地址

为了让add-group在机器重启后立即启动，将add-group 以以下形式加入到crontab:
@reboot /root/addgroup -g multi://239.239.239.8@10.226.13.22#172.175.32.254 &

运行tcpdump -i ens1f1 -nnn
如果能捕获到pyload长度为28或62字节的报文，表示组播报文接收配置成功。

4）运行demo程序
Socket 和 EXANIC版本： https://github.com/nullisnone/DCE-SUB
EFVI版本：https://github.com/nullisnone/DCE-SUB-EFVI
程序输出期货最新报价和5档最优报价表示运行成功。

请注意：
DCE-SUB-EFVI项目运行时需要将网卡名称改为实际环境中的名称。
在main.cpp中修改网卡名称：



