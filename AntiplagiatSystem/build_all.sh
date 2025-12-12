set -e 

echo "=== Building Antiplagiat System ==="
echo ""

check_dependency() {
    local cmd=$1
    local package=$2
    
    if ! command -v $cmd &> /dev/null; then
        echo " Error: $cmd not found."
        echo "   Install: sudo apt-get install $package"
        exit 1
    fi
    echo "✓ $cmd found"
}

echo "Checking dependencies..."
check_dependency "cmake" "cmake"
check_dependency "g++" "g++"
check_dependency "docker" "docker.io"
check_dependency "docker-compose" "docker-compose"

echo ""
echo "Dependencies OK"
echo ""

build_service() {
    local service_name=$1
    echo "=== Building $service_name ==="
    
    if [ ! -d "$service_name" ]; then
        echo " Error: Directory $service_name not found"
        exit 1
    fi
    
    cd "$service_name"

    if [ -d "build" ]; then
        echo "  Removing old build directory..."
        rm -rf build
    fi

    mkdir build
    cd build

    echo "  Running cmake..."
    if ! cmake ..; then
        echo " cmake failed for $service_name"
        exit 1
    fi

    echo "  Running make..."
    if ! make -j$(nproc); then
        echo " make failed for $service_name"
        exit 1
    fi
    
    cd ../..
    echo " $service_name built successfully"
    echo ""
}

build_service "file-service"
build_service "analysis-service"
build_service "api-gateway"

echo "=== All services built successfully ==="
echo ""
echo "Summary:"
echo "  File Service:      ./file-service/build/file-service"
echo "  Analysis Service:  ./analysis-service/build/analysis-service"
echo "  API Gateway:       ./api-gateway/build/api-gateway"
echo ""
echo "To build and run with Docker:"
echo "  docker-compose build"
echo "  docker-compose up -d"
echo ""
echo "To run services locally (for testing):"
echo "  ./file-service/build/file-service &"
echo "  ./analysis-service/build/analysis-service &"
echo "  ./api-gateway/build/api-gateway &"