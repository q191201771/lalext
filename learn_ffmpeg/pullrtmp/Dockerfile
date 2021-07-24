FROM qq191201771/withffmpeg
COPY . .
CMD mkdir output && cd output && cmake .. && make && ./bin/pullrtmp
