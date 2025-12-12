# Система проверки на плагиат (Antiplagiat System)

## Описание
Микросервисная система для проверки студенческих работ на плагиат. Система позволяет загружать работы, автоматически проверять их на заимствования и формировать отчеты.

## Архитектура
Система состоит из 3 микросервисов:
1. **File Service** (порт 8081) - хранение файлов и метаданных
2. **Analysis Service** (порт 8082) - проверка на плагиат
3. **API Gateway** (порт 8080) - единая точка входа

База данных: PostgreSQL (порт 5432)

## Технологии
- C++17
- cpprestsdk (для HTTP сервера)
- libpqxx (для работы с PostgreSQL)
- Docker & Docker Compose
- OpenSSL (для хеширования)

## Алгоритм проверки на плагиат
1. Для каждой загруженной работы вычисляется SHA-256 хеш
2. Хеш сравнивается с хешами ранее загруженных работ
3. Если найден идентичный хеш у другого студента - плагиат обнаружен
4. Формируется отчет с деталями совпадения

## Быстрый старт

### 1. Установка зависимостей
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    g++ \
    cmake \
    libpqxx-dev \
    libcpprest-dev \
    libssl-dev \
    nlohmann-json3-dev \
    docker.io \
    docker-compose
```

### 1. Сборка проекта
```bash
# Собрать все сервисы
./build_all.sh

# Или вручную для каждого сервиса
cd file-service && mkdir build && cd build && cmake .. && make
cd ../analysis-service && mkdir build && cd build && cmake .. && make
cd ../api-gateway && mkdir build && cd build && cmake .. && make
```
