FROM ubuntu:22.04

# Устанавливаем необходимые пакеты
RUN apt-get update && apt-get install -y \
    curl \
    python3-pip \
    build-essential \
    python3-dev \
    libffi-dev \
    gfortran \
    libssl-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем yandex-tank
RUN pip3 install --upgrade pip setuptools && \
    pip3 install git+https://github.com/yandex/yandex-tank.git@b9423263234e3db0cab0b6781e0f5634cd5dd096

# Устанавливаем phantom
RUN apt-get update && apt-get install -y phantom phantom-ssl || true

# Копируем файлы для тестирования
COPY ammo.txt /var/loadtest/ammo.txt
COPY load.yaml /var/loadtest/load.yaml
COPY load_stress.yaml /var/loadtest/load_stress.yaml

WORKDIR /var/loadtest
