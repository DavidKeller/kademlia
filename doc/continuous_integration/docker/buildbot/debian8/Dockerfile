# This is a comment
FROM debian:8
MAINTAINER David keller <david.keller@litchis.fr>
RUN apt-get update && \
	apt-get -y install build-essential python-dev python-setuptools buildbot-slave
RUN apt-get -y install cmake libboost-all-dev libssl-dev git
USER buildbot
WORKDIR /var/lib/buildbot/slaves
RUN buildslave create-slave . buildbot.litchis.fr $SLAVE_NAME $SLAVE_PASSWORD && \
	echo "David Keller <david.keller@litchis.fr>" > info/admin && \
	echo "Debian 8 x64 slave" > info/host
VOLUME /home/buildbot
CMD ["twistd", "--nodaemon", "--python", "buildbot.tac"]
