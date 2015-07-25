FROM centos:centos7
MAINTAINER David keller <david.keller@litchis.fr>
RUN yum -y update && \
	yum -y group install "Development Tools" && \
	yum -y install python-devel python-setuptools && \
	easy_install buildbot-slave && \
	groupadd -r buildbot && useradd -m -r -g buildbot buildbot
RUN yum -y install cmake boost-devel openssl-devel
USER buildbot
WORKDIR /home/buildbot
RUN buildslave create-slave . buildbot.litchis.fr $SLAVE_NAME $SLAVE_PASSWORD && \
	echo "David Keller <david.keller@litchis.fr>" > info/admin && \
	echo "Centos 7 x64 slave" > info/host
VOLUME /home/buildbot
CMD ["twistd", "--nodaemon", "--python", "buildbot.tac"]
