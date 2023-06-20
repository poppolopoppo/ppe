FROM golang

WORKDIR /discovery_test
COPY . .
RUN go get go.cryptoscope.co/secretstream/secrethandshake
RUN go get golang.org/x/net/ipv4
RUN go get golang.org/x/net/ipv6
RUN go build ./examples/discovery/main.go

EXPOSE 43873

ENTRYPOINT ["/discovery_test/main"]

CMD [ "echo", "Default argument from CMD instruction" ]