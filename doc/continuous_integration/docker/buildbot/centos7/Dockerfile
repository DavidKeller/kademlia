FROM centos:centos7
MAINTAINER David keller <david.keller@litchis.fr>
RUN yum -y update && \
	yum -y group install "Development Tools" && \
	yum -y install python-devel python-setuptools && \
	easy_install buildbot-slave && \
	groupadd -r buildbot && useradd -m -r -g buildbot buildbot
ADD https://dl.fedoraproject.org/pub/epel/7/x86_64/b/botan-devel-1.10.8-5.el7.x86_64.rpm /root/
ADD https://dl.fedoraproject.org/pub/epel/7/x86_64/b/botan-1.10.8-5.el7.x86_64.rpm /root/
RUN yum -y localinstall /root/*.rpm
RUN yum -y install cmake boost-devel 
USER buildbot
WORKDIR /home/buildbot
RUN buildslave create-slave . buildbot.litchis.fr $SLAVE_NAME $SLAVE_PASSWORD && \
	echo "David Keller <david.keller@litchis.fr>" > info/admin && \
	echo "Centos 7 x64 slave" > info/host
CMD ["twistd", "--nodaemon", "--python", "buildbot.tac"]
