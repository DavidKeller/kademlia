# This is a comment
FROM ubuntu:14.04
MAINTAINER David keller <david.keller@litchis.fr>
RUN apt-get update && \
	apt-get -y install build-essential python-dev python-setuptools && \
	easy_install buildbot-slave && \
	groupadd -r buildbot && useradd -m -r -g buildbot buildbot
RUN apt-get -y install cmake libboost-all-dev libssl-dev git
USER buildbot
WORKDIR /home/buildbot
RUN buildslave create-slave . buildbot.litchis.fr $SLAVE_NAME $SLAVE_PASSWORD && \
	echo "David Keller <david.keller@litchis.fr>" > info/admin && \
	echo "Ubuntu 14 x64 slave" > info/host
VOLUME /home/buildbot
CMD ["twistd", "--nodaemon", "--python", "buildbot.tac"]
