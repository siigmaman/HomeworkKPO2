FROM alpine:latest

RUN apk update && apk add --no-cache \
    libstdc++ \
    libgcc

WORKDIR /app
COPY build/analysis-service /app/analysis-service

RUN adduser -D appuser && chown -R appuser:appuser /app
USER appuser

EXPOSE 8080

CMD ["/app/analysis-service"]