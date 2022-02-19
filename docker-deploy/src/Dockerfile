FROM gcc:9.3

MAINTAINER hz223@duke.edu

RUN mkdir /var/log/erss
ADD ./ /var/log/erss
WORKDIR /var/log/erss

ENTRYPOINT [ "./entrypoint.sh" ]