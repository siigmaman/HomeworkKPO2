FROM alpine:latest

RUN apk update && apk add --no-cache \
    libstdc++ \
    libgcc

WORKDIR /app
COPY build/api_gateway /app/api_gateway

RUN adduser -D appuser && chown -R appuser:appuser /app
USER appuser

EXPOSE 8080

CMD ["/app/api_gateway"]