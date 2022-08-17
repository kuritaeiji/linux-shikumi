FROM ubuntu:20.04

ARG UID
ARG USERNAME

RUN apt-get update && \
    apt-get install git binutils build-essential \
    sysstat strace man manpages-dev tree sudo mlocate vim lldb -y && \
    yes | unminimize

RUN useradd -m -s /bin/bash -u $UID $USERNAME

RUN echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /home/$USERNAME

USER $USERNAME