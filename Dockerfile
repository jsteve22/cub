FROM debian:testing

RUN apt-get update && apt-get install -y --no-install-recommends --no-install-suggests wget vim git sudo make gcc g++ python3

WORKDIR /home
