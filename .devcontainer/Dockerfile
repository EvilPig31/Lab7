# Используем базовый образ с GCC и CMake
FROM gcc:14.2.0

# Устанавливаем необходимые зависимости
RUN apt-get update && apt-get install -y \
    cmake python3 python3-dev python3-pip wrk clangd\
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y libgtest-dev libgmock-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/gtest
RUN cmake CMakeLists.txt \
    && make \
    && cp lib/*.a /usr/lib
    
RUN pip install numpy --break-system-packages
# Устанавливаем рабочую директорию
WORKDIR /workspace