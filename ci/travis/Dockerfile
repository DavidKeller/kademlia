FROM fedora:latest
MAINTAINER David keller <david.keller@litchis.fr>
RUN dnf -y groupinstall "Development Tools" && \
    dnf -y install make gcc-c++ cmake boost-devel openssl-devel \
	    coreutils lcov rubygems && \
    pip3 install --upgrade behave sure &&  \
	gem install coveralls-lcov
USER 1000:1000
