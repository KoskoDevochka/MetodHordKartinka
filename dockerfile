
FROM ubuntu:22.04


ENV DEBIAN_FRONTEND=noninteractive


RUN apt-get update && apt-get install -y \
    build-essential \
    qt5-qmake \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5sql5 \
    libqt5sql5-sqlite \
    libqt5network5 \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY . /app

RUN mkdir -p /app/data


RUN qmake echoServer.pro -spec linux-g++ && make


EXPOSE 33333


CMD ["./echoServer"]