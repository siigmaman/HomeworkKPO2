FROM alpine:latest

RUN apk update && apk add --no-cache \
    g++ \
    cmake \
    make \
    postgresql-dev \
    curl-dev \
    openssl-dev

WORKDIR /app

COPY build/file-service /app/file-service

RUN adduser -D appuser && chown -R appuser:appuser /app
USER appuser

EXPOSE 8080

CMD ["/app/file-service"]