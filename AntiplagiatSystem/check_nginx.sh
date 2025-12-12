echo "=== Проверка конфигурации nginx ==="

docker run --rm -v $(pwd)/nginx.conf:/etc/nginx/nginx.conf:ro nginx:alpine nginx -t

echo ""
echo "=== Проверка доступности сервисов ==="

echo "1. Проверка nginx..."
curl -f http://localhost:80/health && echo " ✓ nginx работает"

echo "2. Проверка API Gateway через nginx..."
curl -f http://localhost:80/health 2>/dev/null | grep -q "api-gateway" && echo " ✓ API Gateway доступен"

echo "3. Проверка File Service..."
docker exec antiplagiat-file-service curl -f http://localhost:8080/health 2>/dev/null && echo " ✓ File Service работает"

echo "4. Проверка Analysis Service..."
docker exec antiplagiat-analysis-service curl -f http://localhost:8080/health 2>/dev/null && echo " ✓ Analysis Service работает"

echo "5. Проверка PostgreSQL..."
docker exec antiplagiat-postgres pg_isready -U admin && echo " ✓ PostgreSQL работает"

echo ""
echo "=== Все проверки завершены ==="